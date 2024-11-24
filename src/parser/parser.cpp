#include <stdio.h>
#include <cstring>

#include "parser.hpp"
#include "../util/debugging.hpp"

using namespace Lexer;

namespace Parser {

    const char* TokenTypeMap[]{
        "';'", // ENDLINE
        "','", // COMMA
        "'{'", // SCOPE_START
        "'}'", // SCOPE_END
        "'('", // GROUPING_START
        "')'", // GROUPING_END
        "keyword", // KEYWORD
        "symbol", // SYMBOL
        "type", // TYPE
        "operator", // OPERATOR
        "literal", // LITERAL
        "EOF", // FILE_END
    };

    const char* NodeTypeMap[]{
        "FUNCTION",
        "SCOPE",
        "SYMBOL",
        "LITERAL",
        "OPERATION",
        "INVOCATION",
        "IF",
        "ELSE",
        "RETURN",
        "WHILE",
        "BREAK",
        "CONTINUE"
    };

    const char* TypeMap[] {
        "error",
        "i8",
        "i16",
        "i32",
        "i64",
        "u8",
        "u16",
        "u32",
        "u64",
        "f16",
        "f32",
        "f64",
        "chr",
        "str",
        "bool",
        "null"
    };

    const char* OpTypeMap[]{
        "ASSIGNMENT",
        "SINGLE_OP_PREFIX",
        "SINGLE_OP_POSTFIX",
        "MATH",
        "CAST"
    };

    const char* SizeTypeMap[]{
        "byte",
        "word",
        "dword",
        "qword"
    };

    std::unordered_map<std::string, Type> typeMap = {
        {"null",Type::null},

        {"i8",Type::i8},
        {"i16",Type::i16},
        {"i32",Type::i32},
        {"i64",Type::i64},

        {"u8",Type::u8},
        {"u16",Type::u16},
        {"u32",Type::u32},
        {"u64",Type::u64},

        {"f16",Type::f16},
        {"f32",Type::f32},
        {"f64",Type::f64},

        {"char",Type::chr},
        {"string",Type::string},
        {"null",Type::null},
        {"bool",Type::boolean}
    };

    std::unordered_map<std::string, Symbol*> builtins;

    std::unordered_map<std::string, Node*> globals;

    //std::vector<Node*> unresolvedReferences(1024);

    Node* parent = nullptr;
    int depth = 0;

    std::vector<Token>* tokens;
    long long unsigned int index = 0;

    inline char* newStr(const char* c) {
        int len = 0;
        while (c[len++]);
        char* str = new char[len];
        while (len--) str[len] = c[len];
        return str;
    }

    void generateParamMapping(Node* node) {

        Reg reg = Reg::RAX;
        for (int i = 0; i < (int)node->symbol->func->params->size(); i++) {
            if (reg == Reg::RBP) {
                node->symbol->func->params->at(i).reg = (Parser::Reg)Reg::STACK;
            } else {
                node->symbol->func->params->at(i).reg = (Parser::Reg)reg;
                reg = (Reg)(reg+1);
            }
        }
    }

    void buildBuiltins() {
        char* f = newStr("printNum");
        char* p1 = newStr("value");

        Function* func = new Function{};
        func->returnType = Type::null;
        func->params = new std::vector<Param>;
        func->params->push_back(Param{p1,Type::i64,Reg::NUL});
        Symbol* sym = new Symbol{SymbolType::FUNC,f,{.func = {func}},0,Reg::NUL};

        Node* tmp = new Node{};
        tmp->symbol = sym;

        generateParamMapping(tmp);
        delete tmp;

        builtins.insert(std::make_pair(f,sym));


        f = newStr("printStr");
        p1 = newStr("string");

        func = new Function{};
        func->returnType = Type::null;
        func->params = new std::vector<Param>;
        func->params->push_back(Param{p1,Type::string,Reg::NUL});
        sym = new Symbol{SymbolType::FUNC,f,{.func = {func}},0,Reg::NUL};

        tmp = new Node{};
        tmp->symbol = sym;

        generateParamMapping(tmp);
        delete tmp;

        builtins.insert(std::make_pair(f,sym));

        f = newStr("printLine");
        p1 = newStr("string");

        func = new Function{};
        func->returnType = Type::null;
        func->params = new std::vector<Param>;
        func->params->push_back(Param{p1,Type::string,Reg::NUL});
        sym = new Symbol{SymbolType::FUNC,f,{.func = {func}},0,Reg::NUL};

        tmp = new Node{};
        tmp->symbol = sym;

        generateParamMapping(tmp);
        delete tmp;

        builtins.insert(std::make_pair(f,sym));
    }

    void appendChild(Node* parent, Node* child) {
        Node* sibling = parent->firstChild;
        if (sibling) {
            while (sibling->nextSibling) sibling = sibling->nextSibling;
            child->parent = parent;
            sibling->nextSibling = child;
        } else {
            child->parent = parent;
            parent->firstChild = child;
        }
    }

