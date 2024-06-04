
#include "parser.hpp"

using namespace Lexer;

namespace Parser {

    Node* buildFunctionNode() {

        Node* node = new Node{};
        node->type = NodeType::FUNCTION;

        node->symbolMap = new std::unordered_map<std::string, Symbol>;

        node->symbol = Symbol{SymbolType::FUNC,nullptr,{.func = {new Function}}};

        Token token = tokens->at(index++);
        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting function name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (symbolDeclared(token.value, parent, nullptr)) {
            printf("ERROR: %s:%d:%d: function name ('%s') already in use!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        node->symbol.name = token.value;

        token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        token = tokens->at(index++);
        while (token.type != TokenType::GROUPING_END) {
            
            if (token.type == TokenType::COMMA) {
                if (node->symbol.func->params.size() == 0) {
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
                node->symbol.func->params.push_back(Param{paramName,type->second});
                node->symbolMap->insert(std::make_pair(paramName,Symbol{SymbolType::VAR,paramName,{.t={type->second}}}));
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
            node->symbol.func->returnType = type->second;

        token = tokens->at(index++);
        if (token.type != TokenType::SCOPE_START) {
            printf("ERROR: %s:%d:%d: expecting '{', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }


        if (parent) {
            appendChild(parent,node);
        } else {
            globals.insert(std::make_pair(node->symbol.name,node));
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

        if ((parent && symbolDeclaredInScope(token.value, parent, nullptr)) || (!parent && symbolDeclaredGlobal(token.value,nullptr))) {
            printf("ERROR: %s:%d:%d: name '%s' already in use!\n",token.file,token.line,token.column,token.value);
            return false;
        }

        Token typeToken = tokens->at(index++);
        if (typeToken.type != TokenType::TYPE) {
            printf("ERROR: %s:%d:%d: expecting type, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return false;
        }

        Type t;
        if (auto type = typeMap.find(typeToken.value); type == typeMap.end()) {
            printf("ERROR: %s:%d:%d: unknown type '%s'!\n",typeToken.file,typeToken.line,typeToken.column,typeToken.value);
            return false;
        } else {
            t = type->second;
        }

        Node* global;
        Symbol symbol = Symbol{type==Keyword::VAR?SymbolType::VAR:SymbolType::CONST,token.value,{.t={t}}};
        if (parent) {
            parent->symbolMap->insert(std::make_pair(token.value,symbol));
        } else {
            global = new Node{};
            global->type = NodeType::SYMBOL;
            global->symbol = symbol;
            global->token = token;

            globals.insert(std::make_pair(token.value,global));
        }

        Node* node = assignment(token);
        if (!node) return false;
        if (node->type != NodeType::SYMBOL) {
            if (parent)
                appendChild(parent,node);
            else
                appendChild(global,node);
        }
        return true;
    }

}

