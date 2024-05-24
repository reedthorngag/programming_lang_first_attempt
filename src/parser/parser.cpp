#include <stdio.h>

#include "parser.hpp"

using namespace Lexer;

namespace Parser {

    const char* TokenTypeMap[]{
        "';'", // ENDLINE
        "'{'", // SCOPE_START
        "'}'", // SCOPE_END
        "'('", // GROUPING_START
        "')'", // GROUPING_END
        "keyword", // KEYWORD
        "symbol", // SYMBOL
        "':'", // TYPE
        "operator", // OPERATOR
        "literal" // LITERAL
    };

    std::unordered_map<std::string, Node*> globals;

    std::vector<Node*> unresolvedReferences(1024);

    Node* parent = nullptr;
    int depth = 0;

    std::vector<Token>* tokens;
    long long unsigned int index = 0;


    Node* processKeyword(Token token) {

        switch (token.keyword) {
            case Keyword::FUNC:
                
        }

        return nullptr;
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


