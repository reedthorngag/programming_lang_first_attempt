
#include "operations.hpp"

namespace Compiler {

    void swapRegs(Reg a, Reg b) {
        out("xchg",registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        Value tmp = registers[a].value;
        registers[a].value = registers[b].value;
        registers[b].value = tmp;
    }
};

