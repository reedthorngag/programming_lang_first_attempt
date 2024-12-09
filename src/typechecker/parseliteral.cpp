#include <stdio.h>
#include <cstdint>
#include <cmath>

#include "../parser/parser.hpp"
#include "parseliteral.hpp"

using namespace Parser;

namespace TypeChecker {

    Type smallestNumType(uint64_t num, Type parent, bool negative) {
        if (parent == Type::error) {
            if (!num) return Type::u8;
            if (negative) {
                for (int i = Type::i8; i <= Type::i64; i++)
                    if (num >= TypeConstraints[i].min) return TypeConstraints[i].type;
            } else {
                for (int i = Type::i8; i <= Type::u64; i++)
                    if (num <= TypeConstraints[i].max) return TypeConstraints[i].type;
            }
        }

        if (parent >= Type::f16 && parent <= Type::f64) return parent; // TODO: handle floats properly
        if (parent == Type::string) return Type::u64; // size of the ptr to it
        if (parent > Type::f64) return Type::error;

        if (num > TypeConstraints[parent].max || (int64_t)num < TypeConstraints[parent].min) {
            //printf("%lld %d %d\n", (long long int)num, (int)(num > TypeConstraints[parent].max), (int)num < TypeConstraints[parent].min);
            return Type::error;
        }

        return parent;
    }

    inline bool isNumber(char c) {
        return c >= '0' && c <= '9';
    }

    inline bool isBinary(char c) {
        return c == '0' || c == '1';
    }

    inline bool isHex(char c) {
        return isNumber(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
    }

    inline bool isOct(char c) {
        return c >= '0' && c <= '7';
    }

    inline int parseNum(char c) {
        if (isNumber(c)) return c-'0';
        if (c >= 'a' && c <= 'f') return (c-'a') + 10;
        if (c >= 'F' && c <= 'F') return (c-'A') + 10;
        printf("ERROR: an error occurred in TypeChecker::parseNum that shouldn't have been possible!\n");
        exit(1);
    }

    uint64_t parseNum(Lexer::Token token) {
        char* c = token.literal.str;
        uint64_t out = 0;
        uint64_t oldOut = 0;
        int len = 0;
        int n = 0;
        while (c[++n]);

        uint64_t exponent = 0;

        while (n--) {
            if (c[n] == 'e' || c[n] == 'E' || c[n] == 'p' || c[n] == 'P') {
                exponent = out;
                out = 0;
                continue;
            }

            if (c[n] != '_') {
                int num = parseNum(c[n]);
                if (num >= token.literal.base) {
                    printf("ERROR: %s:%d:%d: literal '%s' invalid!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }

                // detect wrap around by the new value being smaller than the old value
                if (out < oldOut) {
                    printf("ERROR: %s:%d:%d: literal '%s' too large!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }
                out += num * pow(token.literal.base, len++);
                oldOut = out;
            }
        }

        while (exponent--) {
            if (out < oldOut) {
                printf("ERROR: %s:%d:%d: literal '%s' too large!\n",token.file.name, token.file.line, token.file.col,c);
                exit(1);
            }
            out *= token.literal.base;
            oldOut = out;
        }

        return out;
    }

    uint64_t parseFloat(Lexer::Token token) {
        char* c = token.literal.str;
        
        int len = 0;
        int n = 0;
        while (c[++n]);

        uint64_t out = 0;
        uint64_t exponent = 0;
        int decimalPlace = 0;

        char signBit = 0;
        uint64_t exponentPart = 0x3ff;
        uint64_t mantissa = 0;

        while (n--) {
            if (c[n] == 'e' || c[n] == 'E' || c[n] == 'p' || c[n] == 'P') {
                exponent = out;
                out = 0;
                continue;
            }

            if (c[n] != '_') {
                int num = parseNum(c[n]);
                if (num >= token.literal.base) {
                    printf("ERROR: %s:%d:%d: literal '%s' invalid!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }

                // detect wrap around by the new value being smaller than the old value
                if (out < oldOut) {
                    printf("ERROR: %s:%d:%d: literal '%s' too large!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }
                out += num * pow(token.literal.base, len++);
                oldOut = out;
            }
        }


    }

    bool parseNumber(Node* node, Type parent) {

        if (node->token.literal.floatingPoint) {
            node->literal._uint = parseFloat(node->token);
        } else
            node->literal._uint = parseNum(node->token);

        node->literal.type = smallestNumType(node->literal._uint, parent, node->literal.negative);

        if (node->literal.type == Type::error) {
            printf("ERROR: %s:%d:%d: literal '%s' doesn't fit in required type! ('%s')\n",node->token.file.name,node->token.file.line,node->token.file.col,node->token.literal.str,TypeMap[parent]);
            return false;
        }

        delete[] node->token.literal.str;
        
        return true;
    }

    bool parseString(Node* node) {

        char* in = node->literal.value;
        int len = 0;
        while (in[len]) len++;

        char* buf = new char[len];
        len = 0;
        int i = 1;
        bool ignoreNext = false;
        while (ignoreNext || in[i] != '"') {
            ignoreNext = false;
            if (in[i] == '\\') ignoreNext = true;
            else {
                buf[len] = in[i];
                len++;
            }
            i++;
        }
        delete[] in;

        node->literal.str.str = buf;
        node->literal.str.len = len;
        node->literal.type = Type::string;

        return true;
    }

    bool parseChar(Node* node) {
        node->literal.chr = node->literal.value[1];
        node->literal.type = Type::chr;
        return true;
    }

    bool parseLiteral(Node* node, Type parent) {

        switch (node->literal.value[0]) {
            case '"':
                return parseString(node);

            case '\'':
                return parseChar(node);

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                return parseNumber(node,parent);

            default:
                printf("ERROR: %s:%d:%d: '%s' invalid literal!\n",node->token.file.name,node->token.file.line,node->token.file.col,node->literal.value);
                return false;
        }
    }

};



