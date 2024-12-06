#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include "lexer.hpp"
#include "util.hpp"


namespace Lexer {

    const char* minMaxLiteralStr(char* str) {
        if (log) printf("Entered minMaxLiteral\n");

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
        if (log) printf("Entered parseSymbol\n");

        char* ptr = Lexer::ptr;
        int len;

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

                return true;
            }
        } while (++len <= MAX_SYMBOL_LEN);

        return false;
    }

    bool parseComment() {
        if (log) printf("Entered parseComment\n");

        char* ptr = Lexer::ptr;
        int len = 0;

        bool multiline = false;
        int commentLevel;


        return false;
    }

    bool parseStr() {
        if (log) printf("Entered parseStr\n");

        char* ptr = Lexer::ptr;

        File f = Lexer::file;

        bool escaped = false;

        std::stringstream str;

        ptr++;
        while (*ptr != '"' || escaped) {
            if (escaped) {
                escaped = false;

                switch (*ptr) {
                    case 'x':
                        if (!isHexNumber(*++ptr) || !isHexNumber(ptr[1])) {
                            printf("ERROR: %s:%d:%d: invalid escape sequence!\n",file.name,file.line,file.col);
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
                        str << "\\x" << toHexByte(*ptr);
                        if (*ptr == '\n') {
                            f.line++;
                            f.col = 0;
                        }
                        break;
                }

                f.col++;
                ptr++;
                continue;
            }

            if (*ptr == 0) return false;

            if (*ptr == '\\') {
                escaped = true;
                ptr++;
                f.col++;
                continue;
            }

            if (*ptr == '\n') f.line++;

            str << *ptr;
            ptr++;
            f.col++;
        }

        Lexer::ptr = ptr;
        Lexer::file = f;

        char* s = newString((char*)str.str().c_str(),str.str().length());
        tokens->push_back(Token{TokenType::LITERAL,{.value={s}},file,false});

        return true;
    }

    bool parseChr() {
        if (log) printf("Entered parseChr\n");

        char* ptr = Lexer::ptr;
        int len = 0;
        
        if (ptr[1] != '\\') {
            if (ptr[2] != '\'') {
                printf("ERROR: %s:%d:%d: expecting ' found '%c'!\n",file.name,file.line,file.col + 2, ptr[2]);
                exit(1);
            }

            char* str = newString(Lexer::ptr, 3);
            tokens->push_back(Token{TokenType::LITERAL,{.value = {str}},file,false});
            Lexer::ptr += 3;
            file.col += 3;
            return true;
        }

        std::stringstream ss;
        ss << '\'';

        ptr++;
        file.col++;

        switch (*++ptr) {
            case 'x':
                if (!isHexNumber(*++ptr) || !isHexNumber(ptr[1])) {
                    printf("ERROR: %s:%d:%d: invalid escape sequence!\n",file.name,file.line,file.col);
                    exit(1);
                }
                ss << *ptr << ptr[1];
                ptr++;
                file.col++;
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
                if (*ptr == '\n') {
                    file.line++;
                    file.col = 0;
                }
                break;
        }

        file.col++;
        ptr++;

        if (*ptr != '\'') {
            printf("ERROR: %s:%d:%d: expecting ' found %c!\n",file.name,file.line,file.col,*ptr);
            exit(1);
        }

        char* s = newString((char*)ss.str().c_str(),ss.str().length());
        tokens->push_back(Token{TokenType::LITERAL,{.value = {s}},file,false});
        Lexer::ptr = ptr;

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
        if (log) printf("Entered parseLiteral\n");

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

        char* str = newString(Lexer::ptr, len);

        tokens->push_back(Token{TokenType::LITERAL, {.value={str}},file, false});

        file.col += len;

        Lexer::ptr = ptr;

        if (log) printf("Parsed Literal number: %d:%d: %s\n",file.line, file.col, str);

        return true;
    }

    bool parseOperator() {
        if (log) printf("Entered parseOperator\n");

        char* ptr = Lexer::ptr;
        int len = 0;

        char* str = new char[4]{};
        str[0] = *ptr;

        if (!isOperatorChar(*ptr)) return false;

        str[1] = isOperatorChar(ptr[1]) ? ptr[1] : 0;
        str[2] = isOperatorChar(ptr[2]) ? ptr[2] : 0;

        len = strlen(str);

        if (auto key = operations.find(str); key != operations.end()) {
            file.col += len;
            Lexer::ptr += len;

            tokens->push_back(Token{TokenType::OPERATOR, {.value = {str}}, file, false});
            return true;
        }

        return false;
    }

    bool parseType() {
        if (log) printf("Entered parseType\n");

        char* ptr = Lexer::ptr;
        int len = 0;

        // slightly hacky, but we just use the parseSymbol 
        if (!parseSymbol()) return false;

        Token type = tokens->back();
        
        if (type.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: invalid type '%s'!\n",type.file.name,type.file.line,type.file.col,type.value);
            exit(1);
        }

        tokens->back().type = TokenType::TYPE;

        return false;
    }
}
