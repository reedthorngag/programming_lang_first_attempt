#include <unordered_map>

#include "../parser/parser.hpp"

#ifndef _COMPILER
#define _COMPILER

namespace Compiler {

    enum Size {
        BYTE,
        WORD,
        DWORD,
        QWORD
    };

    extern const char* SizeString[];

    enum Reg {
        RAX,
        RBX,
        RCX,
        RDX,
        RDI,
        RSI,
        RBP,
        RSP,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15
    };

    struct Register {
        Reg reg;
        const char* subRegs[4];
    };

    extern Register registers[];

    struct Local {
        int offset;
        int size;
    };

    struct Context {
        Parser::Node* node;
        std::unordered_map<char*, Local> locals;

    };

    bool compile(std::unordered_map<std::string, Parser::Node*>* tree, std::ofstream* out);
};

#endif
