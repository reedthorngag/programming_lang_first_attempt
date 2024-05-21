#include <vector>
#include <unordered_map>
#include <string>

#ifndef _LEXER
#define _LEXER

namespace Lexer {

    const int MAX_SYMBOL_LEN = 64;

    enum Keyword {
        FUNCTION,
        VARIABLE,
        CONSTANT,
        IF,
        ELSE,
        WHILE
    };

    enum TokenType {
        ENDLINE,
        BRACE,
        BRACKET,
        KEYWORD,
        TYPE,
        SYMBOL,
        OPERATOR,
        LITERAL
    };

    struct Token {
        TokenType type;
        union {
            char c;
            char* value;
            Keyword keyword;
        };
        char* file;
        int line;
        int column;
    };

    struct Context {
        char* file;
        char* input;
        char** ptr;
        int* line;
        int* column;
        int* symbolLen;
        char* symbolBuf;
    };

    void endSymbol(std::vector<Token>* tokens, Context* context);
    int typeLexer(std::vector<Token>* tokens, Context* context);
    std::vector<Token>* lexerParse(char* file, char* input);

}

#endif