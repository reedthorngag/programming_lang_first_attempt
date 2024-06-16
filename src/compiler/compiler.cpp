#include <fstream>
#include <unordered_map>
#include <cstring>
#include <stack>

#include "compiler.hpp"
#include "../parser/parser.hpp"
#include "operations.hpp"

using namespace Parser;

namespace Compiler {

    bool buildScope(Context* parent); // who needs header files anyway?

    std::ofstream* output;

    const std::unordered_map<std::string,Global> globals;

    const char* toLocalLabel(const char* label) {
        int len = 0;
        while (label[len++]);
        char* str = new char[len+1];
        str[0] = '.';
        str++;
        while (len--) str[len] = label[len];
        str--;
        return str;
    }

    bool freeReg(Reg reg) {

        Value value = registers[reg].value;

        switch (value.type) {
            case ValueType::EMPTY:
                return true;

            case ValueType::GLOBAL:
                out("mov",
                        value.global->symbol->name,
                        registers[reg].subRegs[TypeSizeMap[value.global->symbol->t]]
                    );
                value.global->symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;
            
            case ValueType::PARAMETER:
            case ValueType::LOCAL:
                out("mov",
                        refLocalVar(value.local),
                        registers[reg].subRegs[TypeSizeMap[value.local->size]]
                    );
                value.local->symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;

            default:
                return false;
        }
    }

    Reg findFreeReg() {
        Reg reg = Reg::RAX;
        for (; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.type == ValueType::EMPTY) return reg;
        }

        int minRefCount = INT_MAX;
        Reg minRefCountReg;

        for (; reg != Reg::NUL; reg = (Reg)(reg-1)) {
            Value value = registers[reg].value;
            switch (value.type) {
                case ValueType::GLOBAL:
                    if (value.global->symbol->refCount < minRefCount) {
                        minRefCount = value.global->symbol->refCount;
                        minRefCountReg = reg;
                    }
                    break;
                
                case ValueType::PARAMETER:
                case ValueType::LOCAL:
                    if (value.local->symbol->refCount < minRefCount) {
                        minRefCount = value.local->symbol->refCount;
                        minRefCountReg = reg;
                    }
                    break;
                
                default:
                    break;
            }
        }

        freeReg(minRefCountReg); // dont need to check it succeeds as we already only get available regs

        return reg;
    }

    Reg evaluate(Node* node, Context* context) {

        switch (node->type) {
            case NodeType::SYMBOL:
                return findFreeReg();
            
            case NodeType::LITERAL: {
                Reg reg = findFreeReg();
                if (node->literal.type == Type::string) {
                    // do something special
                }
                out()
            }
                
        }

        return Reg::RAX;
    }

    Reg callFunction(Node* funcCall, Context* context) {

        std::stack<Node*> params;

        Node* paramNode = funcCall->firstChild;
        while (paramNode) {
            params.push(paramNode);
            paramNode = paramNode->nextSibling;
        }
        int p = funcCall->symbol->func->params->size();
        Param param;
        while (!params.empty()) {
            paramNode = params.top();
            params.pop();

            param = funcCall->symbol->func->params->at(--p);

            Reg reg = evaluate(paramNode,context);

            out("push",registers[reg].subRegs[3]);
        }

        for (Param p : *funcCall->symbol->func->params) {
            printf("hello: %s\n",registers[p.reg].subRegs[3]);
            if ((Reg)p.reg == Reg::STACK)
                break;
            
            if (registers[param.reg].value.type != ValueType::EMPTY) {
                printf("WARN: %s:%d:%d: register value not empty!\n",paramNode->token.file,paramNode->token.line,paramNode->token.column);
            }

            out("pop",registers[p.reg].subRegs[3]);
        }

        out("call",funcCall->symbol->name);

        return funcCall->symbol->func->returnType == Type::null ? Reg::NUL : Reg::RAX;
    }

    Context* functionSetup(Node* node) {

        int spaceReq = 0;

        Context* context = new Context{node,new std::unordered_map<std::string, Local>};

        for (auto& [name, symbol] : *node->symbolMap) {
            if (symbol->refCount) {
                spaceReq += SizeByteMap[TypeSizeMap[symbol->t]];
                Local l = Local{symbol,spaceReq,TypeSizeMap[symbol->t]};
                context->locals->insert(std::make_pair(name,l));
            }
        }

        if (spaceReq == 0) return context;

        out("    push rbp");
        out("    mov rbp, rsp");
        out("sub","rsp",std::to_string(spaceReq));

        return context;
    }

    bool createFunction(Node* node) {
        (*output) << node->symbol->name << ":\n";

        Context* context = functionSetup(node);

        if (buildScope(context)) return false;

        if (context->locals->size()) {
            out("    mov rsp, rbp");
            out("    pop rbp");
        }
        out("    ret\n");

        return true;
    }

    // TODO: rename to something that makes more sense
    bool buildScope(Context* context) {

        Node* child = context->node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::BLOCK:
                    break;
                case NodeType::INVOCATION:
                    callFunction(child, context);
                    break;
                case NodeType::OPERATION:
                    evaluate(child, context);
                    break;
                default:
                    printf("ERROR: %s:%d:%d: unexpected node: %s!\n",child->token.file,child->token.line,child->token.column,NodeTypeMap[(int)child->type]);
                    return false;
            }
            child = child->nextSibling;
        }

        return true;
    }


    bool compile(std::unordered_map<std::string, Node*>* tree, std::ofstream* output) {
        Compiler::output = output;

        (*output) << "_start:\n";
        out("    call main");

        auto key = tree->find("main");
        if (key->second->symbol->func->returnType == Type::null) {
            out("    xor rax, rax");
        }

        out("    ret\n");

        for (auto& [key, node] : *tree) {
            switch (node->type) {
                case NodeType::FUNCTION:
                    if (!createFunction(node)) return false;
                    break;

                case NodeType::SYMBOL:
                    //if (node->firstChild && !buildOperation(node->firstChild,node->symbol->t)) return false;
                    break;

                default:
                    // this can't happen
                    break;
            }
        }
        return true;
    }
};
