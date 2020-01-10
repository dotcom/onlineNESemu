#pragma once
#include <SFML/Graphics.hpp>
#include <chrono>
#include "screen.hpp"
#include "cartridge.hpp"
#include "controller.hpp"
#include "cpu.hpp"
#include "ppu.hpp"
#include "netplug.hpp"

namespace NESemu
{
    struct NES
    {
        NES(std::string rom_path, bool server, std::string ipaddr, int port);
        void setKeys(KeyBinding& p1);
        void run();
        void update_controller();
        void update_screen();

        sf::RenderWindow m_window;
        std::string m_romPath;

        Cartridge m_cartridge;
        PPU m_ppu;
        CPU m_cpu;
        PhyController m_controller1;
        NetController m_controller2;
        Screen m_screen;
        float m_screenScale;
        Netplug m_netplug;

        std::chrono::high_resolution_clock::time_point m_cycleTimer;
        std::chrono::high_resolution_clock::duration m_elapsedTime;
    };
}