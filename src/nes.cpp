#include "nes.hpp"
#include <thread>
#include <chrono>
#include <iostream>

namespace NESemu
{
    NES::NES(std::string rom_path, bool server, std::string ipaddr, int port) :
        m_romPath(rom_path),
        m_ppu(m_cartridge, m_cpu, m_screen),
        m_cpu(m_cartridge, m_ppu, m_controller1, m_controller2),
        m_screenScale(4.f),
        m_netplug(server, ipaddr, port)
    {
        if (!m_cartridge.loadRom(m_romPath))
            exit(1);
        
        // sfml window
        m_window.create(sf::VideoMode(256 * m_screenScale, 240 * m_screenScale), "NESemu", sf::Style::Titlebar | sf::Style::Close);
        m_window.setVerticalSyncEnabled(true);
        m_screen.create(256, 240, m_screenScale, sf::Color::White); 

        m_cpu.reset();
        m_ppu.reset();

        m_netplug.plug();
    }

    void NES::setKeys(KeyBinding& p1)
    {
        m_controller1.m_keyBindings = p1;
    }

    void NES::run()
    {
        /* WINDOW LOOP */
        m_cycleTimer = std::chrono::high_resolution_clock::now();
        sf::Event event;
        while (m_window.isOpen())
        {
            while (m_window.pollEvent(event))
            {
                // Exit Event
                if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
                {
                    m_window.close();
                    return;
                }
            }

            update_controller();
            update_screen();

            // Interval
            if(m_netplug.m_server){
                m_elapsedTime = std::chrono::high_resolution_clock::now() - m_cycleTimer;
                std::this_thread::sleep_for(std::chrono::nanoseconds(1'000'000'000 / 60) - std::chrono::nanoseconds(m_elapsedTime));
                m_cycleTimer = std::chrono::high_resolution_clock::now();
            }

            // Draw
            m_window.draw(m_screen);
            m_window.display();
        }
    }

    void NES::update_screen(){
        if(m_netplug.m_server)
        {
            auto flag = m_ppu.m_evenFrame;
            while(flag == m_ppu.m_evenFrame)
            {
                // PPUs
                m_ppu.step();
                m_ppu.step();
                m_ppu.step();
                // CPU
                m_cpu.step();
            }
            m_netplug.send_screen(m_screen);
        }
        else
        {
            m_netplug.receive_screen(m_screen);
        }
    }

    void NES::update_controller(){
        if(m_netplug.m_server)
            m_netplug.receive_controller_state(m_controller2);
        else
            m_netplug.send_controller_state(m_controller1);
    }
}