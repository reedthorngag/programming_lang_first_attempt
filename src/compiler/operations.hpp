#include <fstream>
#include <sstream>

#include "compiler.hpp"

#ifndef _OPERATIONS
#define OPERATIONS

namespace Compiler {

    inline void out(std::string op) {
        (*output) << op << "\n";
    }

    inline void out(std::string op, std::string arg) {
        (*output) << "    " << op << " " << arg << "\n";
    }

    inline void out(std::string op, std::string arg1, std::string arg2) {
        (*output) << "    " << op << " " << arg1 << "," << arg2 << "\n";
    }

    inline void out(std::string op, Type type, std::string arg1, std::string arg2) {
        (*output) << "    " << op << " " << TypeMap[type] << " " << arg1 << "," << arg2 << "\n";
    }

    inline const char* refLocalVar(Local* local) {
        std::stringstream out;
        out << "[rbp-" << std::to_string(local->offset) << "]";
        return out.str().c_str();
    }

    void swapRegs(Reg a, Reg b);
};

#endif
