#include <fstream>

#include "../parser/parser.hpp"

using namespace Parser;

namespace Compiler {

    std::ofstream* out;

    bool createFunction(Node* node) {
        out->write(node->symbol.name,strlen(node->symbol.name));
        out->write(":\n",2);

        allocateLocals(node);

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
    }


    bool compile(std::unordered_map<std::string, Node*>* tree, std::ofstream* out) {

        Compiler::out = out;

        for (auto& [key, node] : *tree) {
            switch (node->type) {
                case NodeType::FUNCTION:
                    if (!createFunction(node)) return false;
                    break;

                case NodeType::SYMBOL:
                    if (node->firstChild && !buildOperation(node->firstChild,node->symbol.t)) return false;
                    break;

                default:
                    // this can't happen
                    break;
            }
        }
        return true;
    }
}