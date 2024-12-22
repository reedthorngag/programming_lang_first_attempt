
#include "parser.hpp"
#include "../compiler/compiler.hpp"
#include "../util/debugging.hpp"

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
            printf("ERROR: %s:%d:%d: expecting function name, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (symbolDeclared(token.value, parent, nullptr)) {
            printf("ERROR: %s:%d:%d: function name ('%s') already in use!\n",token.file.name,token.file.line,token.file.col,token.value);
            return nullptr;
        }

        node->symbol->name = token.value;

        token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return nullptr;
        }

        token = tokens->at(index++);
        while (token.type != TokenType::GROUPING_END) {
            
            if (token.type == TokenType::COMMA) {
                if (node->symbol->func->params->size() == 0) {
                    printf("ERROR: %s:%d:%d: expecting paramater name, found ','!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                } else
                    token = tokens->at(index++);
            }

            if (token.type != TokenType::SYMBOL) {
                printf("ERROR: %s:%d:%d: expecting paramater name, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (symbolDeclared(token.value, node, nullptr)) {
                printf("ERROR: %s:%d:%d: paramater name ('%s') already in use!\n",token.file.name,token.file.line,token.file.col,token.value);
                return nullptr;
            }

            char* paramName = token.value;

            token = tokens->at(index++);
            if (token.type != TokenType::TYPE) {
                printf("ERROR: %s:%d:%d: expecting paramater type, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (auto type = typeMap.find(token.value); type == typeMap.end()) {
                printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file.name,token.file.line,token.file.col,token.value);
                return nullptr;
            } else {
                node->symbol->func->params->push_back(Param{paramName,type->second,Reg::NUL});
                node->symbolMap->insert(std::make_pair(paramName,new Symbol{SymbolType::VAR,paramName,{.t={type->second}},0,Reg::NUL}));
            }
                
            
            token = tokens->at(index++);
        }


        token = tokens->at(index++);
        if (token.type == TokenType::TYPE) {
            // printf("ERROR: %s:%d:%d: expecting return type, found %s!\n",token.file.name,token.file.line,token.file.column,TokenTypeMap[token.type]);
            // return nullptr;
        
            if (auto type = typeMap.find(token.value); type == typeMap.end()) {
                printf("ERROR: %s:%d:%d: unknown type '%s'!\n",token.file.name,token.file.line,token.file.col,token.value);
                return nullptr;
            } else
                node->symbol->func->returnType = type->second;

            token = tokens->at(index++);
            
        } else {
            node->symbol->func->returnType = Type::null;
        }

        if (token.type != TokenType::SCOPE_START) {
            printf("ERROR: %s:%d:%d: expecting '{', found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return nullptr;
        }


        if (parent) {
            appendChild(parent,node);
        } else {
            globals->insert(std::make_pair(node->symbol->name,node));
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
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
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
                    printf("ERROR: %s:%d:%d: unexpected comma!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                case TokenType::GROUPING_START:
                    param = processGrouping();
                    if (!param) return nullptr;
                    goto processNext;

                case TokenType::OPERATOR:
                    param = processPrefixOperator(token);
                    index--;
                    if (!param) return nullptr;
                    goto processNext;

                case TokenType::KEYWORD:
                case TokenType::SYMBOL:
                case TokenType::LITERAL: {
                    param = evaluateValue(token);
                    if (!param) return nullptr;

processNext:
                    token = tokens->at(index++);

                    if (token.type == TokenType::COMMA || token.type == TokenType::GROUPING_END) {
                        inGrouping = inGrouping && token.type != TokenType::GROUPING_END;
                        if (inGrouping) {
                            printf("ERROR: %s:%d:%d: unexpected2 %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                            return nullptr;
                        }
                        appendChild(node,param);
                        index--;
                        break;
                    }

                    if (token.type != TokenType::OPERATOR) {
                        printf("ERROR: %s:%d:%d: expecting operator, comma or close bracket, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                        return nullptr;
                    }

                    appendChild(node,operation(param,token));
                    index--;
                    break;
                }
                default:
                    if (tokens->at(index-2).type == TokenType::GROUPING_END) {
                        index -= 2;
                        break;
                    }
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return nullptr;
            }
            token = tokens->at(index++);
        }

        appendChild(parent, node);

        Node* body = new Node{};
        body->type = NodeType::SCOPE;
        body->symbolMap = new std::unordered_map<std::string, Symbol*>;

        appendChild(parent, body);

        Node* oldParent = parent;
        parent = body;

        token = tokens->at(index++);
        if (token.type != TokenType::SCOPE_START) {
            switch (token.type) {
                case TokenType::ENDLINE:
                    break;
                case TokenType::KEYWORD:
                    parent = processKeyword(token);
                    break;
                case TokenType::SYMBOL:
                    parent = processSymbol(token);
                    break;
                case TokenType::OPERATOR: {
                    Node* node = processPrefixOperator(token);
                    if (!node) {
                        parent = nullptr;
                        break;
                    }
                    appendChild(parent, node);
                    index++;
                    break;
                }
                case TokenType::FILE_END:
                    printf("ERROR: %s:%d:%d: unexpected EOF!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                default:
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return nullptr;
            }

            token = tokens->at(index-1);
            if (token.type != TokenType::ENDLINE) {
                printf("ERROR: %s:%d:%d: expected ';', found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (!parent) return nullptr;
            depth--;
            return oldParent;
        }

        return body;
    }

    Node* buildElseNode() {
        Node* body = new Node{};
        body->type = NodeType::ELSE;
        body->token = tokens->at(index);
        body->symbolMap = new std::unordered_map<std::string, Symbol*>;

        appendChild(parent, body);

        Node* oldParent = parent;
        parent = body;

        Token token = tokens->at(index++);
        if (token.type != TokenType::SCOPE_START) {
            switch (token.type) {
                case TokenType::ENDLINE:
                    break;
                case TokenType::KEYWORD:
                    parent = processKeyword(token);
                    break;
                case TokenType::SYMBOL:
                    parent = processSymbol(token);
                    index++;
                    break;
                case TokenType::OPERATOR: {
                    Node* node = processPrefixOperator(token);
                    if (!node) {
                        parent = nullptr;
                        break;
                    }
                    appendChild(parent, node);
                    break;
                }
                case TokenType::FILE_END:
                    printf("ERROR: %s:%d:%d: unexpected EOF!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                default:
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return nullptr;
            }

            token = tokens->at(index-1);
            if (token.type != TokenType::ENDLINE) {
                printf("ERROR: %s:%d:%d: expected ';', found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                return nullptr;
            }

            if (!parent) return nullptr;
            depth--;
            return oldParent;
        }

        return body;
    }

    Node* buildWhileNode() {

        Node* node = new Node{};
        node->type = NodeType::WHILE;
        node->token = tokens->at(index);
        node->parent = parent;

        Token token = tokens->at(index++);
        if (token.type != TokenType::GROUPING_START) {
            printf("ERROR: %s:%d:%d: expecting '(' found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return nullptr;
        }

        bool global = false;

        token = tokens->at(index++);

        while (token.type != TokenType::GROUPING_END) {

            Node* param = new Node{};

            // TODO: test this code properly to check its working with parentheses properly
            switch (token.type) {
                case TokenType::COMMA:
                    printf("ERROR: %s:%d:%d: unexpected comma!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                case TokenType::GROUPING_START:
                    param = processGrouping();
                    if (!param) return nullptr;
                    goto processNext;

                case TokenType::OPERATOR:
                    param = processPrefixOperator(token);
                    if (!param) return nullptr;
                    goto processNext;
                    
                case TokenType::KEYWORD:
                case TokenType::SYMBOL:
                case TokenType::LITERAL: {
                    param = evaluateValue(token);
                    if (!param) return nullptr;

processNext:
                    token = tokens->at(index++);

                    if (token.type == TokenType::GROUPING_END) {
                        appendChild(node,param);
                        index--;
                        break;
                    }

                    if (token.type != TokenType::OPERATOR) {
                        printf("ERROR: %s:%d:%d: expecting operator or close bracket, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                        return nullptr;
                    }

                    appendChild(node,operation(param,token));
                    index--;
                    break;
                }
                default:
                    if (tokens->at(index-2).type == TokenType::GROUPING_END) {
                        index -= 2;
                        break;
                    }
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return nullptr;
            }
            token = tokens->at(index++);
        }

        appendChild(parent, node);

        Node* body = new Node{};
        body->type = NodeType::SCOPE;
        body->symbolMap = new std::unordered_map<std::string, Symbol*>;

        appendChild(node, body);

        // appendChild sets parent to the first arg, so need to change it after
        body->parent = parent;

        parent = body;

        token = tokens->at(index++);

        if (token.type != TokenType::SCOPE_START) {

            switch (token.type) {
                case TokenType::ENDLINE:
                    break;
                case TokenType::KEYWORD:
                    parent = processKeyword(token);
                    break;
                case TokenType::SYMBOL:
                    parent = processSymbol(token);
                    break;
                case TokenType::OPERATOR: {
                    Node* node = processPrefixOperator(token);
                    if (!node) {
                        parent = nullptr;
                        break;
                    }
                    appendChild(parent, node);
                    break;
                }
                case TokenType::FILE_END:
                    printf("ERROR: %s:%d:%d: unexpected EOF!\n",token.file.name,token.file.line,token.file.col);
                    return nullptr;
                default:
                    printf("ERROR: %s:%d:%d: unexpected %s\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return nullptr;
            }

            if (!parent) return nullptr;
            depth--;
            return parent->parent;
        }

        return body;
    }

    bool processReturn() {

        Node* node = new Node{NodeType::RETURN,parent,nullptr,nullptr,{.symbol = nullptr},tokens->at(index),nullptr};

        appendChild(parent, node);

        if (tokens->at(index).type == TokenType::ENDLINE) {
            return true;
        }

        Node* param;

        Token token = tokens->at(index++);

        switch (token.type) {
            case TokenType::GROUPING_START: {
                param = processGrouping();
                if (!param) return false;
                goto processNext;
            }
            
            case TokenType::KEYWORD:
            case TokenType::SYMBOL:
            case TokenType::LITERAL: {
                param = evaluateValue(token);
                if (!param) return false;

processNext:
                token = tokens->at(index++);

                if (token.type == TokenType::ENDLINE) {
                    appendChild(node, param);
                    return true;
                }

                if (token.type != TokenType::OPERATOR) {
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                    return false;
                }

                appendChild(node, operation(param, token));
                break;
            }
            default:
                printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
                return false;
        }

        if (tokens->at(index-1).type != TokenType::ENDLINE) {
            Token token = tokens->at(index-1);
            printf("ERROR: %s:%d:%d: expecting ';', found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return false;
        }

        appendChild(node, param);
        return true;
    }

    bool buildDeclarationNode(Keyword type) {
        
        const char* typeStr = type==Keyword::VAR?"variable":"constant";

        Token token = tokens->at(index++);
        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting %s name, found %s!\n",token.file.name,token.file.line,token.file.col,typeStr,TokenTypeMap[token.type]);
            return false;
        }

        if ((parent && symbolDeclaredInScope(token.value, parent, nullptr)) || (!parent && symbolDeclaredGlobal(token.value,nullptr))) {
            printf("ERROR: %s:%d:%d: name '%s' already in use!\n",token.file.name,token.file.line,token.file.col,token.value);
            return false;
        }

        Token typeToken = tokens->at(index++);
        if (typeToken.type != TokenType::TYPE) {
            printf("ERROR: %s:%d:%d: expecting type, found %s!\n",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
            return false;
        }

        Type t;
        if (auto type = typeMap.find(typeToken.value); type == typeMap.end()) {
            printf("ERROR: %s:%d:%d: unknown type '%s'!\n",typeToken.file.name,typeToken.file.line,typeToken.file.col,typeToken.value);
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

            globals->insert(std::make_pair(token.value,global));
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

