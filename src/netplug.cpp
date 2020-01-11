#include <netplug.hpp>
#include <iostream>

namespace NESemu{
    Netplug::Netplug(bool server, std::string ipaddr, int port) :
        m_server(server),
        m_ipaddr(ipaddr),
        m_port(port)
    {
    }

    void Netplug::plug(){
        m_socket.bind(m_port);
        std::cout << "plugged port : " << m_port << std::endl;
    }

    void Netplug::send_screen(Screen& screen)
    {
        // DATA
        int sent = 0;
        while(sent != 256*240)
        {
            sf::Packet packet;
            packet << uint8_t(0); // DATA header
            packet << sent;
            for (int i=0; i<std::min(256*240 - sent,1472 - 5); i++)
            {
                packet << screen.m_screen_matrix[sent + i];
            }
            m_socket.send(packet, m_ipaddr, m_port);
            sent +=std::min(256*240 - sent,1472 - 5);
        }
        
        // END
        sf::Packet packet;
        packet << uint8_t(1); // END header
        m_socket.send(packet, m_ipaddr, m_port);
    }

    void Netplug::receive_screen(Screen& screen)
    {
        sf::Packet packet;
        sf::IpAddress remote_addr;
        unsigned short remote_port;
        for(;;)
        {
            uint8_t header;
            m_socket.receive(packet, remote_addr, remote_port);
            if ((remote_addr == static_cast<sf::IpAddress>(m_ipaddr)) && (remote_port == m_port)){
                auto size = packet.getDataSize();
                packet >> header;
                if (header == 0){ // DATA
                    int sent;
                    packet >> sent;
                    for (int i=0; i<size-1; i++)
                    {
                        packet >> screen.m_screen_matrix[sent + i];
                    }
                }
                else // END
                {
                    break;
                }
            }
        }
        
        for (int x=0; x<256; x++){
            for (int y=0; y<240; y++){
                screen.setPixel(x,y,screen.m_screen_matrix[x*240 + y]);
            }
        }
    }

    void Netplug::send_controller_state(PhyController& controller)
    {
        unsigned int keyStates = 0;
        int shift = 0;
        for (int button = Controller::A; button < Controller::TotalButtons; ++button)
        {
            keyStates |= (sf::Keyboard::isKeyPressed(controller.m_keyBindings[button]) << shift++);
        }

        sf::Packet packet;
        packet << keyStates;
        m_socket.send(packet, m_ipaddr, m_port);
    }
        
    void Netplug::receive_controller_state(NetController& controller)
    {
        sf::Packet packet;
        sf::IpAddress remote_addr;
        unsigned short remote_port;

        m_socket.setBlocking(false);
        for(;;) // dequeue all socket buffer
        {
            sf::Packet tmp_packet;
            sf::IpAddress tmp_remote_addr;
            unsigned short tmp_remote_port;
            if (m_socket.receive(tmp_packet, tmp_remote_addr, tmp_remote_port) == sf::Socket::Status::Done)
            {
                packet = tmp_packet;
                remote_addr = tmp_remote_addr;
                remote_port = tmp_remote_port;
            } 
            else
            {
                break;
            }
        }
        m_socket.setBlocking(true);

        if((remote_addr == static_cast<sf::IpAddress>(m_ipaddr)) &&
            (remote_port == m_port) &&
            (packet.getDataSize() == sizeof(controller.m_netKeyState))
        )
            packet >> controller.m_netKeyState;
    }
}