#include <stdio.h>

#include "../parser/parser.hpp"

using namespace Parser;

namespace TypeChecker {

    bool typesImplicitlyCompatible(Type parent, Type child) {

        return true;
    }

    Type literalType(Node* node) {
        
    }

    Type getType(Node* node) {
        switch (node->type) {
            case NodeType::OPERATION:
                return processOperation(node);

            case NodeType::SYMBOL:
                return node->symbol.t;
            
            case NodeType::LITERAL:
                return literalType(node);
            
            default:
                printf("ERROR: %s:%d:%d: '%s' unexpected node!\n",node->token.file,node->token.line,node->token.column,NodeTypeMap[(int)node->type]);
                return Type::error;
        }
    }

    Type processOperation(Node* node) {

        Node* lvalue = node->firstChild;
        if (!lvalue) {
            printf("ERROR: %s:%d:%d: operation without child!\n",node->token.file,node->token.line,node->token.column);
        }
        Type ltype = getType(lvalue);
        if (ltype == Type::error) return Type::error;

        Node* rvalue = lvalue->nextSibling;
        if (!rvalue) {
            printf("you missed an edge case idiot\n");
            return ltype;
        }
        Type rtype = getType(rvalue);
        if (rtype == Type::error) return Type::error;

        if (!typesImplicitlyCompatible(ltype,rtype)) return Type::error;

        return ltype;
    }

    Type processInvocation(Node* node) {
        printf("invocation\n");
        return node->symbol.func->returnType;
    }

    bool processBlock(Node* node) {
        Node* child = node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::FUNCTION:
                case NodeType::BLOCK:
                    if (!processBlock(child)) return false;

                case NodeType::OPERATION:
                    if (processOperation(child) == Type::error) return false;

                case NodeType::INVOCATION:
                    if (processInvocation(child) == Type::error) return false;

                default:
                    printf("ERROR: %s:%d:%d: '%s' unexpected node!\n",node->token.file,node->token.line,node->token.column,NodeTypeMap[(int)node->type]);
                    return false;
            }

            child = child->nextSibling;
        }
        return true;
    }

    bool process(std::unordered_map<std::string, Parser::Node*>* tree) {

        printf("hello from the type checker\n");

        for (auto& [key, node] : *tree) {
            switch (node->type) {
                case NodeType::FUNCTION:
                    break;
                case NodeType::SYMBOL:
                    break;
                default:
                    printf("ERROR: %s:%d:%d: expecting decleration or function, found '%s'!\n",node->token.file,node->token.line,node->token.column,NodeTypeMap[(int)node->type]);
                    return false;
            }
        }
        return true;
    }
}

