#include "cartridge.hpp"
#include <fstream>
#include <string>
#include <iostream>

namespace NESemu
{
    Cartridge::Cartridge(){}

    bool Cartridge::loadRom(std::string path)
    {
        std::ifstream file (path, std::ios_base::binary | std::ios_base::in);
        if (!file)
        {
            std::cerr << "open failed... " << path << std::endl;
            return false;
        }

        /*
        Read Header

        0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
        4: Size of PRG ROM in 16 KB units
        5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
        6: Flags 6 - Mapper, mirroring, battery, trainer
        7: Flags 7 - Mapper, VS/Playchoice, NES 2.0
        8: Flags 8 - PRG-RAM size (rarely used extension)
        9: Flags 9 - TV system (rarely used extension)
        10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
        11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)
        
        */
        std::vector<uint8_t> header(0x10);
        if (!file.read(reinterpret_cast<char*>(&header[0]), 0x10))
        {
            std::cerr << "read failed..." << std::endl;
            return false;
        }

        // マジックナンバー NES\x1A であること
        if (std::string{&header[0], &header[4]} != "NES\x1A")
        {
            std::cerr << "not ines format..." << std::endl;
            return false;
        }

        // Read PRG-ROM
        uint8_t prg_size = header[4];
        std::cout << "PRG_ROM size: " << prg_size << std::endl;
        if (!prg_size)
        {
            std::cerr << "no PRG-ROM..." << std::endl;
            return false;
        }
        else
        {
            m_PRG_ROM.resize(0x4000 * prg_size); // 16KB * prg_size
            if (!file.read(reinterpret_cast<char*>(&m_PRG_ROM[0]), 0x4000 * prg_size))
            {
                std::cerr << "Reading PRG-ROM from image file failed." << std::endl;
                return false;
            }
        }
        
        // Read CHR-ROM
        uint8_t chr_size = header[5];
        std::cout << "8KB CHR-ROM size: " << chr_size << std::endl;
        if (!chr_size)
        {
            // CHR-RAM 
            std::cout << "CHR-RAM is unsupported." << std::endl;
            return false;
        }
        else
        {
            m_CHR_ROM.resize(0x2000 * chr_size); // 8KB * chr_size
            if (!file.read(reinterpret_cast<char*>(&m_CHR_ROM[0]), 0x2000 * chr_size))
            {
                std::cerr << "Reading CHR-ROM from image file failed." << std::endl;
                return false;
            }
        }
        
        // Table Mirroring
        m_nameTableMirroring = header[6] & 0xB;
        std::cout << "Name Table Mirroring: " << +m_nameTableMirroring << std::endl;

        return true;
    }
}
