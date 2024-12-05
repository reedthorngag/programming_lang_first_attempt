#include <vector>
#include <unordered_map>
#include <string>

#ifndef _LEXER
#define _LEXER

namespace Lexer {

    extern bool log;

    const int MAX_SYMBOL_LEN = 64;

    enum NumberType {
        i8, i16, i32, i64,
        u8, u16, u32, u64,
        f16, f32, f64
    };

    enum Keyword {
        FUNC,
        VAR,
        CONST,
        GLOBAL,
        IF,
        ELSE,
        WHILE,
        RETURN,
        BREAK,
        CONTINUE
    };

    enum TokenType {
        ENDLINE,
        COMMA,
        SCOPE_START,
        SCOPE_END,
        GROUPING_START,
        GROUPING_END,
        KEYWORD,
        SYMBOL,
        TYPE,
        OPERATOR,
        LITERAL,
        FILE_END,
        ARRAY_START,
        ARRAY_END
    };

    struct File {
        char* name;
        int line;
        int col;
    };

    struct Token {
        TokenType type;
        union {
            char* value;
            Keyword keyword;
        };
        File file;
        bool negative;
    };

    struct Context {
        char* file;
        char* input;
        char** ptr;
        int* line;
        int* column;
        int* symbolLen;
        char* symbolBuf;

        bool* specialType;
        int* isOperator;
        int* isLiteral;
        int* isSymbol;
    };

    struct Number {
        bool hasMinMax;
        const char* minStr;
        const char* maxStr;
    };

    extern const char* maxStr[];
    extern const char* maxStr[];

    extern File file;

    extern char* ptr;

    extern std::vector<Token>* tokens;

    extern std::unordered_map<std::string,Keyword> keywordMap;
    extern std::unordered_map<std::string,bool> operations;
    extern std::unordered_map<std::string, std::string> builtinLiteralTypes;
    extern std::unordered_map<std::string, int> literalTypesMap;
    extern std::unordered_map<std::string, Number> numberTypes;

    bool parseSymbol();
    bool parseComment();
    bool parseLiteral();
    bool parseOperator();
    bool parseType();

    bool parseStr();
    bool parseChr();

    bool isSymbolChar(char c, int pos);
    inline bool validSymbol(char* c, int len);
    bool operatorChar(char c);
    inline bool validOperation(char* c, int len);
    inline bool validNumberLiteral(char c);
    bool validLiteral(char* c, int len);
    inline char* newString(char* c, int len);
    bool endSymbol(std::vector<Token>* tokens, Context* context);
    bool typeLexer(std::vector<Token>* tokens, Context* context);
    std::vector<Token>* parse(char* file, char* input);

}

#endif