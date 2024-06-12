
#include "operations.hpp"

namespace Compiler {

    void swapRegs(Reg a, Reg b) {
        out("xchg",registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }
};

