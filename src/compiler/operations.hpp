#include <fstream>

#include "compiler.hpp"

#ifndef _OPERATIONS
#define OPERATIONS

namespace Compiler {

    inline void out(std::string str) {
        (*output) << str << "\n";
    }

    inline void out(std::string str, std::string arg) {
        (*output) << "    " << str << " " << arg << "\n";
    }

    inline void out(std::string str, std::string arg1, std::string arg2) {
        (*output) << "    " << str << " " << arg1 << "," << arg2 << "\n";
    }

    void swapRegs(Reg a, Reg b);
};

#endif
