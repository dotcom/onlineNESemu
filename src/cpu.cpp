#include "cpu.hpp"
#include <iostream>
#include <iomanip>

namespace NESemu
{
    const auto InstructionModeMask = 0x3;

    const auto OperationMask = 0xe0;
    const auto OperationShift = 5;

    const auto AddrModeMask = 0x1c;
    const auto AddrModeShift = 2;

    const auto BranchInstructionMask = 0x1f;
    const auto BranchInstructionMaskResult = 0x10;
    const auto BranchConditionMask = 0x20;
    const auto BranchOnFlagShift = 6;

    const auto NMIVector = 0xfffa;
    const auto ResetVector = 0xfffc;
    const auto IRQVector = 0xfffe;

    enum BranchOnFlag
    {
        Negative,
        Overflow,
        Carry,
        Zero
    };

    enum Operation1
    {
        ORA,
        AND,
        EOR,
        ADC,
        STA,
        LDA,
        CMP,
        SBC,
    };

    enum AddrMode1
    {
        IndexedIndirectX,
        ZeroPage,
        Immediate,
        Absolute,
        IndirectY,
        IndexedX,
        AbsoluteY,
        AbsoluteX,
    };

    enum Operation2
    {
        ASL,
        ROL,
        LSR,
        ROR,
        STX,
        LDX,
        DEC,
        INC,
    };

    enum AddrMode2
    {
        Immediate_,
        ZeroPage_,
        Accumulator,
        Absolute_,
        Indexed         = 5,
        AbsoluteIndexed = 7,
    };

    enum Operation0
    {
        BIT  = 1,
        STY  = 4,
        LDY,
        CPY,
        CPX,
    };

    enum OperationImplied
    {
        NOP = 0xea,
        BRK = 0x00,
        JSR = 0x20,
        RTI = 0x40,
        RTS = 0x60,

        JMP  = 0x4C,
        JMPI = 0x6C, //JMP Indirect

        PHP = 0x08,
        PLP = 0x28,
        PHA = 0x48,
        PLA = 0x68,

        DEY = 0x88,
        DEX = 0xca,
        TAY = 0xa8,
        INY = 0xc8,
        INX = 0xe8,

        CLC = 0x18,
        SEC = 0x38,
        CLI = 0x58,
        SEI = 0x78,
        TYA = 0x98,
        CLV = 0xb8,
        CLD = 0xd8,
        SED = 0xf8,

        TXA = 0x8a,
        TXS = 0x9a,
        TAX = 0xaa,
        TSX = 0xba,
    };

    int OperationCycles[0x100] = {
            7, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 0, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 3, 3, 5, 0, 4, 2, 2, 0, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 0, 3, 5, 0, 3, 2, 2, 0, 3, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            6, 6, 0, 0, 0, 3, 5, 0, 4, 2, 2, 0, 5, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            0, 6, 0, 0, 3, 3, 3, 0, 2, 0, 2, 0, 4, 4, 4, 0,
            2, 6, 0, 0, 4, 4, 4, 0, 2, 5, 2, 0, 0, 5, 0, 0,
            2, 6, 2, 0, 3, 3, 3, 0, 2, 2, 2, 0, 4, 4, 4, 0,
            2, 5, 0, 0, 4, 4, 4, 0, 2, 4, 2, 0, 4, 4, 4, 0,
            2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 0, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
            2, 6, 0, 0, 3, 3, 5, 0, 2, 2, 2, 2, 4, 4, 6, 0,
            2, 5, 0, 0, 0, 4, 6, 0, 2, 4, 0, 0, 0, 4, 7, 0,
    };

    enum IORegisters
    {
        PPUCTRL = 0x2000,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROL,
        PPUADDR,
        PPUDATA,
        OAMDMA = 0x4014,
        JOY1 = 0x4016,
        JOY2 = 0x4017,
    };

    CPU::CPU(Cartridge& c, PPU& p, Controller& c1, Controller& c2) :
        m_cartridge(c),
        m_ppu(p),
        m_controller1(c1),
        m_controller2(c2),
        m_RAM(0x800, 0)
    {}

    void CPU::reset()
    {
        reset(readAddress(ResetVector));
    }

    void CPU::reset(uint16_t start_addr)
    {
        m_skipCycles = m_cycles = 0;
        r_A = r_X = r_Y = 0;
        f_I = true;
        f_C = f_D = f_N = f_V = f_Z = false;
        r_PC = start_addr;
        r_SP = 0xfd; //documented startup state
   }

