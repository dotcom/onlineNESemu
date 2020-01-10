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
        sf::Packet packet;
        for (int i=0; i<sizeof(screen.m_screen_matrix); i++)
        {
            packet << screen.m_screen_matrix[i];
        }
        m_socket.send(packet, m_ipaddr, m_port);
    }

    void Netplug::receive_screen(Screen& screen)
    {
        sf::Packet packet;
        sf::IpAddress remote_addr;
        unsigned short remote_port;
        while(
            not (
                (remote_addr == static_cast<sf::IpAddress>(m_ipaddr)) &&
                (remote_port == m_port) &&
                (packet.getDataSize() == sizeof(screen.m_screen_matrix))
            )
        )
            m_socket.receive(packet, remote_addr, remote_port);
        
        for (int i=0; i<sizeof(screen.m_screen_matrix); i++)
        {
            packet >> screen.m_screen_matrix[i];
        }
        /*
        size_t received = 0;
        while(received != 256*240){
            size_t num = 0;
            m_socket_screen.receive(&screen.m_screen_matrix[received], 256*240 - received, num);
            received += num;
        }
        */
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
        while(
            not (
                (remote_addr == static_cast<sf::IpAddress>(m_ipaddr)) &&
                (remote_port == m_port) &&
                (packet.getDataSize() == sizeof(controller.m_netKeyState))
            )
        )
            m_socket.receive(packet, remote_addr, remote_port);
        /*
        size_t received;
        m_socket_controller.receive(&keyStates, sizeof(keyStates), received);
        */
        packet >> controller.m_netKeyState;
    }
}