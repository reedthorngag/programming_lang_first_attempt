
#include "parser.hpp"
#include "../compiler/compiler.hpp"

using namespace Lexer;

namespace Parser {

    Node* buildFunctionNode() {

        Node* node = new Node{};
        node->type = NodeType::FUNCTION;
        node->token = tokens->at(index);

        node->symbolMap = new std::unordered_map<std::string, Symbol*>;

        node->symbol = new Symbol{SymbolType::FUNC,nullptr,{.func = {new Function{}}},0,Reg::NUL};
        node->symbol->func->params = new std::vector<Param>;

        Token token = tokens->at(index++);
        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting function name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (symbolDeclared(token.value, parent, nullptr)) {
            printf("ERROR: %s:%d:%d: function name ('%s') already in use!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        node->symbol->name = token.value;

        token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        token = tokens->at(index++);
        while (token.type != TokenType::GROUPING_END) {
            
            if (token.type == TokenType::COMMA) {
                if (node->symbol->func->params->size() == 0) {
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
                node->symbol->func->params->push_back(Param{paramName,type->second,Reg::NUL});
                node->symbolMap->insert(std::make_pair(paramName,new Symbol{SymbolType::VAR,paramName,{.t={type->second}},0,Reg::NUL}));
            }
                
            
            token = tokens->at(index++);
        }


        token = tokens->at(index++);
        if (token.type == TokenType::TYPE) {
            // printf("ERROR: %s:%d:%d: expecting return type, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            // return nullptr;
        
            if (auto type = typeMap.find(token.value); type == typeMap.end()) {
                printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file,token.line,token.column,token.value);
                return nullptr;
            } else
                node->symbol->func->returnType = type->second;

            token = tokens->at(index++);
            
        } else {
            node->symbol->func->returnType = Type::null;
        }

        if (token.type != TokenType::SCOPE_START) {
            printf("ERROR: %s:%d:%d: expecting '{', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }


        if (parent) {
            appendChild(parent,node);
        } else {
            globals.insert(std::make_pair(node->symbol->name,node));
        }

        generateParamMapping(node);

        return node;
    }

    Node* buildIfNode() {

        Node* node = new Node{};
        node->type = NodeType::IF;
        node->token = tokens->at(index);
        node->parent = parent;

        Token token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        bool inGrouping = false;
        bool global = false;

        token = tokens->at(index++);

        while (token.type != TokenType::GROUPING_END) {

            Node* param = new Node{};

            // TODO: test this code properly to check its working with parentheses properly
            switch (token.type) {
                case TokenType::COMMA:
                    printf("ERROR: %s:%d:%d: unexpected comma!\n",token.file,token.line,token.column);
                    return nullptr;
                case TokenType::GROUPING_START:
                    inGrouping = true;
                    break;
                case TokenType::KEYWORD:
                    if (token.keyword != Keyword::GLOBAL) {
                        printf("ERROR: %s:%d:%d: unexpected keyword!\n",token.file,token.line,token.column);
                        return nullptr;
                    }
                    global = true;
                    token = tokens->at(index++);
                    if (token.type != TokenType::SYMBOL) {
                        printf("ERROR: %s:%d:%d: expecting name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                        return nullptr;
                    }
                    [[fallthrough]];
                case TokenType::SYMBOL: {
                    param->type = NodeType::SYMBOL;
                    Symbol* symbol;

                    if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
                        printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                        return nullptr;
                    }
                    symbol->refCount++;
                    global = false;
                    param->symbol = symbol;
                    param->token = token;
                }
                    [[fallthrough]];
                case TokenType::LITERAL: {
                    
                    if (!(int)param->type) {
                        param->type = NodeType::LITERAL;
                        param->literal = Literal{Type::null,{.value = {token.value}}};
                        param->token = token;
                    }

                    token = tokens->at(index++);

                    if (token.type == TokenType::COMMA || token.type == TokenType::GROUPING_END) {
                        inGrouping = inGrouping && token.type != TokenType::GROUPING_END;
                        if (inGrouping) {
                            printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                            return nullptr;
                        }
                        appendChild(node,param);
                        index--;
                        break;
                    }

                    if (token.type != TokenType::OPERATOR) {
                        printf("ERROR: %s:%d:%d: expecting operator, comma or close bracket, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                        return nullptr;
                    }

                    appendChild(node,operation(param,token));
                    break;
                }
                default:
                    if (tokens->at(index-2).type == TokenType::GROUPING_END) {
                        index -= 2;
                        break;
                    }
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
            }
            token = tokens->at(index++);
        }

        appendChild(parent, node);

        return parent;
    }

    Node* buildElseNode() {
        Node* node = new Node{};
        node->type = NodeType::ELSE;
        node->token = tokens->at(index);
        node->parent = parent;

        Token token = tokens->at(index++);
        if (token.type != TokenType::SCOPE_START) {
            printf("ERROR: %s:%d:%d: expecting '{', found '%s'!\n",token.file,token.line,token.column, token.value);
            return nullptr;
        }

        node->symbolMap = new std::unordered_map<std::string, Symbol*>;

        appendChild(parent, node);

        return node;
    }

    Node* buildWhileNode() {

        return nullptr;
    }

    bool processReturn() {

        Node* node = new Node{NodeType::RETURN,parent,nullptr,nullptr,{.symbol = nullptr},tokens->at(index),nullptr};

        bool inGrouping = false;
        bool global = false;

        if (tokens->at(index).type == TokenType::ENDLINE) {
            appendChild(parent, node);
            return true;
        }

        Node* param = evaluateValue();

        if (tokens->at(index-1).type != TokenType::ENDLINE) {
            Token token = tokens->at(index-1);
            printf("ERROR: %s:%d:%d: expecting ';', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return false;
        }

        appendChild(node, param);
        return true;
    }

    bool buildDeclarationNode(Keyword type) {

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
        Symbol* symbol = new Symbol{type==Keyword::VAR?SymbolType::VAR:SymbolType::CONST,token.value,{.t={t}},0,Reg::NUL};
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

