#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "lexer.hpp"
#include "util.hpp"


namespace Lexer {

    const char* minMaxLiteralStr(char* str) {

        int len = strlen(str);

        if (len < 6) return nullptr;

        int i = 2;
        while (str[i] != '_' && ++i < len);

        if (i == len) return nullptr;

        char* type = newString(str, i);

        if (auto key = numberTypes.find(type); key != numberTypes.end()) {

            char* minMaxStr = newString(str+i+1, len-(i+1));

            if (auto key2 = literalTypesMap.find(minMaxStr); key2 != literalTypesMap.end()) {
                return (key2->second ? key->second.maxStr : key->second.minStr);
            }

        }

        return nullptr;
    }

    bool parseSymbol() {
        if (log) {
            printf("\rEntered parseSymbol      ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        int len = 0;

        do {
            if (!isSymbolChar(ptr[len], len)) {
                if (!len) return false;

                if (!isBreakChar(ptr[len])) return false;

                char* str = newString(ptr, len);

                char* minMaxStr = (char*)minMaxLiteralStr(str);

                if (auto key = builtinLiteralTypes.find(str); key != builtinLiteralTypes.end())
                    tokens->push_back(Token{TokenType::LITERAL,{.value={str}},file,false});

                else if (minMaxStr)
                    tokens->push_back(Token{TokenType::LITERAL,{.value={(char*)str}},file,false});
                
                else if (auto key = keywordMap.find(str); key != keywordMap.end())
                    tokens->push_back(Token{TokenType::KEYWORD,{.keyword={key->second}},file,false});
                
                else
                    tokens->push_back(Token{TokenType::SYMBOL,{.value={str}},file,false});
                
                len--;
                Lexer::ptr += len;
                file.col += len;

                printf("\n");
                return true;
            }
        } while (++len <= MAX_SYMBOL_LEN);

        return false;
    }

#define SHORT(x, y) (short)((y << 8) | (char)x)

    bool parseComment() {
        if (log) {
            printf("\rEntered parseComment      ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        File f = Lexer::file;

        bool multiline = false;
        int commentLevel = 0;

        switch (*++ptr) {
            case '/':
                break;
            case '*':
                multiline = true;
                break;
            default:
                return false;
        }

        commentLevel++;
        f.col += 2;

        while (commentLevel) {
            ptr++;
            f.col++;

            if (multiline && !*ptr) {
                printf("\nERROR: %s:%d:%d: unexpected EOF, expecting '*/'!\n",f.name,f.line,f.col);
                exit(1);
            }

            if (!multiline && (*ptr == '\n' || !*ptr)) break;

            if (*ptr == '\n') {
                f.line++;
                f.col = 0;
            }

            switch (*(short*)ptr) {
                case SHORT('/','/'):
                    if (!multiline) commentLevel++;
                    ptr++;
                    break;
                case SHORT('/','*'):
                    commentLevel++;
                    ptr++;
                    break;
                case SHORT('*','/'):
                    commentLevel--;
                    ptr++;
                    break;
                default:
                    break;
            }
        }

        if (*ptr == '\n') {
            f.line++;
            f.col = 0;
        }

        file = f;
        Lexer::ptr = ptr;

        printf("\n");
        return true;
    }

    bool parseStr() {
        if (log) {
            printf("\rEntered parseStr       ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;

        File f = Lexer::file;

        bool escaped = false;

        std::stringstream str;

        str << '"';

        ptr++;
        while (*ptr != '"' || escaped) {
            if (escaped) {
                escaped = false;

                switch (*ptr) {
                    case 'x':
                        if (!isHexNumber(*++ptr) || !isHexNumber(ptr[1])) {
                            printf("\nERROR: %s:%d:%d: invalid escape sequence!\n",file.name,file.line,file.col);
                            exit(1);
                        }
                        str << *ptr << ptr[1];
                        ptr++;
                        f.col++;
                        break;
                    case 'n':
                        str << "\\x" << toHexByte('\n');
                        break;
                    case 'b':
                        str << "\\x" << toHexByte('\b');
                        break;
                    case 't':
                        str << "\\x" << toHexByte('\t');
                        break;
                    case 'r':
                        str << "\\x" << toHexByte('\r');
                        break;
                    default:
                        if (*ptr == '\r' && ptr[1] == '\n') {
                            f.line++;
                            f.col = 0;
                            ptr++;
                        } else if (*ptr == '\n') {
                            f.line++;
                            f.col = 0;
                        } else
                            str << "\\x" << toHexByte(*ptr);
                        break;
                }

                f.col++;
                ptr++;
                continue;
            }

            if (*ptr == 0) {
                printf("\nERROR: %s:%d:%d: unexpected EOF, expecting '\"'!\n",f.name,f.line,f.col);
                exit(1);
            }

            if (*ptr == '\\') {
                escaped = true;
                ptr++;
                f.col++;
                continue;
            }

            if (*ptr == '\n') {
                f.line++;
                f.col = 0;
            }

            str << *ptr;
            ptr++;
            f.col++;
        }

        f.col++;
        str << '"';

        char* s = newString((char*)str.str().c_str(),str.str().length());
        tokens->push_back(Token{TokenType::LITERAL,{.value={s}},file,false});

        Lexer::ptr = ptr;
        Lexer::file = f;

        printf("\n");
        return true;
    }

    bool parseChr() {
        if (log) {
            printf("\rEntered parseChr       ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        
        if (*++ptr != '\\') {
            if (*++ptr != '\'') {
                printf("\nERROR: %s:%d:%d: expecting ' found '%c'!\n",file.name,file.line,file.col + 2, *ptr);
                exit(1);
            }

            char* str = newString(Lexer::ptr, 3);
            tokens->push_back(Token{TokenType::LITERAL,{.value = {str}},file,false});
            Lexer::ptr += 2;
            file.col += 2;

            printf("\n");
            return true;
        }

        File f = Lexer::file;
        std::stringstream ss;
        ss << '\'';

        f.col++;

        switch (*++ptr) {
            case 'x':
                if (!isHexNumber(*++ptr) || !isHexNumber(ptr[1])) {
                    printf("\nERROR: %s:%d:%d: invalid escape sequence!\n",file.name,file.line,file.col);
                    exit(1);
                }
                ss << "\\x" << *ptr << ptr[1];
                ptr++;
                f.col++;
                break;
            case 'n':
                ss << "\\x" << toHexByte('\n');
                break;
            case 'b':
                ss << "\\x" << toHexByte('\b');
                break;
            case 't':
                ss << "\\x" << toHexByte('\t');
                break;
            case 'r':
                ss << "\\x" << toHexByte('\r');
                break;
            default:
                ss << "\\x" << toHexByte(*ptr);
                if (*ptr == '\r' && ptr[1] == '\n') {
                    f.line++;
                    f.col = 0;
                    ptr++;
                } else if (*ptr == '\n') {
                    f.line++;
                    f.col = -1;
                }
                break;
        }

        ss << '\'';

        f.col++;
        ptr++;

        if (*ptr != '\'') {
            printf("\nERROR: %s:%d:%d: expecting ' found %c (0x%x)!\n",f.name,f.line,f.col,*ptr,*ptr);
            exit(1);
        }

        char* s = newString((char*)ss.str().c_str(),ss.str().length());
        tokens->push_back(Token{TokenType::LITERAL,{.value = {s}},file,false});
        Lexer::ptr = ptr;

        file = f;

        printf("\n");
        return true;
    }

    bool isBinary(char c) {
        return c == '0' || c == '1' || c == '_';
    }

    bool isOctal(char c) {
        return (c >= '1' && c <= '7') || c == '0' || c == '_';
    }

    bool isDecimal(char c) {
        return isNumber(c) || c == '_';
    }

    bool isHexadecimal(char c) {
        return isHexNumber(c) || c == '_';
    }

    bool parseLiteral() {
        if (log) {
            printf("\rEntered parseLiteral     ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        int len = 0;

        // only valid start chars are 0 - 9 or . (- is handled in the parser)
        if ((*ptr < '0' || *ptr > '9') && *ptr != '.') return false;

        bool(*evalFunc)(char);

        if (*ptr == '0') {
            len += 2;
            switch (*++ptr) {
                case 'b':
                    evalFunc = isBinary;
                    break;
                case 'o':
                    evalFunc = isOctal;
                    break;
                case 'x':
                    evalFunc = isHexadecimal;
                    break;
                default:
                    evalFunc = isDecimal;
                    ptr--;
                    len--;
                    break;
            }
            ptr++;
        } else evalFunc = isDecimal;

        bool decimal = false;
        bool exponent = false;

        while (true) {
            char c = *ptr;
            ptr++;
            len++;

            if (evalFunc(c)) continue;

            // decimal part
            if (!decimal && c == '.') {
                decimal = true;
                continue;
            }

            // exponent
            if (!exponent && (c == 'e' || c == 'p')) {
                exponent = true;
                continue;
            }

            if (isBreakChar(c) || isOperatorChar(c)) break;

            return false;
        }

        char* str = newString(Lexer::ptr, --len);

        tokens->push_back(Token{TokenType::LITERAL, {.value={str}},file, false});

        if (log) printf("\nParsed Literal number: %d:%d: %s\n",file.line, file.col, str);

        file.col += len;
        Lexer::ptr += --len;

        return true;
    }

    bool parseOperator() {
        if (log) {
            printf("\rEntered parseOperator     ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        int len = 0;

        char* str = new char[4]{};
        str[0] = *ptr;

        if (!isOperatorChar(*ptr)) return false;

        str[1] = isOperatorChar(ptr[1]) ? ptr[1] : 0;
        str[2] = isOperatorChar(ptr[2]) ? ptr[2] : 0;

        len = strlen(str);

        if (auto key = operations.find(str); key != operations.end()) {

            tokens->push_back(Token{TokenType::OPERATOR, {.value = {str}}, file, false});

            len--;
            file.col += len;
            Lexer::ptr += len;

            printf("\n");
            return true;
        }

        return false;
    }

    bool parseType() {
        if (log) {
            printf("\rEntered parseType      ");
            fflush(stdout);
        }

        char* ptr = Lexer::ptr;
        int len = 0;

        // slightly bad, but we just use the parseSymbol function
        if (!parseSymbol()) {
            printf("darn!\n");
            return false;
        }

        Token type = tokens->back();
        
        if (type.type != TokenType::SYMBOL) {
            printf("\nERROR: %s:%d:%d: invalid type '%s'!\n",type.file.name,type.file.line,type.file.col,type.value);
            exit(1);
        }

        tokens->back().type = TokenType::TYPE;

        printf("\n");
        return true;
    }
}
