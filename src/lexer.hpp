#include <vector>
#include <unordered_map>
#include <string>

#ifndef _LEXER
#define _LEXER

namespace Lexer {

    const int MAX_SYMBOL_LEN = 64;

    enum NumberType {
        i8, i16, i32, i64, i128,
        u8, u16, u32, u64, u128,
        f16, f32, f64
    };

    const char* maxStr[] = {
        "127", "32_767", "2_147_483_647", "9_223_372_036_854_775_807", "170_141_183_460_469_231_731_687_303_715_884_105_727",
        "255", "65_535","4_294_967_295", "18_446_744_073_709_551_615", "340_282_366_920_938_463_463_374_607_431_768_211_455"
    };

    const char* minStr[] = {
        "-127", "-32_768", "-2_147_483_648", "-9_223_372_036_854_775_808", "-170_141_183_460_469_231_731_687_303_715_884_105_728",
        "0", "0", "0", "0", "0"
    };

    struct Number {
        bool hasMinMax;
        const char* minStr;
        const char* maxStr;
    };



    enum Keyword {
        FUNC,
        VAR,
        CONST,
        IF,
        ELSE,
        WHILE
    };

    enum TokenType {
        ENDLINE,
        SCOPE_START,
        SCOPE_END,
        GROUPING_START,
        GROUPING_END,
        KEYWORD,
        SYMBOL,
        TYPE,
        OPERATOR,
        LITERAL
    };

    struct Token {
        TokenType type;
        union {
            char c[4];
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

    bool symbolChar(unsigned char c, int pos);
    bool operatorChar(char c);
    bool validLiteral(char* c, int len);
    void endSymbol(std::vector<Token>* tokens, Context* context);
    int typeLexer(std::vector<Token>* tokens, Context* context);
    std::vector<Token>* lexerParse(char* file, char* input);

}

#endif