
#include "lexer.hpp"

namespace Lexer {

    std::unordered_map<std::string,Keyword> keywordMap = {
        {
            "func",Keyword::FUNC
        },
        {
            "var",Keyword::VAR
        },
        {
            "const",Keyword::CONST
        },
        {
            "if",Keyword::IF
        },
        {
            "else",Keyword::ELSE
        },
        {
            "while",Keyword::WHILE
        }
        
    };

    std::unordered_map<std::string,bool> operations = {
        {"+",true},
        {"-",true},
        {"*",true},
        {"/",true},
        {"=",true},
        {"+=",true},
        {"-=",true},
        {"/=",true},
        {"*=",true},
        {"%=",true},
        {"<<=",true},
        {">>=",true},
        {"^=",true},
        {"&=",true},
        {"|=",true},
        {"%",true},
        {"&",true},
        {"!",true},
        {"~",true},
        {"|",true},
        {"^",true},
        {"&",true},
        {"||",true},
        {"&&",true},
        {"==",true},
        {"!=",true},
        {"<=",true},
        {">=",true},
        {">",true},
        {"<",true},
        {"++",true},
        {"--",true},
        {"<<",true},
        {">>",true},
        {">>>",true},
    };

    std::unordered_map<std::string, Number> numberTypes = {
        {"i8",Number{true,minStr[NumberType::i8],maxStr[NumberType::i8]}},
        {"i16",Number{true,minStr[NumberType::i16],maxStr[NumberType::i16]}},
        {"i32",Number{true,minStr[NumberType::i32],maxStr[NumberType::i32]}},
        {"i64",Number{true,minStr[NumberType::i64],maxStr[NumberType::i64]}},
        {"i128",Number{true,minStr[NumberType::i128],maxStr[NumberType::i128]}},

        {"u8",Number{true,minStr[NumberType::u8],maxStr[NumberType::u8]}},
        {"u16",Number{true,minStr[NumberType::u16],maxStr[NumberType::u16]}},
        {"u32",Number{true,minStr[NumberType::u32],maxStr[NumberType::u32]}},
        {"u64",Number{true,minStr[NumberType::u64],maxStr[NumberType::u64]}},
        {"u128",Number{true,minStr[NumberType::u128],maxStr[NumberType::u128]}},

        {"f16",Number{false}},
        {"f32",Number{false}},
        {"f64",Number{false}},
    };

    std::unordered_map<std::string, int> literalTypesMap = {
        {"MIN",0},
        {"MAX",1}
    };

    std::vector<Token>* lexerParse(char* file, char* input) {

        std::vector<Token>* tokens = new std::vector<Token>;
        
        char* ptr = input;
        int line = 0;
        int col = 0;
        int symbolLen = 0;
        char symbolBuf[MAX_SYMBOL_LEN];

        struct Context context = {
            file,
            input,
            &ptr,
            &line,
            &col,
            &symbolLen,
            symbolBuf,
        };

        while (*ptr) {




            switch (*ptr) {
                case '\r': break;
                case '\n':
                    endSymbol(tokens,&context);
                    line++;
                    col = 0;
                    break;
                case ':':
                    endSymbol(tokens,&context);
                    if (typeLexer(tokens,&context)) return nullptr;
                    continue;
                case ';':
                    endSymbol(tokens,&context);
                    tokens->push_back(Token{TokenType::ENDLINE,{},file,line,col});
                    break;
                case '(':
                case ')':
                    endSymbol(tokens,&context);
                    tokens->push_back(Token{*ptr=='('?TokenType::GROUPING_START:TokenType::GROUPING_END,{},file,line,col});
                    break;
                case '{':
                case '}':
                    endSymbol(tokens,&context);
                    tokens->push_back(Token{*ptr=='{'?TokenType::SCOPE_START:TokenType::SCOPE_END,{},file,line,col});
                    break;
                case ' ':
                    endSymbol(tokens,&context);
                    break;
                default:
                    if (auto op = operatorMap.find(*ptr); op != operatorMap.end()) {
                        endSymbol(tokens,&context);
                        tokens->push_back(Token{TokenType::OPERATOR,{.c={*ptr}},file,line,col});
                    } else {
                        if (symbolLen == MAX_SYMBOL_LEN) {
                            printf("ERROR: %s:%d:%d: symbol too long!",file,line,col-MAX_SYMBOL_LEN);
                        }
                        symbolBuf[symbolLen++] = *ptr;
                    }
                    break;
            }
            ptr++;
            col++;
        }
        return tokens;
    }

