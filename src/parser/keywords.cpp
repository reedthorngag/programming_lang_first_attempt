
#include "parser.hpp"

using namespace Lexer;

namespace Parser {

    Node* buildFunctionNode() {

        Node* node = new Node;
        node->type = NodeType::FUNCTION;

        node->symbolMap = new std::unordered_map<std::string, Symbol>;

        node->function = new Function;

        Token token = tokens->at(index++);
        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting function name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (symbolDeclared(token.value, parent, nullptr)) {
            printf("ERROR: %s:%d:%d: function name ('%s') already in use!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        node->function->name = token.value;


        token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        token = tokens->at(index++);
        while (token.type != TokenType::GROUPING_END) {
            
            if (token.type == TokenType::COMMA) {
                if (node->function->params.size() == 0) {
                    printf("ERROR: %s:%d:%d: expecting paramater name, found ','!\n",token.file,token.line,token.column);
                    return nullptr;
                } else
                    token = tokens->at(index++);
            }

            if (token.type != TokenType::SYMBOL) {
                printf("ERROR: %s:%d:%d: expecting paramater name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (symbolDeclared(token.value, node, nullptr)) {
                printf("ERROR: %s:%d:%d: paramater name ('%s') already in use!\n",token.file,token.line,token.column,token.value);
                return nullptr;
            }

            char* paramName = token.value;

            token = tokens->at(index++);
            if (token.type != TokenType::TYPE) {
                printf("ERROR: %s:%d:%d: expecting paramater type, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (auto type = typeMap.find(token.value); type == typeMap.end()) {
                printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file,token.line,token.column,token.value);
                return nullptr;
            } else {
                node->function->params.push_back(Param{token.value,type->second});
            }
                
            
            token = tokens->at(index++);
        }


        token = tokens->at(index++);
        if (token.type != TokenType::TYPE) {
            printf("ERROR: %s:%d:%d: expecting return type, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }
        if (auto type = typeMap.find(token.value); type == typeMap.end()) {
            printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        } else
            node->function->returnType = type->second;

        token = tokens->at(index++);
        if (token.type != TokenType::SCOPE_START) {
            printf("ERROR: %s:%d:%d: expecting '{', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }


        if (parent) {
            appendChild(parent,node);
        } else {
            globals.insert(std::make_pair(node->function->name,Symbol{SymbolType::FUNC,node->function->name,{.func = {node->function}}}));
        }

        return node;
    }

    Node* buildIfNode() {

        return nullptr;
    }

    Node* buildWhileNode() {

        return nullptr;
    }

    bool buildDeclerationNode(Keyword type) {

        const char* typeStr = type==Keyword::VAR?"variable":"constant";

        Token token = tokens->at(index++);
        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting %s name, found %s!\n",token.file,token.line,token.column,typeStr,TokenTypeMap[token.type]);
            return false;
        }

        if (symbolDeclared(token.value, parent, nullptr)) {
            printf("ERROR: %s:%d:%d: %s name %s already in use!\n",token.file,token.line,token.column,typeStr,token.value);
            return false;
        }

        Token typeToken = tokens->at(index++);
        if (token.type != TokenType::TYPE) {
            printf("ERROR: %s:%d:%d: expecting type, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return false;
        }

        Type t;
        if (auto type = typeMap.find(token.value); type == typeMap.end()) {
            printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file,token.line,token.column,token.value);
            return false;
        } else {
            t = type->second;
        }

        if (parent) {
            parent->symbolMap->insert(std::make_pair(token.value,Symbol{type==Keyword::VAR?SymbolType::VAR:SymbolType::CONST,token.value,{.t={t}}}));
        } else {
            globals.insert(std::make_pair(token.value,Symbol{type==Keyword::VAR?SymbolType::VAR:SymbolType::CONST,token.value,{.t={t}}}));
        }

        return assignment(token);
    }

}

