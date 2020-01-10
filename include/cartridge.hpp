#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace NESemu
{
    struct Cartridge
    {
        Cartridge();
        bool loadRom(std::string path);

        std::vector<uint8_t> m_PRG_ROM;
        std::vector<uint8_t> m_CHR_ROM;
        uint8_t m_nameTableMirroring;
    };
};