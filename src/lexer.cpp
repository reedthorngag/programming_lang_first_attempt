
#include "lexer.hpp"

namespace Lexer {

    std::unordered_map<std::string,Keyword> keywordMap = {
        {
            "func",Keyword::FUNCTION
        },
        {
            "var",Keyword::VARIABLE
        },
        {
            "const",Keyword::CONSTANT
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

    std::unordered_map<char,bool> operatorMap = {
        {
            '+',true
        },
        {
            '-',true
        },
        {
            '*',true
        },
        {
            '/',true
        },
        {
            '=',true
        },
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
                    tokens->push_back(Token{TokenType::BRACKET,{.c={*ptr}},file,line,col});
                    break;
                case '{':
                case '}':
                    endSymbol(tokens,&context);
                    tokens->push_back(Token{TokenType::BRACE,{.c={*ptr}},file,line,col});
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

}
