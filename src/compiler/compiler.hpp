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
        QWORD
    };

    const int SizeByteMap[] {
        1,
        2,
        4,
        8
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

        Size::WORD,// f16, these will require special handling prob (in xmm regs?)
        Size::DWORD,// f32,
        Size::QWORD,// f64,

        Size::BYTE,// chr,
        Size::BYTE,// string, // this also requires special handling
        Size::BYTE,// boolean, // may be handled seperately to only take 1 bit in future
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
        STACK // simplifies stuff for what reg params use
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
        ValueType type;
        union {
            Local* local;
            Symbol* symbol;
            Parser::Node* parent;
        };
        bool modified;
        bool preserveModified;
        bool locked;
    };

    struct Register {
        Reg reg;
        const char* subRegs[4];
        Value value;
    };

    extern Register registers[];

    struct Context {
        Parser::Node* node;
        std::unordered_map<std::string, Local*>* locals;
    };

    bool freeReg(Reg reg);

    bool compile(std::unordered_map<std::string, Parser::Node*>* tree, std::ofstream* out);
};

#endif
