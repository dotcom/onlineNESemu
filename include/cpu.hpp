#pragma once
#include "cartridge.hpp"
#include "ppu.hpp"
#include "controller.hpp"

namespace NESemu
{
    struct PPU;
    struct CPU
    {     
        enum InterruptType
        {
            IRQ,
            NMI,
            BRK_
        };

        CPU(Cartridge& c, PPU& p, Controller& c1, Controller& c2);

        void interrupt(InterruptType type);
        void step();
        void reset();
        void reset(uint16_t start_addr);
        void log();

        uint16_t getPC() { return r_PC; }
        
        bool executeImplied(uint8_t opcode);
        bool executeBranch(uint8_t opcode);
        bool executeType0(uint8_t opcode);
        bool executeType1(uint8_t opcode);
        bool executeType2(uint8_t opcode);

        uint16_t readAddress(uint16_t addr);

        void pushStack(uint8_t value);
        uint8_t pullStack();

        //If a and b are in different pages, increases the m_SkipCycles by inc
        void setPageCrossed(uint16_t a, uint16_t b, int inc = 1);
        void setZN(uint8_t value);

        // bus
        void busWrite(uint16_t addr, uint8_t value);
        uint8_t busRead(uint16_t addr);
        void DMA(uint8_t page);

        int m_skipCycles;
        int m_cycles;

        //Registers
        uint16_t r_PC;
        uint8_t r_SP;
        uint8_t r_A;
        uint8_t r_X;
        uint8_t r_Y;

        //Status flags.
        //Is storing them in one byte better ?
        bool f_C;
        bool f_Z;
        bool f_I;
//          bool f_B;
        bool f_D;
        bool f_V;
        bool f_N;

        Cartridge& m_cartridge;
        PPU& m_ppu;
        Controller &m_controller1;
        Controller &m_controller2;
        std::vector<uint8_t> m_RAM;
    };
};