#pragma once
#include <SFML/Window.hpp>
#include <cstdint>
#include <vector>

namespace NESemu
{
    using KeyBinding = std::vector<sf::Keyboard::Key>;

    struct Controller
    {
        enum Buttons
        {
            A,
            B,
            Select,
            Start,
            Up,
            Down,
            Left,
            Right,
            TotalButtons,
        };

        virtual void write(uint8_t b) = 0;
        virtual uint8_t read() = 0;

    };

    struct PhyController : Controller
    {
        PhyController();
        void write(uint8_t b);
        uint8_t read();

        bool m_flag;
        unsigned int m_keyStates;
        KeyBinding m_keyBindings;
    };

    struct NetController : Controller
    {
        NetController();
        void write(uint8_t b);
        uint8_t read();
        bool m_flag;
        unsigned int m_keyStates;
        unsigned int m_netKeyState;
    };
}