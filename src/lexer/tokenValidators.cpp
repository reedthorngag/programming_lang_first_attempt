#include <sstream>
#include <stdlib.h>
#include <stdio.h>

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

                break;
            }
        } while (++len <= MAX_SYMBOL_LEN);

        return false;
    }

    bool parseComment() {

        char* ptr = Lexer::ptr;
        int len = 0;

        bool multiline = false;
        int commentLevel;



    }

    bool parseStr() {

        char* ptr = Lexer::ptr;

        File f = Lexer::file;

        bool escaped = false;

        std::stringstream str;

        while (*ptr != '"' || escaped) {
            if (escaped) {
                escaped = false;

                switch (*ptr) {
                    case 'x':
                        if (!isHexNumber(*ptr) || !isHexNumber(ptr[1])) {
                            printf("ERROR: %s:%d:%d: invalid escape sequence!\n",file.name,file.line,file.col);
                            exit(1);
                        }
                        str << *ptr << *++ptr;
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
        tokens->push_back(Token{TokenType::LITERAL,{.value={new string(str.str()).c_str()}},file,false});

        return true;
    }

    bool parseChr() {

        char* ptr = Lexer::ptr;
        int len = 0;

        ptr++;
        
        if (*ptr != '\\') {
            
        }
    }

    bool parseLiteral() {

        char* ptr = Lexer::ptr;
        int len = 0;

        if (ptr[0] == '"') return parseStr();

        if (ptr[0] == '\'') return parseChr();

        bool hex = false;

        if (ptr[0] == '0') {

        }
    }

    bool parseOperator() {

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
}