    bool symbolDeclared(char* name, Node* parent, Symbol** symbol) {
        return symbolDeclaredGlobal(name,symbol) || symbolDeclaredInScope(name,parent,symbol);
    }

    bool symbolBuiltin(char* name, Symbol** symbol) {
        auto key = builtins.find(name); 
        if (key != builtins.end()) {
            if (symbol) *symbol = key->second;
            return true;
        }
        return false;
    }

    inline bool symbolDeclaredInScope(char* name, Node* parent, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        Node* node = parent;
        while (node) {
            auto key = node->symbolMap->find(name);
            if (key != node->symbolMap->end()) {
                if (symbol) *symbol = key->second;
                return true;
            }
            node = node->parent;
        }

        return false;
    }

    inline bool symbolDeclaredGlobal(char* name, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        auto key = globals.find(name); 
        if (key != globals.end()) {
            if (symbol) *symbol = key->second->symbol;
            return true;
        }
        return false;
    }

    Node* evaluateValue(Token token) {

        Node* node = new Node{};

        bool global = false;
        switch (token.type) {

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
                node->type = NodeType::SYMBOL;
                Symbol* symbol;

                if (!(!global && (symbolDeclaredInScope(token.value,parent,&symbol)
                            || symbolDeclaredGlobal(token.value,&symbol)
                            || symbolBuiltin(token.value,&symbol)))
                        && !(global && (symbolDeclaredGlobal(token.value,&symbol) || symbolBuiltin(token.value, &symbol)))) {
                    printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                    return nullptr;
                }
                symbol->refCount++;

                if (tokens->at(index).type == TokenType::GROUPING_START) {
                    delete node;
                    index++; // functionCall func expects index to be pointing at token after (
                    node = functionCall(symbol);
                    if (!node) return nullptr;
                    break;
                }

                node->symbol = symbol;
                node->token = token;
                break;
            }

            case TokenType::LITERAL:
                node->type = NodeType::LITERAL;
                node->literal = Literal{Type::null,token.value,token.negative,{._int = 0}};
                node->token = token;
                break;

            default:
                printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                return nullptr;
        }

