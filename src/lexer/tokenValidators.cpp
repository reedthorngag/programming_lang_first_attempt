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
                            f.col = -1; // gets incremented to 0 below switch
                        }
                        break;
                }

                f.col++;
                ptr++;
                continue;
            }

            if (*ptr == '\\') {
                escaped = true;
                ptr++;
                f.col++;
                continue;
            }

            if (*ptr == '\n') f.line++;

            str << *ptr;
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
                printf("ERROR: %s:%d:%d: expecting ', found '%c'!\n",file.name,file.line,file.col + 2, ptr[2]);
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

        switch (*ptr) {
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
                    file.col = -1;
                }
                break;
        }

        file.col++;
        ptr++;

        if (*ptr != '\'') {
            printf("ERROR: %s:%d:%d: expecting ', found %c!\n",file.name,file.line,file.col,*ptr);
            exit(1);
        }

        char* s = newString((char*)ss.str().c_str(),ss.str().length());
        tokens->push_back(Token{TokenType::LITERAL,{.value = {s}},file,false});
        Lexer::ptr = ptr;

        return true;
    }

    bool parseLiteral() {
        if (log) printf("Entered parseLiteral\n");

        char* ptr = Lexer::ptr;
        int len = 0;

        if (ptr[0] == '"') return parseStr();

        if (ptr[0] == '\'') return parseChr();

        if (isHexNumber(ptr[0]) && ptr[0] != '.') return false;

        bool hex = false;

        if (ptr[0] == '0') {

        }

        return false;
    }

    bool parseOperator() {
        if (log) printf("Entered parseOperator\n");

        char* ptr = Lexer::ptr;
        int len = 0;

        char* str = new char[4]{};
        str[0] = *ptr;

        if (operatorChar(ptr[1])) str[1] = ptr[1];
        if (operatorChar(ptr[2])) str[2] = ptr[2];

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

        return false;
    }
}
