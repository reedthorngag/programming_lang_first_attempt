
#include "operations.hpp"

namespace Compiler {


    void assign(Reg a, Reg b) {
        out("mov", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void swap(Reg a, Reg b) {
        out("xchg", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void add(Reg a, Reg b) {
        out("add", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void sub(Reg a, Reg b) {
        out("sub", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void div(Reg a, Reg b) {
        out("   unsupported OP! (div)");
    }

    void mul(Reg a, Reg b) {
        out("   unsupported OP! (mul)");
    }

    void mod(Reg a, Reg b) {
        out("   unsupported OP! (mod)");
    }

    void shl(Reg a, Reg b) {
        out("shl", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void shr(Reg a, Reg b) {
        out("shr", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void _xor(Reg a, Reg b) {
        out("xor", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void _and(Reg a, Reg b) {
        out("and", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void _or(Reg a, Reg b) {
        out("or", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    void _not(Reg a) {
        out("not", registers[a].subRegs[3]);
    }

    void dec(Reg a) {
        out("dec", registers[a].subRegs[3]);
    }

    void inc(Reg a) {
        out("inc", registers[a].subRegs[3]);
    }

    void cmp(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
    }

    std::unordered_map<std::string, void (*)(Reg a, Reg b)> assignmentOps = {
        {"=",assign},
        {"+=",add},
        {"-=",sub},
        {"/=",div},
        {"*=",mul},
        {"%=",div},
        {"<<=",shl},
        {">>=",shr},
        {"^=",_xor},
        {"&=",_and},
        {"|=",_or},
    };

    std::unordered_map<std::string, int> mathmaticalOps = {
        {"||",6},
        {"&&",7},
        {"==",8},
        {"!=",8},
        {"<=",9},
        {">=",9},
        {">",9},
        {"<",9},
        {"|",10},
        {"^",11},
        {"&",12},
        {"<<",13},
        {">>",13},
        {">>>",13},
        {"+",14},
        {"-",14},
        {"*",15},
        {"/",15},
        {"%",15},
    };

    std::unordered_map<std::string, void (*)(Reg a)> singleOperandOps = {
        {"!",_not},
        {"~",_not},
        {"++",inc},
        {"--",dec},
    };

    void swapRegs(Reg a, Reg b) {
        out("xchg",registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        Value tmp = registers[a].value;
        registers[a].value = registers[b].value;
        registers[b].value = tmp;
    }

    Reg assignmentOp(Node* op, Reg lvalue, Reg rvalue) {
        return Reg::NUL;
    }

    Reg mathmaticalOp(Node* op, Reg lvalue, Reg rvalue) {
        return Reg::NUL;
    }
    
    Reg doOp(Node* op, Reg a, Reg b) {

        switch (op->op.type) {
            case OpType::ASSIGNMENT:
                return assignmentOp(op, a, b);
            case OpType::MATH:
                return mathmaticalOp(op, a, b);
            case OpType::SINGLE_OP_POSTFIX:
            case OpType::SINGLE_OP_PREFIX:
                break;
        }

        return Reg::NUL;
    }

}

