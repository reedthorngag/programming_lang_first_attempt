#include <fstream>
#include <unordered_map>
#include <cstring>
#include <stack>

#include "compiler.hpp"
#include "../parser/parser.hpp"
#include "operations.hpp"

using namespace Parser;

namespace Compiler {

    std::ofstream* output;

    const std::unordered_map<std::string,Global> globals;

    Reg evaluate(Node* node) {
        return Reg::RAX;
    }

    bool callFunction(Node* funcCall) {

        std::stack<Node*> params;

        Node* paramNode = funcCall->firstChild;
        while (paramNode) {
            params.push(paramNode);
            paramNode = paramNode->nextSibling;
        }
        int p = funcCall->symbol.func->params->size();
        Param param;
        while (!params.empty()) {
            paramNode = params.top();
            params.pop();

            param = funcCall->symbol.func->params->at(--p);

            Reg reg = evaluate(paramNode);

            out("push",registers[reg].subRegs[TypeSizeMap[param.type]]);
        }

        for (Param p : *funcCall->symbol.func->params) {
            
            if (p.reg == Reg::STACK)
                break;
            
            if (registers[param.reg].value.type != ValueType::EMPTY) {
                printf("WARN: %s:%d:%d: register value not empty!\n",paramNode->token.file,paramNode->token.line,paramNode->token.column);
            }

            out("pop",registers[p.reg].subRegs[TypeSizeMap[p.type]]);
        }

        return true;
    }

    void generateParamMapping(Node* node) {

        Reg reg = Reg::RAX;
        for (int i = 0; i < (int)node->symbol.func->params->size(); i++) {
            if (reg == Reg::RBP) {
                node->symbol.func->params->at(i).reg = (Parser::Reg)Reg::STACK;
            } else {
                node->symbol.func->params->at(i).reg = (Parser::Reg)reg;
                reg = (Reg)(reg+1);
            }
        }
    }


    Context* functionSetup(Node* node) {

        int spaceReq = 0;

        Context* context = new Context{node,new std::unordered_map<std::string, Local>};

        for (auto& [name, symbol] : *node->symbolMap) {
            if (*symbol.refCount) {
                spaceReq += SizeByteMap[TypeSizeMap[symbol.t]];
                Local l = Local{symbol,spaceReq,TypeSizeMap[symbol.t]};
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
        (*output) << node->symbol.name << ":\n";

        Context* context = functionSetup(node);

        generateParamMapping(node);

        Node* child = node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::BLOCK:
                    break;
                case NodeType::INVOCATION:
                    callFunction(child);
                    break;
                case NodeType::OPERATION:
                    break;
                default:
                    printf("ERROR: %s:%d:%d: unexpected node: %s!\n",child->token.file,child->token.line,child->token.column,NodeTypeMap[(int)child->type]);
                    return false;
            }
            child = child->nextSibling;
        }

        if (context->locals->size()) {
            out("    mov rsp, rbp");
            out("    pop rbp");
        }
        out("    ret\n");

        return true;
    }


    bool compile(std::unordered_map<std::string, Node*>* tree, std::ofstream* output) {
        Compiler::output = output;

        (*output) << "_start:\n";
        out("    call main");

        auto key = tree->find("main");
        if (key->second->symbol.func->returnType == Type::null) {
            out("    xor rax, rax");
        }

        out("    ret\n");

        for (auto& [key, node] : *tree) {
            switch (node->type) {
                case NodeType::FUNCTION:
                    if (!createFunction(node)) return false;
                    break;

                case NodeType::SYMBOL:
                    //if (node->firstChild && !buildOperation(node->firstChild,node->symbol.t)) return false;
                    break;

                default:
                    // this can't happen
                    break;
            }
        }
        return true;
    }
};
