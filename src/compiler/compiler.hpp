#include <unordered_map>

#ifndef _COMPILER
#define _COMPILER

#include "../parser/parser.hpp"
using namespace Parser;

namespace Compiler {

    extern std::ofstream* output;

    enum Size {
        BYTE,
        WORD,
        DWORD,
        QWORD,
        XMM = BYTE,
        YMM = WORD,
        ZMM = DWORD,
    };

    const int SizeByteMap[] {
        1,
        2,
        4,
        8
    };

    const int AvxByteMap[] {
        16,
        32,
        64
    };

    const Size TypeSizeMap[] {
        Size::BYTE,// error, doesnt occur in the program output

        Size::BYTE,// i8,
        Size::WORD,// i16,
        Size::DWORD,// i32,
        Size::QWORD,// i64,

        Size::BYTE,// u8,
        Size::WORD,// u16,
        Size::DWORD,// u32,
        Size::QWORD,// u64,

        Size::WORD,// f16, these will require special handling prob
        Size::DWORD,// f32,
        Size::QWORD,// f64,

        Size::BYTE,// chr,
        Size::QWORD,// string, this is a pointer to the string, this also requires special handling
        Size::BYTE,// boolean, // may be handled separately to only take 1 bit in future
        Size::BYTE,// null
    };

    extern const char* SizeString[];

    enum Reg {
        NUL, // so functions that generally return a value in a register can return nothing
        RAX,
        RBX,
        RCX,
        RDX,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        RSI,
        RDI,
        RBP,
        RSP,
        XMM0,
        XMM1,
        XMM2,
        XMM3,
        XMM4,
        XMM5,
        XMM6,
        XMM7,
        XMM8,
        XMM9,
        XMM10,
        XMM11,
        XMM12,
        XMM13,
        XMM14,
        XMM15,
        STACK, // simplifies stuff for what reg params use
    };

    struct Local {
        Parser::Symbol* symbol;
        int offset; // total offset. to get the value: mov reg, size [rbp - offset]
        Size size;
    };

    struct Global {
        Parser::Symbol* symbol;
    };

    enum ValueType {
        EMPTY,
        LOCAL,
        GLOBAL,
        INTERMEDIATE,
        PARAMETER,
    };

    struct Value {
        ValueType type = ValueType::EMPTY;
        union {
            Local* local;
            Symbol* symbol;
            Parser::Node* parent;
        };
        bool modified = false;
        bool preserveModified = true;
        bool locked = false;
    };

    extern unsigned int position;

    struct Register {
        Reg reg;
        const char* subRegs[4];
        Value value;
        unsigned int position;
    };

    extern Register registers[];

    struct Context {
        Parser::Node* node;
        Context* parent;
        std::unordered_map<std::string, Local*>* locals;
        int spaceReq;
        std::string* breakLabel;
        std::string* continueLabel;
    };

    bool freeReg(Reg reg);
    Reg findFreeReg();

    bool compile(std::unordered_map<std::string, Parser::Node*>* tree, std::ofstream* out);
};

#endif
