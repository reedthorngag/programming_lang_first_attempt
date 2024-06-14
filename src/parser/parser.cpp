#include <stdio.h>

#include "parser.hpp"

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
        "BLOCK",
        "SYMBOL",
        "LITERAL",
        "OPERATION",
        "INVOCATION",
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
        "string",
        "boolean",
        "null"
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
        char* f = newStr("print");
        char* p1 = newStr("value");

        Function* func = new Function{};
        func->returnType = Type::null;
        func->params = new std::vector<Param>;
        func->params->push_back(Param{p1,Type::i64,Reg::NUL});
        Symbol* sym = new Symbol{SymbolType::FUNC,f,{.func = {func}},0};

        Node* tmp = new Node{};
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

    inline bool symbolBuiltin(char* name, Symbol** symbol) {
        if (auto key = builtins.find(name); key != builtins.end()) {
            if (symbol) *symbol = key->second;
            return true;
        }
        return false;
    }

    inline bool symbolDeclaredInScope(char* name, Node* parent, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        Node* node = parent;
        while (node) {
            if (auto key = node->symbolMap->find(name); key != node->symbolMap->end()) {
                if (symbol) *symbol = key->second;
                return true;
            }
            node = node->parent;
        }

        return false;
    }

    inline bool symbolDeclaredGlobal(char* name, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        if (auto key = globals.find(name); key != globals.end()) {
            if (symbol) *symbol = key->second->symbol;
            return true;
        }
        return false;
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

            case Keyword::VAR:
            case Keyword::CONST:
                if (buildDeclerationNode(token.keyword)) return parent;
                else {
                    depth++;
                    return nullptr;
                }

            default:
                printf("ERROR: %s:%d:%d: keyword not yet implemented!\n",token.file,token.line,token.column);
                return nullptr;
        }
    }

    Node* processSymbol(Token token) {
        // assignment handles function calls (yes its bad, I'll fix at some point hopefully)
        Node* node = assignment(token);
        if (!node) {
            depth++;
            return nullptr;
        }
        appendChild(parent,node);
        return parent;
    }

    Node* processOperator(Token token) {
        printf("not implemented\n");
        return nullptr;
    }

    Node* newScope(Token token) {
        printf("not implemented\n");
        return nullptr;
    }

    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* _tokens) {
        buildBuiltins();

        tokens = _tokens;

        while (index < tokens->size()) {
            Token token = tokens->at(index++);

            switch (token.type) {
                case TokenType::ENDLINE:
                    break;
                case TokenType::KEYWORD:
                    parent = processKeyword(token);
                    break;
                case TokenType::SYMBOL:
                    parent = processSymbol(token);
                    break;
                case TokenType::OPERATOR:
                    parent = processOperator(token);
                    break;
                case TokenType::SCOPE_START:
                    parent = newScope(token);
                    break;
                case TokenType::SCOPE_END:
                    if (!depth-- || !parent) {
                        printf("ERROR: %s:%d:%d: unexpected '}'\n",token.file,token.line,token.column);
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


