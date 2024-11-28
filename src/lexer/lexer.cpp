#include <cstring>

#include "lexer.hpp"

namespace Lexer {

    bool log = true;

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
            "global", Keyword::GLOBAL
        },
        {
            "if",Keyword::IF
        },
        {
            "else",Keyword::ELSE
        },
        {
            "while",Keyword::WHILE
        },
        {
            "return", Keyword::RETURN
        },
        {
            "break", Keyword::BREAK
        },
        {
            "continue", Keyword::CONTINUE
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
        {"<<<",true},
        {">>",true},
        {">>>",true},
    };

    const char* maxStr[] {
        "127", "32_767", "2_147_483_647", "9_223_372_036_854_775_807",
        "255", "65_535","4_294_967_295", "18_446_744_073_709_551_615",
    };

    const char* minStr[] {
        "-127", "-32_768", "-2_147_483_648", "-9_223_372_036_854_775_808",
        "0", "0", "0", "0"
    };

    struct Number {
        bool hasMinMax;
        const char* minStr;
        const char* maxStr;
    };

    std::unordered_map<std::string, Number> numberTypes = {
        {"i8",Number{true,minStr[NumberType::i8],maxStr[NumberType::i8]}},
        {"i16",Number{true,minStr[NumberType::i16],maxStr[NumberType::i16]}},
        {"i32",Number{true,minStr[NumberType::i32],maxStr[NumberType::i32]}},
        {"i64",Number{true,minStr[NumberType::i64],maxStr[NumberType::i64]}},

        {"u8",Number{true,minStr[NumberType::u8],maxStr[NumberType::u8]}},
        {"u16",Number{true,minStr[NumberType::u16],maxStr[NumberType::u16]}},
        {"u32",Number{true,minStr[NumberType::u32],maxStr[NumberType::u32]}},
        {"u64",Number{true,minStr[NumberType::u64],maxStr[NumberType::u64]}},

        {"f16",Number{false}},
        {"f32",Number{false}},
        {"f64",Number{false}},
    };

    std::unordered_map<std::string, int> literalTypesMap = {
        {"MIN",0},
        {"MAX",1}
    };

    std::unordered_map<std::string, std::string> builtinTypes = {
        {"true","1"},
        {"false","0"}
    };

    std::vector<Token>* lexerParse(char* file, char* input) {

        std::vector<Token>* tokens = new std::vector<Token>;
        
        char* ptr = input;
        int line = 1;
        int col = 1;
        int symbolLen = 0;
        char symbolBuf[MAX_SYMBOL_LEN];

        int commentLevel = 0;
        bool notMultiLine = true;
        bool firstEndChar = false;
        bool firstStartChar = false;

        bool isInString = false;
        bool escaped = false;
        int stringLine;
        int stringCol;

        int isOperator = 0;
        int isLiteral = 0;
        int isSymbol = 0;

        bool specialType = false; // currently only for '

        struct Context context = {
            file,
            input,
            &ptr,
            &line,
            &col,
            &symbolLen,
            symbolBuf,
            &specialType,
            &isOperator,
            &isLiteral,
            &isSymbol
        };

        while (*ptr) {

            if (commentLevel) isOperator = false; // because the first '/' is detected as an operator, so reset it

            while (commentLevel && *ptr) {
                col++;
                switch (*ptr) {
                    case '*':
                        commentLevel += firstStartChar;
                        firstEndChar = true;
                        break;
                    case '/':
                        commentLevel -= firstEndChar;
                        firstStartChar = !notMultiLine;
                        break;
                    case '\n':
                        commentLevel -= notMultiLine;
                        line++;
                        col = 1;
                        break;
                    default:
                        firstEndChar = false;
                        firstStartChar = false;
                        break;
                }
                ptr++;
            }

            if (commentLevel) {
                printf("ERROR: %s:%d:0: unexpected EOF, expecting '*/'\n",file,line);
                return nullptr;
            }

            if (isInString) {
                if (escaped) {
                    escaped = false;
                    symbolLen++;
                } else if (*ptr == '\\') {
                    escaped = true;
                    symbolLen++;

                } else if (*ptr == '"') {
                    tokens->push_back(Token{TokenType::LITERAL,{.value={newString(ptr-symbolLen,symbolLen+1)}},file,stringLine,stringCol,false});
                    symbolLen = 0;
                    isInString = false;
                } else if (*ptr == '\n') {
                    symbolLen++;
                    line++;
                    col = 0;
                } else
                    symbolLen++;

            } else {
                if (*ptr != '/' && *ptr != '*') {
                    firstStartChar = false;
                    commentLevel = 0;
                }
                switch (*ptr) {
                    case '/':
                        if (firstStartChar) {
                            notMultiLine = true;
                            commentLevel++;
                            symbolLen = 0;
                            ptr++;
                            continue;
                        }
                        firstStartChar = true;
                        goto addChar;
                    case '*':
                        if (firstStartChar) {
                            notMultiLine = false;
                            commentLevel++;
                            symbolLen = 0;
                            ptr++;
                            continue;
                        }
                        goto addChar;

                    case '\r':
                        ptr++;
                        continue;
                    
                    case '\n':
                        line++;
                        col = 0;
                        break;

                    case '\'':
                        specialType = true;
                        goto addChar;

                    case ',':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        tokens->push_back(Token{TokenType::COMMA,{},file,line,col});
                        break;

                    case ':':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        if (!typeLexer(tokens,&context)) return nullptr;
                        continue;

                    case ';':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        tokens->push_back(Token{TokenType::ENDLINE,{},file,line,col});
                        break;
                    
                    case '(':
                    case ')':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        tokens->push_back(Token{*ptr=='('?TokenType::GROUPING_START:TokenType::GROUPING_END,{},file,line,col});
                        break;
                    
                    case '{':
                    case '}':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        tokens->push_back(Token{*ptr=='{'?TokenType::SCOPE_START:TokenType::SCOPE_END,{},file,line,col});
                        break;
                    
                    case '\t':
                    case ' ':
                        if (!endSymbol(tokens,&context)) return nullptr;
                        break;

                    case '\\':
                        printf("ERROR: %s:%d:%d: unexpected backslash!\n",file,line,col);
                        return nullptr;

                    case '"':
                        if (symbolLen) {
                            printf("ERROR: %s:%d:%d: unexpected string literal!\n",file,line,col);
                            return nullptr;
                        }
                        isInString = true;
                        stringLine = line;
                        stringCol = col;
                        [[fallthrough]];
                    default:
                        addChar:
                        //printf("%c %d%d %d%d %d%d  %d\n",*ptr,operatorChar(*ptr),isOperator,symbolChar(*ptr,isSymbol),isSymbol,validNumberLiteral(*ptr),isLiteral,symbolLen);
                        if (!specialType) {
                            if (operatorChar(*ptr)) {
                                isOperator++;
                            } else if (isOperator == symbolLen) {
                                isOperator = 0;
                                if (!endSymbol(tokens,&context)) return nullptr;
                            }
                            
                            if (symbolChar(*ptr,isSymbol)) {
                                isSymbol++;
                            } else if (isSymbol == symbolLen) {
                                isSymbol = 0;
                                if (!endSymbol(tokens,&context)) return nullptr;
                            }

                            if (validNumberLiteral(*ptr)) {
                                isLiteral++;
                            } else if (isLiteral == symbolLen) {
                                isLiteral = 0;
                                if (validLiteral(symbolBuf,symbolLen))
                                    if (!endSymbol(tokens,&context)) return nullptr;
                            }
                        }
                        if (symbolLen == MAX_SYMBOL_LEN) {
                            printf("ERROR: %s:%d:%d: symbol too long!\n",file,line,col-MAX_SYMBOL_LEN);
                            return nullptr;
                        }
                        symbolBuf[symbolLen++] = *ptr;
                        escaped = false;
                }
            }
            ptr++;
            col++;
        }

        if (symbolLen) {
            printf("ERROR: %s:%d:0: unexpected EOF!\n",file,line+1);
            return nullptr;
        }

        tokens->push_back(Token{TokenType::FILE_END,{},file,line+1,0});
        return tokens;
    }

    bool endSymbol(std::vector<Token>* tokens, Context* context) {
        *context->specialType = false;

        int len = *context->symbolLen;
        if (!len) return true;
        char* str = new char[len+1];
        str[len] = 0;
        while (len--) str[len] = context->symbolBuf[len];
        len = *context->symbolLen;

        if (validOperation(str,len)) {
            *context->isOperator = 0;
            tokens->push_back(Token{TokenType::OPERATOR,{.value={str}},context->file,*context->line,*context->column-len,false});
        }
        else if (validLiteral(str,len)) {
            printf("here! %s\n",str);
            *context->isLiteral = 0;
            tokens->push_back(Token{TokenType::LITERAL,{.value={str}},context->file,*context->line,*context->column-len,false});
        }
        else if (auto key = builtinTypes.find(str); key != builtinTypes.end()) {
            *context->isLiteral = 0;
            char* str = new char[key->second.size()+1];
            strcpy(str,key->second.c_str());
            tokens->push_back(Token{TokenType::LITERAL,{.value={str}},context->file,*context->line,*context->column-len,false});
        }
        else if (auto key = keywordMap.find(str); key != keywordMap.end()) {
            *context->isSymbol = 0;
            tokens->push_back(Token{TokenType::KEYWORD,{.keyword={key->second}},context->file,*context->line,*context->column-len,false});
        } else if (validSymbol(str,len)) {
            *context->isSymbol = 0;
            tokens->push_back(Token{TokenType::SYMBOL,{.value={str}},context->file,*context->line,*context->column-len,false});
        } else {
            printf("ERROR: %s:%d:%d: invalid symbol: '%s'\n",context->file,*context->line,*context->column-len,str);
            return false;
        }
        *context->symbolLen = 0;
        return true;
    }

    bool typeLexer(std::vector<Token>* tokens, Context* context) {
        char* ptr = *context->ptr;
        ptr++;
        (*context->column)++;
        int typeLen = 0;
        while (true) {
            switch (*ptr) {
                case '\n':
                    (*context->line)++;
                    *context->column = 1;
                    if (typeLen) goto endType;
                    [[fallthrough]];
                case ' ':
                    if (!typeLen) {
                        break;
                    }
                    [[fallthrough]];
                case '(':
                case ')':
                case '{':
                case '}':
                case ',': 
                case ';': {
                    endType:
                    if (!typeLen) {
                        printf("ERROR: %s:%d:%d: expected type!\n",context->file,*context->line,*context->column);
                        return false;
                    }
                    char* str = new char[typeLen+1];
                    str[typeLen] = 0;
                    int len = typeLen;
                    while (typeLen--) str[typeLen] = context->symbolBuf[typeLen];
                    tokens->push_back(Token{TokenType::TYPE,{.value={str}},context->file,*context->line,*context->column-len,false});
                    *context->ptr = ptr;
                    return true;
                }
                default:
                    if (operatorChar(*ptr)) goto endType;
                    if (typeLen == MAX_SYMBOL_LEN) {
                        printf("ERROR: %s:%d:%d: type too long!\n",context->file,*context->line,*context->column);
                    }
                    context->symbolBuf[typeLen++] = *ptr;
                    break;
            }
            ptr++;
            (*context->column)++;
        }
    }

    inline char* newString(char* c, int len) {
        char* str = new char[len+1];
        str[len] = 0;
        while (len--) str[len] = c[len];
        return str;
    }

    inline bool isNumber(char c) {
        return c >= '0' && c <= '9';
    }

    inline bool isHexNumber(char c) {
        return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    inline bool symbolChar(char c, int pos) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') return true;
        if (isNumber(c) && pos) return true;
        return false;
    }

    inline bool validSymbol(char* c, int len) {
        while (len--) if (!symbolChar(c[len],len)) return false;
        return true;
    }

    bool operatorChar(char c) {
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

    inline bool validNumberLiteral(char c) {
        if (isNumber(c)) return true;
        return c == 'x' || c == 'b' || c == 'o' || c == '_' || c ==  '.';
    }

    inline bool numberLiteral(char* c, int len) {

        bool decimalPoint = false;

        if (!isNumber(c[0])) return false;

        if (c[0] == '.')
            decimalPoint = true;

        if (len > 1 && c[1] == '.') {
            if (decimalPoint) return false; 
            else decimalPoint = true;
        }

        if (len > 2) {

            if (isNumber(c[1]) || c[1] == 'x' || c[1] == 'b' || c[1] == 'o' || c[1] == '.') {

                for (int i = 2; i < len; i++) {
                    if (c[i] == '.') {
                        if (decimalPoint) return false;
                        else decimalPoint = true;
                    }
                    else if (!isHexNumber(c[i]) && c[i] != '_') return false;
                }

                return c[len-1] != '_';

            } else 
                return false;
        } else
            return len == 1 || isNumber(c[1]);
    }

    inline bool validCharLiteral(char* c, int len) {
        if (c[0] == '\'') { // add support for stuff like \0x0 and check valid
            return c[len-1] == '\'';
        }
        return false;
    }

    bool validBuiltInLiteral(char* c, int len) {
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

    bool validLiteral(char* c, int len) {
        if (!len) return false;

        if (numberLiteral(c,len)) return true;

        if (validCharLiteral(c,len)) return true;

        if (validBuiltInLiteral(c,len)) return true;

        return false;
    }

}
