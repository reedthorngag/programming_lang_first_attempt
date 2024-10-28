
#include "operations.hpp"

#include <string>

namespace Compiler {


    void assign(Reg a, Reg b) {
        if (a != b) out("mov", registers[a].subRegs[Size::QWORD], registers[b].subRegs[Size::QWORD]);
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

        if (b != Reg::RCX) swap(b, Reg::RCX);

        out("shl", registers[a].subRegs[3], registers[Reg::RCX].subRegs[0]);

        if (b != Reg::RCX) swap(b, Reg::RCX);
    }

    void shr(Reg a, Reg b) {

        if (b != Reg::RCX) swap(b, Reg::RCX);
        out("shr", registers[a].subRegs[3], registers[Reg::RCX].subRegs[0]);
        if (b != Reg::RCX) swap(b, Reg::RCX);
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

    void equal(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovz", registers[a].subRegs[3], "1");
        out("cmovnz", registers[a].subRegs[3], "0");
    }

    void lor(Reg a, Reg b) {
        out("or", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovnz", registers[a].subRegs[3], "1");
    }

    void land(Reg a, Reg b) {
        // this code is super bad and way longer than it needs to be probably, but my brain isnt working

        out("push", registers[b].subRegs[3]);

        out("cmp", registers[a].subRegs[3], "0");
        out("cmovnz", registers[a].subRegs[3], "1");
        out("cmp", registers[b].subRegs[3], "0");
        out("cmovnz", registers[b].subRegs[3], "1");

        out("test", registers[a].subRegs[3], registers[a].subRegs[3]);
        out("cmovnz", registers[b].subRegs[3], "0");
        out("cmovz", registers[b].subRegs[3], "1");

        out("pop", registers[a].subRegs[3]);
    }

    void ne(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovz", registers[a].subRegs[3], "0");
        out("cmovnz", registers[a].subRegs[3], "1");
    }

    void le(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovle", registers[a].subRegs[3], "1");
        out("cmovg", registers[a].subRegs[3], "0");
    }

    void ge(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmoge", registers[a].subRegs[3], "1");
        out("cmovg", registers[a].subRegs[3], "0");
    }

    void l(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovl", registers[a].subRegs[3], "1");
        out("cmovge", registers[a].subRegs[3], "0");
    }

    void g(Reg a, Reg b) {
        out("cmp", registers[a].subRegs[3], registers[b].subRegs[3]);
        out("cmovg", registers[a].subRegs[3], "1");
        out("cmovle", registers[a].subRegs[3], "0");
    }

    std::unordered_map<std::string, void (*)(Reg a, Reg b)> assignmentOps = {
        {"=",assign},
        {"+=",add},
        {"-=",sub},
        //{"/=",div},
        //{"*=",mul},
        //{"%=",mod},
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
        {"<<",shl},
        {">>",shr},
        //{">>>",13},
        {"+",add},
        {"-",sub},
        //{"*",mul},
        //{"/",div},
        //{"%",mod},
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
        printf("%d: %s %s %s\n",op->token.line,registers[lvalue].subRegs[Size::QWORD],op->op.value,registers[rvalue].subRegs[Size::QWORD]);
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
            case OpType::SINGLE_OP_PREFIX:
                return singleOperandOp(op, a);
        }

        return Reg::NUL;
    }

}