        return node;
    }

    Node* processCast() {

        Token token = tokens->at(index);
        if (token.type == TokenType::TYPE) {

            Node* node = new Node{NodeType::OPERATION,nullptr,nullptr,nullptr,{.op = Operator{OpType::CAST,token.value}},token,nullptr};

            if (tokens->at(++index).type != TokenType::GROUPING_END) {
                printf("ERROR: %s:%d:%d: expecting ')' to end type cast!\n",token.file,token.line,token.column);
                return nullptr;
            }

            index++;

            token = tokens->at(index++);

            Node* param;
            switch (token.type) {
                case TokenType::OPERATOR:
                    param = processPrefixOperator(token);
                    if (!param) return nullptr;
                    appendChild(node,param);
                    goto processNext;

                case TokenType::GROUPING_START:
                    param = processGrouping();
                    if (!param) return nullptr;
                    appendChild(node,param);
                    goto processNext;

                case TokenType::LITERAL:
                case TokenType::SYMBOL:
                case TokenType::KEYWORD:
                    param = evaluateValue(token);
                    if (!param) return nullptr;
                    appendChild(node,param);

processNext:
                    token = tokens->at(index++);

                    if (token.type == TokenType::COMMA || token.type == TokenType::GROUPING_END || token.type == TokenType::ENDLINE || token.type == TokenType::GROUPING_END) {
                        return node;
                        index--;
                        break;
                    }

                    if (token.type != TokenType::OPERATOR) {
                        printf("ERROR: %s:%d:%d: expecting operator, comma or close bracket, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                        return nullptr;
                    }

                    return operation(node,token);
            }

            return node;
        }

        return parent;
    }

    Node* processKeyword(Token token) {

        switch (token.keyword) {
            case Keyword::FUNC:
                depth++;
                return buildFunctionNode();

            case Keyword::IF:
                if (!parent) {
                    printf("ERROR: %s:%d:%d: only variable and function definitions allowed in global scope!\n",token.file,token.line,token.column);
                    depth++;
                    return nullptr;
                }
                depth++;
                return buildIfNode();
            
            case Keyword::ELSE:
                if (!parent) {
                    printf("ERROR: %s:%d:%d: only variable and function definitions allowed in global scope!\n",token.file,token.line,token.column);
                    depth++;
                    return nullptr;
                }
                depth++;
                return buildElseNode();

            case Keyword::WHILE:
                if (!parent) {
                    printf("ERROR: %s:%d:%d: only variable and function definitions allowed in global scope!\n",token.file,token.line,token.column);
                    depth++;
                    return nullptr;
                }
                depth++;
                return buildWhileNode();

            
            case Keyword::GLOBAL: {
                if (!parent) {
                    printf("ERROR: %s:%d:%d: only variable and function definitions allowed in global scope!\n",token.file,token.line,token.column);
                    depth++;
                    return nullptr;
                }
                Token t = tokens->at(index);
                if (t.type != TokenType::SYMBOL) {
                    printf("ERROR: %s:%d:%d: expecting name, found %s!\n",t.file,t.line,t.column,TokenTypeMap[token.type]);
                    return nullptr;
                }
                Node* node = assignment(token);
                if (!node) {
                    depth++;
                    return nullptr;
                }
                appendChild(parent,node);
                return parent;
            }

            case Keyword::RETURN:
                if (!processReturn()) return nullptr;
                return parent;

            case Keyword::VAR:
            case Keyword::CONST:
                if (buildDeclarationNode(token.keyword)) return parent;
                else {
                    depth++;
                    return nullptr;
                }

            case Keyword::BREAK:
            case Keyword::CONTINUE: {
                Node* node = new Node{token.keyword == Keyword::BREAK ? NodeType::BREAK : NodeType::CONTINUE,nullptr,nullptr,nullptr,{.symbol={nullptr}},token,nullptr};
                appendChild(parent,node);

                token = tokens->at(index++);
                if (token.type != TokenType::ENDLINE) {
                    printf("ERROR: %s:%d:%d: unexpected %s, expecting ';'!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
                }
                return parent;
            }

            default:
                printf("ERROR: %s:%d:%d: keyword not yet implemented!\n",token.file,token.line,token.column);
                return nullptr;
        }
    }

    Node* processSymbol(Token token) {
        // assignment handles function calls (yes its not ideal, I'll fix at some point hopefully)
        Node* node = assignment(token);
        if (!node) {
            depth++;
            return nullptr;
        }
        appendChild(parent,node);
        return parent;
    }

    Node* processPrefixOperator(Token token) {
        
        Node* node = new Node{};
        node->type = NodeType::OPERATION;
        node->token = token;

        node->op = Operator{token.value, getOpType(token.value)};

        if (token.value[0] == '-' && strlen(token.value) == 1) {
            if (tokens->at(index).type == TokenType::LITERAL) {
                token = tokens->at(index++);
                node->type = NodeType::LITERAL;
                node->literal = Literal{Type::null,token.value,true,{._int = 0}};
                node->token = token;
                goto nodeNegativeLiteral;
            }
        }

        if (node->op.type != OpType::SINGLE_OP_PREFIX) {
            printf("ERROR: %s:%d:%d: invalid operation! Missing lvalue!\n",token.file,token.line,token.column);
            return nullptr;
        }

        token = tokens->at(index++);

        if (token.type == TokenType::ENDLINE) {
            printf("ERROR: %s:%d:%d: expecting value, found ';'!\n",token.file,token.line,token.column);
            return nullptr;
        }

        Node* value = evaluateValue(token);
        if (!value) return nullptr;

        if (strlen(node->op.value) == 2 && value->type != NodeType::SYMBOL) {
            printf("ERROR: %s:%d:%d: operand must be a modifiable value! (aka a variable)\n",token.file,token.line,token.column);
            return nullptr;
        }

        appendChild(node, value);

nodeNegativeLiteral:
        token = tokens->at(index++);

        switch (token.type) {
            case TokenType::COMMA:
            case TokenType::ENDLINE:
            case TokenType::GROUPING_END:
                return node;

            case TokenType::OPERATOR:
                return operation(node, token);

            default:
                printf("ERROR: %s:%d:%d: unexpected token %s!\n",token.file,token.line,token.column, TokenTypeMap[token.type]);
                return nullptr;
        }

        return nullptr;
    }

    Node* newScope(Token token) {
        
        Node* node = new Node{};
        node->type = NodeType::SCOPE;
        node->parent = parent;

        node->symbolMap = new std::unordered_map<std::string, Symbol*>;

        appendChild(parent, node);

        depth++;
        return node;
    }

    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* _tokens) {
        buildBuiltins();

        tokens = _tokens;

        const char* SymbolTypeMap[]{
            "FUNC",
            "VAR",
            "CONST",
            "GLOBAL",
            "IF",
            "ELSE",
            "WHILE",
            "RETURN",
            "BREAK",
            "RETURN"
        };
        while (index < tokens->size()) {
            Token token = tokens->at(index++);
            state(token);
            //printf("token: %s",TokenTypeMap[token.type]);
            // if (token.type == TokenType::KEYWORD) printf(" %d\n",token.keyword);
            // else printf("\n");
            
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
                case TokenType::SCOPE_START:
                    parent = newScope(token);
                    break;
                case TokenType::SCOPE_END:
                    if (!depth-- || !parent) {
                        printf("ERROR: %s:%d:%d: unexpected '}' %d %d\n",token.file,token.line,token.column,depth,!parent);
                        return nullptr;
                    }
                    parent = parent->parent;
                    break;
                case TokenType::FILE_END:
                    break;
                default:
                    printf("ERROR: %s:%d:%d: unexpected %s\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
            }

            if (!parent && depth) {
                return nullptr;
            }
        }

        return &globals;
    }

}


