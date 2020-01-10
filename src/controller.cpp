#include "controller.hpp"
#include <iostream>

namespace NESemu
{
    PhyController::PhyController() :
        m_flag(false),
        m_keyStates(0),
        m_keyBindings(TotalButtons)
    {}

    void PhyController::write(uint8_t b)
    {
        // 0x01 -> 0x00 と書き込まれたらキー状態を取得
        if (m_flag && ((b & 1) == 0))
        {
            m_keyStates = 0;
            int shift = 0;
            for (int button = A; button < TotalButtons; ++button)
            {
                m_keyStates |= (sf::Keyboard::isKeyPressed(m_keyBindings[button]) << shift++);
            }
        }
        m_flag = (b & 1);
    }

    uint8_t PhyController::read()
    {
        // リードすると次のボタンの情報にシフト
        uint8_t ret = (m_keyStates & 1);
        m_keyStates >>= 1;
        return ret | 0x40;
    }


    NetController::NetController() :
        m_flag(false),
        m_keyStates(0)
    {}

    void NetController::write(uint8_t b)
    {
        // 0x01 -> 0x00 と書き込まれたらキー状態を取得
        if (m_flag && ((b & 1) == 0))
        {
            m_keyStates = m_netKeyState;
        }
        m_flag = (b & 1);
    }

    uint8_t NetController::read()
    {
        // リードすると次のボタンの情報にシフト
        uint8_t ret = (m_keyStates & 1);
        m_keyStates >>= 1;
        return ret | 0x40;
    }
}