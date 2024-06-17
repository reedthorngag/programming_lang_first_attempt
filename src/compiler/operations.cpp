
#include "operations.hpp"

namespace Compiler {


    Reg assign(Reg a, Reg b) {

    }

    Reg assignAdd(Reg a, Reg b) {
        out("add", registers[a].subRegs[4])
    }

    Reg assignSub(Reg a, Reg b) {
        
    }

    Reg assignDiv(Reg a, Reg b) {
        
    }

    Reg assignMul(Reg a, Reg b) {
        
    }

    Reg assignMod(Reg a, Reg b) {
        
    }

    Reg assignShl(Reg a, Reg b) {
        
    }

    Reg assignShr(Reg a, Reg b) {
        
    }

    Reg assignXor(Reg a, Reg b) {

    }

    Reg assignAnd(Reg a, Reg b) {
        
    }

    Reg assignOr(Reg a, Reg b) {
        
    }

    std::unordered_map<std::string, void (*)(Reg a, Reg b)> assignmentOps = {
        {"=",1},
        {"+=",2},
        {"-=",2},
        {"/=",2},
        {"*=",2},
        {"%=",2},
        {"<<=",2},
        {">>=",2},
        {"^=",2},
        {"&=",2},
        {"|=",2},
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

    std::unordered_map<std::string, int> singleOperandOps = {
        {"!",16},
        {"~",16},
        {"++",17},
        {"--",17},
    };

    void swapRegs(Reg a, Reg b) {
        out("xchg",registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        Value tmp = registers[a].value;
        registers[a].value = registers[b].value;
        registers[b].value = tmp;
    }

    Reg assignmentOp(Node* op, Reg lvalue, Reg rvalue) {

    }

    Reg mathmaticalOp(Node* op, Reg lvalue, Reg rvalue) {
        
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

