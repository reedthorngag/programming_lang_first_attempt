#include <fstream>
#include <unordered_map>
#include <cstring>

#include "../parser/parser.hpp"

using namespace Parser;

namespace Compiler {

    std::ofstream* output;

    inline void out(const char* str) {
        (*output) << str << "\n";
    }

    inline void out(const char* str, const char* arg) {
        (*output) << "    " << str << " " << arg << "\n";
    }

    inline void out(const char* str, const char* arg1, const char* arg2) {
        (*output) << "    " << str << " " << arg1 << "," << arg2 << "\n";
    }


    bool functionSetup(Node* node) {

        int spaceReq = 0;

        for (auto& [name, symbol] : *node->symbolMap) {

        }

        if (spaceReq == 0) return false;

        out("    push rbp");
        out("    mov rbp, rsp");

        return true;
    }

    bool createFunction(Node* node) {
        (*output) << node->symbol.name << ":\n";

        bool hasLocals = functionSetup(node);

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

        if (hasLocals) {
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
