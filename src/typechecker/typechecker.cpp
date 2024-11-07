#include <stdio.h>

#include "typechecker.hpp"
#include "../parser/parser.hpp"
#include "parseliteral.hpp"

using namespace Parser;

namespace TypeChecker {

    bool typesImplicitlyCompatible(Type parent, Type child) {
        if (parent == child) return true;

        if (parent == Type::error) return true;
        if (child == Type::error) return false;

        return TypeCompatibility[parent] & TypeMask[child];
    }

    Type literalType(Node* node, Type parentType) {
        if (!parseLiteral(node, parentType)) return Type::error;
        if (!typesImplicitlyCompatible(parentType,node->literal.type)) {
            printf("ERROR: %s:%d:%d: incompatible types! ('%s' and '%s')\n",node->token.file,node->token.line,node->token.column,TypeMap[parentType],TypeMap[node->literal.type]);
            return Type::error;
        }
        return node->literal.type;
    }

    Type getType(Node* node, Type parentType) {
        switch (node->type) {
            case NodeType::OPERATION:
                return processOperation(node, parentType);

            case NodeType::SYMBOL:
                return node->symbol->t;
            
            case NodeType::LITERAL:
                return literalType(node,parentType);
            
            case NodeType::INVOCATION: {
                Type type = processInvocation(node);
                if (type == Type::error) return Type::error;
                if (!typesImplicitlyCompatible(parentType,type)) {
                    printf("ERROR: %s:%d:%d: incompatible types! ('%s' and '%s')\n",node->token.file,node->token.line,node->token.column,TypeMap[parentType],TypeMap[type]);
                    return Type::error;
                }
                return type;
            }
            
            default:
                printf("ERROR: %s:%d:%d: '%s' unexpected node!\n",node->token.file,node->token.line,node->token.column,NodeTypeMap[(int)node->type]);
                return Type::error;
        }
    }

    Type processOperation(Node* node, Type parentType) {

        Node* lvalue = node->firstChild;
        if (!lvalue) {
            printf("ERROR: %s:%d:%d: operation without child!\n",node->token.file,node->token.line,node->token.column);
        }
        Type ltype = getType(lvalue, parentType);
        if (ltype == Type::error) return Type::error;

        Node* rvalue = lvalue->nextSibling;
        if (!rvalue) {
            printf("you missed an edge case idiot\n");
            return ltype;
        }
        Type rtype = getType(rvalue, ltype);
        if (rtype == Type::error) return Type::error;

        if (!typesImplicitlyCompatible(ltype,rtype)) {
            printf("ERROR: %s:%d:%d: incompatible types! ('%s' and '%s')\n",rvalue->token.file,rvalue->token.line,rvalue->token.column,TypeMap[ltype],TypeMap[rtype]);
            return Type::error;
        };

        if (!typesImplicitlyCompatible(parentType,ltype)) {
            printf("ERROR: %s:%d:%d: incompatible types! ('%s' and '%s')\n",node->token.file,node->token.line,node->token.column,TypeMap[parentType],TypeMap[ltype]);
            return Type::error;
        };
        return ltype;
    }

    Type processInvocation(Node* node) {

        int paramNum = 0;
        Node* child = node->firstChild;
        while (child) {
            child = child->nextSibling;
            paramNum++;
        }

        if (paramNum != (int)node->symbol->func->params->size()) {
            printf("ERROR: %s:%d:%d: expecting %d parameters, found %d\n",node->token.file,node->token.line,node->token.column,(int)node->symbol->func->params->size(),paramNum);
            return Type::error;
        }

        child = node->firstChild;
        for (Param p : *node->symbol->func->params) {

            Type type = getType(child,p.type);
            if (!typesImplicitlyCompatible(p.type,type)) {
                printf("ERROR: %s:%d:%d: incompatible type! (requires '%s', found '%s')\n",child->token.file,child->token.line,child->token.column,TypeMap[p.type],TypeMap[type]);
                return Type::error;
            }

            child = child->nextSibling;
        }

        return node->symbol->func->returnType;
    }

    bool processScope(Node* node) {
        Node* child = node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::IF:
                    // TODO: check it doesnt call a function that returns nothing
                    // or other such edge cases
                    break;

                case NodeType::FUNCTION:
                case NodeType::SCOPE:
                case NodeType::ELSE:
                    if (!processScope(child)) return false;
                    break;

                case NodeType::OPERATION:
                    if (processOperation(child,Type::error) == Type::error) return false;
                    break;

                case NodeType::INVOCATION:
                    if (processInvocation(child) == Type::error) return false;
                    break;

                default:
                    printf("ERROR: %s:%d:%d: '%s' unexpected node!\n",child->token.file,child->token.line,child->token.column,NodeTypeMap[(int)child->type]);
                    return false;
            }

            child = child->nextSibling;
        }
        return true;
    }

    bool process(std::unordered_map<std::string, Parser::Node*>* tree) {

        for (auto& pair : *tree) {
            switch (pair.second->type) {
                case NodeType::FUNCTION:
                    if (!processScope(pair.second)) return false;
                    break; 

                case NodeType::SYMBOL:
                    if (pair.second->firstChild && !processOperation(pair.second->firstChild,pair.second->symbol->t)) return false;
                    break;

                default:
                    printf("ERROR: %s:%d:%d: expecting declaration or function, found '%s'!\n",pair.second->token.file,pair.second->token.line,pair.second->token.column,NodeTypeMap[(int)pair.second->type]);
                    return false;
            }
        }
        return true;
    }
};

