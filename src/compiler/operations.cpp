
#include "operations.hpp"

#include <string>

namespace Compiler {


    void assign(Reg a, Reg b) {
        if (a != b) out("mov", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void swap(Reg a, Reg b) {
        out("xchg", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void add(Reg a, Reg b) {
        out("add", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void sub(Reg a, Reg b) {
        out("sub", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void div(Reg a, Reg b) {

        if (a != Reg::RAX) out("push",registers[Reg::RAX].subRegs[Size::QWORD]);
        out("push",registers[Reg::RDX].subRegs[Size::QWORD]);
        out("push",registers[Reg::RCX].subRegs[Size::QWORD]);

        out("xor",registers[Reg::RDX].subRegs[Size::QWORD],registers[Reg::RDX].subRegs[Size::QWORD]);

        if (b == Reg::RAX) 
            if (a == Reg::RCX) swap(a,b);
            else assign(Reg::RCX, b);

        assign(Reg::RAX, a);

        out("div",registers[b].subRegs[Size::QWORD]);

        out("pop",registers[Reg::RCX].subRegs[Size::QWORD]);
        out("pop",registers[Reg::RDX].subRegs[Size::QWORD]);

        if (a != Reg::RAX) {
            assign(a,Reg::RAX);
            out("pop",registers[Reg::RAX].subRegs[Size::QWORD]);
        }
    }

    void mul(Reg a, Reg b) {
        out("   unsupported OP! (mul)");
    }

    void mod(Reg a, Reg b) {

        out("push",registers[Reg::RAX].subRegs[Size::QWORD]);
        if (a != Reg::RDX) out("push",registers[Reg::RDX].subRegs[Size::QWORD]);
        out("push",registers[Reg::RCX].subRegs[Size::QWORD]);

        out("xor",registers[Reg::RDX].subRegs[Size::QWORD],registers[Reg::RDX].subRegs[Size::QWORD]);

        if (b == Reg::RAX) 
            if (a == Reg::RCX) swap(a,b);
            else assign(Reg::RCX, b);

        assign(Reg::RAX, a);

        out("div",registers[b].subRegs[Size::QWORD]);


        out("pop",registers[Reg::RCX].subRegs[Size::QWORD]);

        if (a != Reg::RDX) {
            assign(a,Reg::RDX);
            out("pop",registers[Reg::RDX].subRegs[Size::QWORD]);
        }
        
        out("pop",registers[Reg::RAX].subRegs[Size::QWORD]);
    }

    void shl(Reg a, Reg b) {

        if (b != Reg::RCX) swap(b, Reg::RCX);

        out("shl", registers[a].subRegs[Size::QWORD], registers[Reg::RCX].subRegs[0]);

        if (b != Reg::RCX) swap(b, Reg::RCX);
    }

    void shr(Reg a, Reg b) {

        if (b != Reg::RCX) swap(b, Reg::RCX);
        out("shr", registers[a].subRegs[Size::QWORD], registers[Reg::RCX].subRegs[0]);
        if (b != Reg::RCX) swap(b, Reg::RCX);
    }

    void sal(Reg a, Reg b) {

        if (b != Reg::RCX) swap(b, Reg::RCX);

        out("sal", registers[a].subRegs[Size::QWORD], registers[Reg::RCX].subRegs[0]);

        if (b != Reg::RCX) swap(b, Reg::RCX);
    }

    void sar(Reg a, Reg b) {

        if (b != Reg::RCX) swap(b, Reg::RCX);
        out("sar", registers[a].subRegs[Size::QWORD], registers[Reg::RCX].subRegs[0]);
        if (b != Reg::RCX) swap(b, Reg::RCX);
    }

    void _xor(Reg a, Reg b) {
        out("xor", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void _and(Reg a, Reg b) {
        out("and", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void _or(Reg a, Reg b) {
        out("or", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void _not(Reg a) {
        out("not", registers[a].subRegs[Size::QWORD]);
    }

    void lnot(Reg a) {
        out("cmp",registers[a].subRegs[Size::QWORD],"0");
        out("mov",registers[a].subRegs[Size::QWORD],"0");
        out("setz",registers[a].subRegs[Size::BYTE]);
    }

    void dec(Reg a) {
        out("dec", registers[a].subRegs[Size::QWORD]);
    }

    void inc(Reg a) {
        out("inc", registers[a].subRegs[Size::QWORD]);
    }

    void cmp(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
    }

    void equal(Reg a, Reg b) {
        out("xor", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setz", registers[a].subRegs[Size::BYTE]);
    }

    void lor(Reg a, Reg b) {
        out("or", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("setnz", registers[a].subRegs[Size::BYTE]);
    }

    // logical and
    void land(Reg a, Reg b) {
        // this code is super bad and way longer than it needs to be probably, but my brain isnt working

        out("push", registers[b].subRegs[Size::QWORD]);

        out("cmp", registers[a].subRegs[Size::QWORD], "0");
        out("setnz", registers[a].subRegs[Size::BYTE]);
        out("cmp", registers[b].subRegs[Size::QWORD], "0");
        out("setnz", registers[b].subRegs[Size::BYTE]);

        out("test", registers[a].subRegs[Size::QWORD], registers[a].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setz", registers[a].subRegs[Size::BYTE]);

        out("pop", registers[b].subRegs[Size::QWORD]);
    }

    void ne(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setnz", registers[a].subRegs[Size::BYTE]);
    }

    void le(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setle", registers[a].subRegs[Size::BYTE]);
    }

    void ge(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setge", registers[a].subRegs[Size::BYTE]);
    }

    void l(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setl", registers[a].subRegs[Size::BYTE]);
    }

    void g(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
        out("mov", registers[a].subRegs[Size::QWORD], "0");
        out("setg", registers[a].subRegs[Size::BYTE]);
    }

    std::unordered_map<std::string, void (*)(Reg a, Reg b)> assignmentOps = {
        {"=",assign},
        {"+=",add},
        {"-=",sub},
        {"/=",div},
        //{"*=",mul},
        {"%=",mod},
        {"<<=",shl},
        {">>=",shr},
        {"^=",_xor},
        {"&=",_and},
        {"|=",_or},
    };

    std::unordered_map<std::string, void (*)(Reg a, Reg b)> mathmaticalOps = {
        {"||",lor},
        {"&&",land},
        {"==",equal},
        {"!=",ne},
        {"<=",le},
        {">=",ge},
        {">",g},
        {"<",l},
        {"|",_or},
        {"^",_xor},
        {"&",_and},
        {"<<",sal},
        {"<<<",shl},
        {">>",sar},
        {">>>",shr},
        {"+",add},
        {"-",sub},
        //{"*",mul},
        {"/",div},
        {"%",mod},
    };

    std::unordered_map<std::string, void (*)(Reg a)> singleOperandOps = {
        {"!",lnot},
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
        auto pair = assignmentOps.find(op->op.value);
        if (pair == assignmentOps.end()) {
            printf("ERROR: %s:%d:%d: unsupported op: '%s'!\n",op->token.file,op->token.line,op->token.column,op->op.value);
            return Reg::NUL;
        }

        pair->second(lvalue, rvalue);

        registers[lvalue].value.modified = true;
        
        return lvalue;
    }

    Reg mathmaticalOp(Node* op, Reg lvalue, Reg rvalue) {
        auto pair = mathmaticalOps.find(op->op.value);
        if (pair == mathmaticalOps.end()) {
            printf("ERROR: %s:%d:%d: unsupported op: '%s'!\n",op->token.file,op->token.line,op->token.column,op->op.value);
            return Reg::NUL;
        }

        freeReg(lvalue); // this preserves the value if it should be preserved
        pair->second(lvalue, rvalue);

        registers[lvalue].value = Value{ ValueType::INTERMEDIATE, 0, false, false, false };

        return lvalue;
    }

    Reg singleOperandOp(Node* op, Reg a) {
        auto pair = singleOperandOps.find(op->op.value);
        if (pair == singleOperandOps.end()) {
            printf("ERROR: %s:%d:%d: unsupported op: '%s'!\n",op->token.file,op->token.line,op->token.column,op->op.value);
            return Reg::NUL;
        }

        pair->second(a);

        registers[a].value.modified = true;

        return a;
    }
    
    Reg doOp(Node* op, Reg a, Reg b) {

        switch (op->op.type) {
            case OpType::ASSIGNMENT:
                return assignmentOp(op, a, b);
            case OpType::MATH:
                return mathmaticalOp(op, a, b);
            case OpType::SINGLE_OP_POSTFIX:
            case OpType::SINGLE_OP_PREFIX: {
                Reg reg = singleOperandOp(op, a);
                if (OpType::SINGLE_OP_PREFIX) return reg;

                Reg reg2 = findFreeReg();
                registers[reg].value = Value{ValueType::INTERMEDIATE,{.symbol={nullptr}},false,false,false};
                registers[reg].position = position++;

                assign(reg2, reg);
                return reg2;
            }
        }

        return Reg::NUL;
    }

}

