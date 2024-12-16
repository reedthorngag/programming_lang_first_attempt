#include <vector>
#include <unordered_map>
#include <string>

#include "../types.hpp"

#ifndef _LEXER
#define _LEXER

namespace Lexer {

    extern bool log;

    const int MAX_SYMBOL_LEN = 64;

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
    bool isOperatorChar(char c);
    inline bool validOperation(char* c, int len);
    inline bool validNumberLiteral(char c);
    bool validLiteral(char* c, int len);
    inline char* newString(char* c, int len);
    bool endSymbol(std::vector<Token>* tokens, Context* context);
    bool typeLexer(std::vector<Token>* tokens, Context* context);
    std::vector<Token>* parse(char* file, char* input);

}

#endif