#include <netplug.hpp>
#include <iostream>

namespace NESemu{
    Netplug::Netplug(bool server, std::string ipaddr, int port) :
        m_server(server),
        m_ipaddr(ipaddr),
        m_port(port)
    {
        if(m_server){
            m_listener.listen(m_port);
        }
    }

    void Netplug::plug(){
        if(m_server){
            std::cout << "server : listenning..." << std::endl;
            m_listener.accept(m_socket_screen);
            m_listener.accept(m_socket_controller);
            std::cout << "server : be connected !" << std::endl;
        }
        else
        {
            std::cout << "client : connecting..." << std::endl;
            m_socket_screen.connect(m_ipaddr, m_port);
            m_socket_controller.connect(m_ipaddr, m_port);
            std::cout << "client : connect succeed!" << std::endl;
        }
        //m_socket.setBlocking(false);
    }

    void Netplug::send_screen(Screen& screen)
    {
        m_socket_screen.send(screen.m_screen_matrix, 256*240);
    }

    void Netplug::receive_screen(Screen& screen)
    {
        size_t received = 0;
        while(!(received == 256*240)){
            size_t num = 0;
            m_socket_screen.receive(&screen.m_screen_matrix[received], 256*240 - received, num);
            received += num;
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
        m_socket_controller.send(&keyStates, sizeof(keyStates));
    }
        
    void Netplug::receive_controller_state(NetController& controller)
    {
        unsigned int keyStates;
        size_t received;
        m_socket_controller.receive(&keyStates, sizeof(keyStates), received);
        controller.m_netKeyState = keyStates;
    }
}