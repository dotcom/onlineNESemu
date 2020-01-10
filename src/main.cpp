#include "nes.hpp"
#include <string>
#include <sstream>
#include <iostream>

int main(int argc, char** argv)
{
    NESemu::KeyBinding p1 {sf::Keyboard::J, sf::Keyboard::K, sf::Keyboard::RShift, sf::Keyboard::Return,
                           sf::Keyboard::W, sf::Keyboard::S, sf::Keyboard::A, sf::Keyboard::D};
    
    if (argc != 5){
        std::cerr << "invalid args" << std::endl;
        return 1;
    }

    bool server;

    std::string arg = argv[2];
    if (arg == "p1")
        server=true;
    else if (arg == "p2")
        server=false;
    else
    {
        std::cerr << "invalid args" << std::endl;
        return 1;
    }

    std::string addr = argv[3];
    std::string port = argv[4];
    
    std::cout << argv[2] << std::endl;
    NESemu::NES emulator(argv[1], server, addr, stoi(port));
    emulator.setKeys(p1);
    emulator.run();
    return 0;
}
