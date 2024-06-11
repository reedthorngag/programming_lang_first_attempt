#include <fstream>
#include <unordered_map>
#include <cstring>

#include "../parser/parser.hpp"

using namespace Parser;

namespace Compiler {

    std::ofstream* output;

    inline void out(const char* str) {
        *output << str;
    }

    inline void out(const char* str, const char* arg) {
        *output << str << " " << arg;
    }

    inline void out(const char* str, const char* arg1, const char* arg2) {
        *output << str << " " << arg1 << "," << arg2;
    }


    void functionSetup(Node* node) {
        out("push rbp");

        int spaceReq = 0;

        for (auto& [name, symbol] : *node->symbolMap) {

        }

        if (spaceReq == 0) return;
    }

    bool createFunction(Node* node) {
        out(node->symbol.name);
        out(":\n");

        functionSetup(node);

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
        }
        return true;
    }


    bool compile(std::unordered_map<std::string, Node*>* tree, std::ofstream* out) {

        Compiler::output = out;

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