    void CPU::interrupt(InterruptType type)
    {
        if (f_I && type != NMI && type != BRK_)
            return;

        if (type == BRK_) //Add one if BRK, a quirk of 6502
            ++r_PC;

        pushStack(r_PC >> 8);
        pushStack(r_PC);

        uint8_t flags = f_N << 7 |
                     f_V << 6 |
                       1 << 5 | //unused bit, supposed to be always 1
          (type == BRK_) << 4 | //B flag set if BRK
                     f_D << 3 |
                     f_I << 2 |
                     f_Z << 1 |
                     f_C;
        pushStack(flags);

        f_I = true;

        switch (type)
        {
            case IRQ:
            case BRK_:
                r_PC = readAddress(IRQVector);
                break;
            case NMI:
                r_PC = readAddress(NMIVector);
                break;
        }

        m_skipCycles += 7;
    }

    void CPU::pushStack(uint8_t value)
    {
        busWrite(0x100 | r_SP, value);
        --r_SP; //Hardware stacks grow downward!
    }

    uint8_t CPU::pullStack()
    {
        return busRead(0x100 | ++r_SP);
    }

    void CPU::setZN(uint8_t value)
    {
        f_Z = !value;
        f_N = value & 0x80;
    }

    void CPU::setPageCrossed(uint16_t a, uint16_t b, int inc)
    {
        //Page is determined by the high byte
        if ((a & 0xff00) != (b & 0xff00))
            m_skipCycles += inc;
    }

    void CPU::step()
    {
        ++m_cycles;

        if (m_skipCycles-- > 1)
            return;

        m_skipCycles = 0;

        uint8_t opcode = busRead(r_PC++);

        auto CycleLength = OperationCycles[opcode];

        //Using short-circuit evaluation, call the other function only if the first failed
        //ExecuteImplied must be called first and ExecuteBranch must be before ExecuteType0
        if (CycleLength && (executeImplied(opcode) || executeBranch(opcode) ||
                        executeType1(opcode) || executeType2(opcode) || executeType0(opcode)))
        {
            m_skipCycles += CycleLength;
            //m_cycles %= 340; //compatibility with Nintendulator log
            //m_skipCycles = 0; //for TESTING
        }
        else
        {
            std::cerr << "Unrecognized opcode: " << std::hex << +opcode << std::endl;
        }
    }

