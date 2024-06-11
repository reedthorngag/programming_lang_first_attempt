#include <fstream>
#include <unordered_map>
#include <cstring>

#include "compiler.hpp"
#include "../parser/parser.hpp"

using namespace Parser;

namespace Compiler {

    std::ofstream* output;

    inline void out(std::string str) {
        (*output) << str << "\n";
    }

    inline void out(std::string str, std::string arg) {
        (*output) << "    " << str << " " << arg << "\n";
    }

    inline void out(std::string str, std::string arg1, std::string arg2) {
        (*output) << "    " << str << " " << arg1 << "," << arg2 << "\n";
    }


    Context* functionSetup(Node* node) {

        int spaceReq = 0;

        Context* context = new Context{node,new std::unordered_map<std::string, Local>};

        for (auto& [name, symbol] : *node->symbolMap) {
            if (*symbol.refCount) {
                printf("thing: %s %d\n",name.c_str(),SizeByteMap[TypeSizeMap[symbol.t]]);
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

        Node* child = node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::BLOCK:
                    break;
                case NodeType::INVOCATION:
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