    void endSymbol(std::vector<Token>* tokens, Context* context) {
        int len = *context->symbolLen;
        if (!len) return;
        char* symbol = new char[len+1];
        symbol[len] = 0;
        while (len--) symbol[len] = context->symbolBuf[len];
        *context->symbolLen = 0;
        if (auto key = keywordMap.find(symbol); key != keywordMap.end())
            tokens->push_back(Token{TokenType::KEYWORD,{.keyword={key->second}},context->file,*context->line,*context->column});
        else
            tokens->push_back(Token{TokenType::SYMBOL,{.value={symbol}},context->file,*context->line,*context->column});
    }

    int typeLexer(std::vector<Token>* tokens, Context* context) {
        char* ptr = *context->ptr;
        ptr++;
        (*context->column)++;
        int typeLen = 0;
        while (true) {
            switch (*ptr) {
                case ' ':
                    if (!typeLen) {
                        break;
                    }
                    [[fallthrough]];
                case '\n':
                    (*context->line)++;
                    [[fallthrough]];
                case ')':
                case '=':
                case ',': {
                        if (!typeLen) {
                            printf("ERROR: %s:%d:%d: expected type!",context->file,*context->line,*context->column);
                            return -1;
                        }
                        char* str = new char[typeLen+1];
                        str[typeLen] = 0;
                        while (typeLen--) str[typeLen] = context->symbolBuf[typeLen];
                        tokens->push_back(Token{TokenType::TYPE,{.value={str}},context->file,*context->line,*context->column});
                        *context->ptr = ptr;
                        return 0;
                    }
                default:
                    if (typeLen == MAX_SYMBOL_LEN) {
                        printf("ERROR: %s:%d:%d: type too long!",context->file,*context->line,*context->column);
                    }
                    context->symbolBuf[typeLen++] = *ptr;
                    break;
            }
            ptr++;
            (*context->column)++;
        }
    }

    inline bool isNumber(char c) {
        return c >= 48 && c <= 57;
    }

    inline bool symbolChar(unsigned char c, int pos) {
        if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == 96) return true;
        if (isNumber(c) && pos) return true;
        return false;
    }

    inline bool operatorChar(char c) {
        switch (c) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '>':
            case '<':
            case '=':
            case '%':
            case '^':
            case '&':
            case '|':
            case '~':
            case '!':
            case '.':
                return true;
            default:
                return false;
        }
    }

    inline bool validOperation(char* c, int len) {
        char* str = new char[len+1];
        str[len] = 0;
        while (len--) {
            if (!operatorChar(c[len])) return false;
            str[len] = c[len];
        }
        if (auto key = operations.find(str); key != operations.end()) {
            return true;
        }
        return false;
    }

    inline bool numberLiteral(char* c, int len) {

        if (!isNumber(c[0])) return false;

        if (len > 2) {

            if (isNumber(c[1]) || c[1] == 'x' || c[1] == 'b' || c[1] == 'o') {

                for (int i = 2; i < len; i++)
                    if (!isNumber(c[i]) && c[i] != '_') return false;

                return c[len-1] != '_';

            } else 
                return false;
        } else
            return isNumber(c[1]);
    }

    bool validLiteral(char* c, int len) {
        if (!len) return false;
        if (numberLiteral(c,len)) return true;

        if (c[0] == '\'' || c[0] == '"') {
            return (c[len-1] == c[0] && c[len-2]);
        }

        char* type = new char[len];
        char* literal = new char[len];
        char* ptr = type;
        int i = 0;
        for (; i < len; i++) {
            if (c[i] == '_') ptr = literal;
            else ptr[i] = c[i];
        }

        if (ptr != literal) return false;

        if (auto key = numberTypes.find(type); key != numberTypes.end()) {
            if (auto key2 = literalTypesMap.find(type); key2 != literalTypesMap.end()) {
                return true;
            }
        }

        return false;
    }

}
