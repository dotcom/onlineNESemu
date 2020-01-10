#pragma once
#include <functional>
#include <array>
#include <cstdint>
#include "screen.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"

/*
パレットの色が
p = 0b00100010
だとすると、
p & (0b00001111) -> 輝度
p & (0b00110000) -> 色度

実際の色をRGBAに対応させると、以下になる
*/

namespace NESemu
{

    const int ScanlineCycleLength = 341;
    const int ScanlineEndCycle = 340;
    const int VisibleScanlines = 240;
    const int ScanlineVisibleDots = 256;
    const int FrameEndScanline = 261;

    const int AttributeOffset = 0x3C0;

    struct CPU;
    struct PPU
    {    
        PPU(Cartridge& cartridge, CPU& cpu, Screen& screen);
        void step();
        void reset();

        void doDMA(const uint8_t* page_ptr);

        //Callbacks mapped to CPU address space
        //Addresses written to by the program
        void control(uint8_t ctrl);
        void setMask(uint8_t mask);
        void setOAMAddress(uint8_t addr);
        void setDataAddress(uint8_t addr);
        void setScroll(uint8_t scroll);
        void setData(uint8_t data);
        //Read by the program
        uint8_t getStatus();
        uint8_t getData();
        uint8_t getOAMData();
        void setOAMData(uint8_t value);
        
        uint8_t readOAM(uint8_t addr);
        void writeOAM(uint8_t addr, uint8_t value);
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t value);
        uint8_t readPalette(uint8_t paletteAddr);
        void updateMirroring();
        Screen &m_screen;
        std::vector<uint8_t> m_RAM;
        std::size_t NameTable0, NameTable1, NameTable2, NameTable3; //indices where they start in RAM vector
        std::vector<uint8_t> m_palette;
        Cartridge& m_cartridge;
        CPU& m_cpu;

        std::vector<uint8_t> m_spriteMemory;

        std::vector<uint8_t> m_scanlineSprites;

        enum State
        {
            PreRender,
            Render,
            PostRender,
            VerticalBlank
        } m_pipelineState;
        int m_cycle;
        int m_scanline;
        bool m_evenFrame;

        bool m_vblank;
        bool m_sprZeroHit;

        //Registers
        uint16_t m_dataAddress;
        uint16_t m_tempAddress;
        uint8_t m_fineXScroll;
        bool m_firstWrite;
        uint8_t m_dataBuffer;

        uint8_t m_spriteDataAddress;

        //Setup flags and variables
        bool m_longSprites;
        bool m_generateInterrupt;

        bool m_greyscaleMode;
        bool m_showSprites;
        bool m_showBackground;
        bool m_hideEdgeSprites;
        bool m_hideEdgeBackground;

        enum CharacterPage
        {
            Low,
            High,
        } m_bgPage,
            m_sprPage;

        uint16_t m_dataAddrIncrement;

        std::vector<std::vector<uint8_t>> m_pictureBuffer;
    };
}
