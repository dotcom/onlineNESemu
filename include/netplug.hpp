#include <SFML/Network.hpp>
#include <screen.hpp>
#include <controller.hpp>

namespace NESemu{
    struct Netplug
    {
        Netplug(bool server, std::string ipaddr, int port);
        void plug();
        size_t send(void* data, size_t size);
        size_t receive(void* buf, size_t size);
        void send_screen(Screen& screen);
        void receive_screen(Screen& screen);
        void send_controller_state(PhyController& controller);
        void receive_controller_state(NetController& controller);

        bool m_server;
        std::string m_ipaddr;
        int m_port;

        sf::TcpListener m_listener;
        sf::TcpSocket m_socket_screen;
        sf::TcpSocket m_socket_controller;
    };
}