    bool CPU::executeImplied(uint8_t opcode)
    {
        switch (static_cast<OperationImplied>(opcode))
        {
            case NOP:
                break;
            case BRK:
                interrupt(BRK_);
                break;
            case JSR:
                //Push address of next instruction - 1, thus r_PC + 1 instead of r_PC + 2
                //since r_PC and r_PC + 1 are address of subroutine
                pushStack(static_cast<uint8_t>((r_PC + 1) >> 8));
                pushStack(static_cast<uint8_t>(r_PC + 1));
                r_PC = readAddress(r_PC);
                break;
            case RTS:
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                ++r_PC;
                break;
            case RTI:
                {
                    uint8_t flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                r_PC = pullStack();
                r_PC |= pullStack() << 8;
                break;
            case JMP:
                r_PC = readAddress(r_PC);
                break;
            case JMPI:
                {
                    uint16_t location = readAddress(r_PC);
                    //6502 has a bug such that the when the vector of anindirect address begins at the last byte of a page,
                    //the second byte is fetched from the beginning of that page rather than the beginning of the next
                    //Recreating here:
                    uint16_t Page = location & 0xff00;
                    r_PC = busRead(location) |
                           busRead(Page | ((location + 1) & 0xff)) << 8;
                }
                break;
            case PHP:
                {
                    uint8_t flags = f_N << 7 |
                                 f_V << 6 |
                                   1 << 5 | //supposed to always be 1
                                   1 << 4 | //PHP pushes with the B flag as 1, no matter what
                                 f_D << 3 |
                                 f_I << 2 |
                                 f_Z << 1 |
                                 f_C;
                    pushStack(flags);
                }
                break;
            case PLP:
                {
                    uint8_t flags = pullStack();
                    f_N = flags & 0x80;
                    f_V = flags & 0x40;
                    f_D = flags & 0x8;
                    f_I = flags & 0x4;
                    f_Z = flags & 0x2;
                    f_C = flags & 0x1;
                }
                break;
            case PHA:
                pushStack(r_A);
                break;
            case PLA:
                r_A = pullStack();
                setZN(r_A);
                break;
            case DEY:
                --r_Y;
                setZN(r_Y);
                break;
            case DEX:
                --r_X;
                setZN(r_X);
                break;
            case TAY:
                r_Y = r_A;
                setZN(r_Y);
                break;
            case INY:
                ++r_Y;
                setZN(r_Y);
                break;
            case INX:
                ++r_X;
                setZN(r_X);
                break;
            case CLC:
                f_C = false;
                break;
            case SEC:
                f_C = true;
                break;
            case CLI:
                f_I = false;
                break;
            case SEI:
                f_I = true;
                break;
            case CLD:
                f_D = false;
                break;
            case SED:
                f_D = true;
                break;
            case TYA:
                r_A = r_Y;
                setZN(r_A);
                break;
            case CLV:
                f_V = false;
                break;
            case TXA:
                r_A = r_X;
                setZN(r_A);
                break;
            case TXS:
                r_SP = r_X;
                break;
            case TAX:
                r_X = r_A;
                setZN(r_X);
                break;
            case TSX:
                r_X = r_SP;
                setZN(r_X);
                break;
            default:
                return false;
        };
        return true;
    }

    bool CPU::executeBranch(uint8_t opcode)
    {
        if ((opcode & BranchInstructionMask) == BranchInstructionMaskResult)
        {
            //branch is initialized to the condition required (for the flag specified later)
            bool branch = opcode & BranchConditionMask;

            //set branch to true if the given condition is met by the given flag
            //We use xnor here, it is true if either both operands are true or false
            switch (opcode >> BranchOnFlagShift)
            {
                case Negative:
                    branch = !(branch ^ f_N);
                    break;
                case Overflow:
                    branch = !(branch ^ f_V);
                    break;
                case Carry:
                    branch = !(branch ^ f_C);
                    break;
                case Zero:
                    branch = !(branch ^ f_Z);
                    break;
                default:
                    return false;
            }

            if (branch)
            {
                int8_t offset = busRead(r_PC++);
                ++m_skipCycles;
                auto newPC = static_cast<uint16_t>(r_PC + offset);
                setPageCrossed(r_PC, newPC, 2);
                r_PC = newPC;
            }
            else
                ++r_PC;
            return true;
        }
        return false;
    }

    bool CPU::executeType1(uint8_t opcode)
    {
        if ((opcode & InstructionModeMask) == 0x1)
        {
            uint16_t location = 0; //Location of the operand, could be in RAM
            auto op = static_cast<Operation1>((opcode & OperationMask) >> OperationShift);
            switch (static_cast<AddrMode1>(
                    (opcode & AddrModeMask) >> AddrModeShift))
            {
                case IndexedIndirectX:
                    {
                        uint8_t zero_addr = r_X + busRead(r_PC++);
                        //Addresses wrap in zero page mode, thus pass through a mask
                        location = busRead(zero_addr & 0xff) | busRead((zero_addr + 1) & 0xff) << 8;
                    }
                    break;
                case ZeroPage:
                    location = busRead(r_PC++);
                    break;
                case Immediate:
                    location = r_PC++;
                    break;
                case Absolute:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case IndirectY:
                    {
                        uint8_t zero_addr = busRead(r_PC++);
                        location = busRead(zero_addr & 0xff) | busRead((zero_addr + 1) & 0xff) << 8;
                        if (op != STA)
                            setPageCrossed(location, location + r_Y);
                        location += r_Y;
                    }
                    break;
                case IndexedX:
                    // Address wraps around in the zero page
                    location = (busRead(r_PC++) + r_X) & 0xff;
                    break;
                case AbsoluteY:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + r_Y);
                    location += r_Y;
                    break;
                case AbsoluteX:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    if (op != STA)
                        setPageCrossed(location, location + r_X);
                    location += r_X;
                    break;
                default:
                    return false;
            }

            switch (op)
            {
                case ORA:
                    r_A |= busRead(location);
                    setZN(r_A);
                    break;
                case AND:
                    r_A &= busRead(location);
                    setZN(r_A);
                    break;
                case EOR:
                    r_A ^= busRead(location);
                    setZN(r_A);
                    break;
                case ADC:
                    {
                        uint8_t operand = busRead(location);
                        uint16_t sum = r_A + operand + f_C;
                        //Carry forward or UNSIGNED overflow
                        f_C = sum & 0x100;
                        //SIGNED overflow, would only happen if the sign of sum is
                        //different from BOTH the operands
                        f_V = (r_A ^ sum) & (operand ^ sum) & 0x80;
                        r_A = static_cast<uint8_t>(sum);
                        setZN(r_A);
                    }
                    break;
                case STA:
                    busWrite(location, r_A);
                    break;
                case LDA:
                    r_A = busRead(location);
                    setZN(r_A);
                    break;
                case SBC:
                    {
                        //High carry means "no borrow", thus negate and subtract
                        uint16_t subtrahend = busRead(location),
                                 diff = r_A - subtrahend - !f_C;
                        //if the ninth bit is 1, the resulting number is negative => borrow => low carry
                        f_C = !(diff & 0x100);
                        //Same as ADC, except instead of the subtrahend,
                        //substitute with it's one complement
                        f_V = (r_A ^ diff) & (~subtrahend ^ diff) & 0x80;
                        r_A = diff;
                        setZN(diff);
                    }
                    break;
                case CMP:
                    {
                        uint16_t diff = r_A - busRead(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType2(uint8_t opcode)
    {
        if ((opcode & InstructionModeMask) == 2)
        {
            uint16_t location = 0;
            auto op = static_cast<Operation2>((opcode & OperationMask) >> OperationShift);
            auto addr_mode =
                    static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift);
            switch (addr_mode)
            {
                case Immediate_:
                    location = r_PC++;
                    break;
                case ZeroPage_:
                    location = busRead(r_PC++);
                    break;
                case Accumulator:
                    break;
                case Absolute_:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    {
                        location = busRead(r_PC++);
                        uint8_t index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        //The mask wraps address around zero page
                        location = (location + index) & 0xff;
                    }
                    break;
                case AbsoluteIndexed:
                    {
                        location = readAddress(r_PC);
                        r_PC += 2;
                        uint8_t index;
                        if (op == LDX || op == STX)
                            index = r_Y;
                        else
                            index = r_X;
                        setPageCrossed(location, location + index);
                        location += index;
                    }
                    break;
                default:
                    return false;
            }

            uint16_t operand = 0;
            switch (op)
            {
                case ASL:
                case ROL:
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = f_C;
                        f_C = r_A & 0x80;
                        r_A <<= 1;
                        //If Rotating, set the bit-0 to the the previous carry
                        r_A = r_A | (prev_C && (op == ROL));
                        setZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = busRead(location);
                        f_C = operand & 0x80;
                        operand = operand << 1 | (prev_C && (op == ROL));
                        setZN(operand);
                        busWrite(location, operand);
                    }
                    break;
                case LSR:
                case ROR:
                    if (addr_mode == Accumulator)
                    {
                        auto prev_C = f_C;
                        f_C = r_A & 1;
                        r_A >>= 1;
                        //If Rotating, set the bit-7 to the previous carry
                        r_A = r_A | (prev_C && (op == ROR)) << 7;
                        setZN(r_A);
                    }
                    else
                    {
                        auto prev_C = f_C;
                        operand = busRead(location);
                        f_C = operand & 1;
                        operand = operand >> 1 | (prev_C && (op == ROR)) << 7;
                        setZN(operand);
                        busWrite(location, operand);
                    }
                    break;
                case STX:
                    busWrite(location, r_X);
                    break;
                case LDX:
                    r_X = busRead(location);
                    setZN(r_X);
                    break;
                case DEC:
                    {
                        auto tmp = busRead(location) - 1;
                        setZN(tmp);
                        busWrite(location, tmp);
                    }
                    break;
                case INC:
                    {
                        auto tmp = busRead(location) + 1;
                        setZN(tmp);
                        busWrite(location, tmp);
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }
        return false;
    }

    bool CPU::executeType0(uint8_t opcode)
    {
        if ((opcode & InstructionModeMask) == 0x0)
        {
            uint16_t location = 0;
            switch (static_cast<AddrMode2>((opcode & AddrModeMask) >> AddrModeShift))
            {
                case Immediate_:
                    location = r_PC++;
                    break;
                case ZeroPage_:
                    location = busRead(r_PC++);
                    break;
                case Absolute_:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    break;
                case Indexed:
                    // Address wraps around in the zero page
                    location = (busRead(r_PC++) + r_X) & 0xff;
                    break;
                case AbsoluteIndexed:
                    location = readAddress(r_PC);
                    r_PC += 2;
                    setPageCrossed(location, location + r_X);
                    location += r_X;
                    break;
                default:
                    return false;
            }
            uint16_t operand = 0;
            switch (static_cast<Operation0>((opcode & OperationMask) >> OperationShift))
            {
                case BIT:
                    operand = busRead(location);
                    f_Z = !(r_A & operand);
                    f_V = operand & 0x40;
                    f_N = operand & 0x80;
                    break;
                case STY:
                    busWrite(location, r_Y);
                    break;
                case LDY:
                    r_Y = busRead(location);
                    setZN(r_Y);
                    break;
                case CPY:
                    {
                        uint16_t diff = r_Y - busRead(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                case CPX:
                    {
                        uint16_t diff = r_X - busRead(location);
                        f_C = !(diff & 0x100);
                        setZN(diff);
                    }
                    break;
                default:
                    return false;
            }

            return true;
        }
        return false;
    }

    uint16_t CPU::readAddress(uint16_t addr)
    {
        return busRead(addr) | busRead(addr + 1) << 8;
    }

    uint8_t CPU::busRead(uint16_t addr){
        if (addr < 0x2000)
            return m_RAM[addr & 0x7ff];
        else if (addr < 0x4020)
        {
            if (addr < 0x4000) //PPU registers, mirrored
            {
                switch(addr & 0x2007) // 0x2007 == 0x2000 + 0b0111
                {
                    case(PPUSTATUS):
                        return m_ppu.getStatus();
                    case(PPUDATA):
                        return m_ppu.getData();
                }
            }
            else if (addr < 0x4018 && addr >= 0x4014) //Only *some* IO registers
            {
                switch(addr)
                {
                    case(JOY1):
                        return m_controller1.read();
                    case(JOY2):
                        return m_controller2.read();
                    case(OAMDATA):
                        return m_ppu.getOAMData();
                }
            }
        }
        else if (addr < 0x8000)
        {
        }
        else
        {
            if(m_cartridge.m_PRG_ROM.size() == 0x4000)
                return m_cartridge.m_PRG_ROM[(addr - 0x8000) & 0x3fff];
            return m_cartridge.m_PRG_ROM[(addr - 0x8000)];
        }
        return 0;
    }

    void CPU::busWrite(uint16_t addr, uint8_t value){
        if (addr < 0x2000)
            m_RAM[addr & 0x7ff] = value;
        else if (addr < 0x4020)
        {
            if (addr < 0x4000) //PPU registers, mirrored
            {
                switch(addr & 0x2007) // 0x2007 == 0x2000 + 0b0111
                {
                    case(PPUCTRL):
                        m_ppu.control(value);
                        break;
                    case(PPUMASK):
                        m_ppu.setMask(value);
                        break;
                    case(OAMADDR):
                        m_ppu.setOAMAddress(value);
                        break;
                    case(PPUADDR):
                        m_ppu.setDataAddress(value);
                        break;
                    case(PPUSCROL):
                        m_ppu.setScroll(value);
                        break;
                    case(PPUDATA):
                        m_ppu.setData(value);
                        break;
                }
            }
            else if (addr < 0x4017 && addr >= 0x4014) //only some registers
            {
                switch(addr)
                {
                    case(OAMDMA):
                        DMA(value);
                        break;
                    case(JOY1):
                        m_controller1.write(value);
                        m_controller2.write(value);
                        break;
                    case(OAMDATA):
                        m_ppu.setOAMData(value);
                        break;
                }
            }
        }
        else if (addr < 0x8000)
        {
        }
        else
        {
            if(m_cartridge.m_PRG_ROM.size() == 0x4000)
                m_cartridge.m_PRG_ROM[(addr - 0x8000) & 0x3fff] = value;
            m_cartridge.m_PRG_ROM[(addr - 0x8000)] = value;
        }
    }

    void CPU::DMA(uint8_t page)
    {
        m_ppu.doDMA(&m_RAM[(page << 8) & 0x7ff]);
        m_skipCycles += 513; //256 read + 256 write + 1 dummy read
        m_skipCycles += (m_cycles & 1); //+1 if on odd cycle
    }
};
