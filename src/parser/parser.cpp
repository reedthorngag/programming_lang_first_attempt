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
        "':'", // TYPE
        "operator", // OPERATOR
        "literal", // LITERAL
        "EOF", // FILE_END
    };

    std::unordered_map<std::string, Type> typeMap = {
        {"null",Type::null},

        {"i8",Type::u8},
        {"i16",Type::u16},
        {"i32",Type::u32},
        {"i64",Type::u64},
        {"i128",Type::u128},

        {"u8",Type::u8},
        {"u16",Type::u16},
        {"u32",Type::u32},
        {"u64",Type::u64},
        {"u128",Type::u128},

        {"f16",Type::u16},
        {"f32",Type::u32},
        {"f64",Type::u64},

        {"char",Type::c},
        {"string",Type::string},
    };

    std::unordered_map<std::string, Node*> globals;

    std::vector<Node*> unresolvedReferences(1024);

    Node* parent = nullptr;
    int depth = 0;

    std::vector<Token>* tokens;
    long long unsigned int index = 0;

    bool symbolDeclared(char* name, Node* parent) {

        return symbolDeclaredGlobal(name) || symbolDeclaredInScope(name,parent);
    }

    inline bool symbolDeclaredInScope(char* name, Node* parent) {

        Node* node = parent;
        while (node) {
            if (auto key = node->symbolMap->find(name); key != node->symbolMap->end())
                return true;
            
            node = node->parent;
        }

        return false;
    }

    inline bool symbolDeclaredGlobal(char* name) {

        auto key = globals.find(name); 
        return key != globals.end();
    }

    Node* processKeyword(Token token) {

        switch (token.keyword) {
            case Keyword::FUNC:
                depth++;
                return buildFunctionNode();
            case Keyword::IF:
                depth++;
                return buildIfNode();
            case Keyword::WHILE:
                depth++;
                return buildWhileNode();
            case Keyword::VAR:
                buildDeclerationNode(token.keyword);
                return parent;
            default:
                printf("ERROR: %s:%d:%d: keyword not yet implemented!\n",token.file,token.line,token.column);
                return nullptr;
        }
    }

    Node* processSymbol(Token token) {

        return nullptr;
    }

    Node* processOperator(Token token) {

        return nullptr;
    }

    Node* newScope(Token token) {

        return nullptr;
    }

    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* _tokens) {
        tokens = _tokens;

        while (index < tokens->size()) {
            Token token = tokens->at(index++);

            switch (token.type) {
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


