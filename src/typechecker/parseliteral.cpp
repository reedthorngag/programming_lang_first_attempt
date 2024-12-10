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
                    if ((int64_t)num >= TypeConstraints[i].min) return TypeConstraints[i].type;
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
                len = 0;
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

        printf("Parsing float: '%s'\n",c);
        
        int len = 0;
        int n = 0;
        while (c[++n]);

        uint64_t out = 0;
        int specificExponent = 0;
        uint64_t decimalPart = 0;
        uint64_t integerPart = 0;

        uint8_t decimalPartLen = 0;

        while (n--) {
            if (c[n] == '-') {
                continue;
            }

            if (c[n] == 'e' || c[n] == 'E' || c[n] == 'p' || c[n] == 'P') {
                printf("Exponent part found! value: %d (0x%lx)\n",(int)out, out);
                specificExponent = out;
                out = 0;
                len = 0;
                continue;
            }

            if (c[n] == '.') {
                printf("Fractional part found! value: %d (0x%lx)\n",(int)out, out);
                decimalPart = out;
                decimalPartLen = len;
                out = 0;
                len = 0;
                continue;
            }

            if (c[n] != '_') {
                int num = parseNum(c[n]);
                if (num >= token.literal.base) {
                    printf("ERROR: %s:%d:%d: literal '%s' invalid!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }

                // detect wrap around by the new value being smaller than the old value
                if (out > ((uint64_t)1 << 53)) {
                    printf("ERROR: %s:%d:%d: literal '%s' too large!\n",token.file.name, token.file.line, token.file.col,c);
                    exit(1);
                }
                out += num * pow(token.literal.base, len++);
            }
        }

        printf("Float integer part: %d (0x%lx)\n",(int)out, out);

        char signBit = token.literal.negative;
        int64_t exponentPart = 0x3ff;
        uint64_t mantissa = 0;

        int implicitExponent = 0;

        uint64_t decimal = 0;
        uint64_t decimalThreshold = 1;

        uint8_t integerPartNumBits = 0;

        n = 0;
        while (n < 64)
            if ((out >> n++) & 1) integerPartNumBits = n;

        uint8_t decimalAllowance = 53-integerPartNumBits;

        printf("Integer part num bits: %d, decimal part allowance: %d\n",integerPartNumBits,decimalAllowance);

        while (decimalPartLen--) decimalThreshold *= 10;

        printf("Decimal threshold: %lu\n",decimalThreshold);

        uint8_t allowance = decimalAllowance + 1;

        while (decimalPart && --allowance) {
            decimalPart *= 2;
            decimal <<= 1;
            if (decimalPart >= decimalThreshold) {
                decimal |= 1;
                decimalPart -= decimalThreshold;
            }
        }

        printf("decimal before padding: 0x%lx allowance: %d\n",decimal,allowance);

        // pad with trailing zeros
        decimal <<= allowance;

        if (!out) {
            n = decimalAllowance;
            while (!((decimal >> --n) & 1));
            printf("n: %d %d\n",n,decimalAllowance);
            if (n) {
                implicitExponent = n-decimalAllowance;
                decimal <<= decimalAllowance-n;
            } else {
                implicitExponent = 0;
            }
        } else {
            implicitExponent = integerPartNumBits - 1;
        }

        printf("Decimal part: 0x%lx\n",decimal);

        mantissa = ((out << decimalAllowance) | decimal) & ~(1LL << 53);

        printf("Mantissa part: 0x%lx\n",mantissa);

        if (token.literal.negativeExponent) specificExponent = -specificExponent;

        exponentPart += specificExponent + implicitExponent;

        printf("Exponent part: %ld (0x%lx) (specific exponent: %d (0x%x) implicit exponent: %d (0x%x))\n",exponentPart,exponentPart,specificExponent,specificExponent,implicitExponent,implicitExponent);

        if (exponentPart >= (1<<12)) {
            printf("ERROR: %s:%d:%d: double exponent too large!\n",token.file.name,token.file.line,token.file.col);
            exit(1);
        }

        out = ((uint64_t)signBit << 63) | exponentPart << 52 | mantissa;


        printf("Sign: %d %d (0x%lx)\n",token.literal.negative,signBit,(uint64_t)signBit << 63);

        printf("Final double: 0x%lx\n",out);

        return out;
    }

    bool parseNumber(Node* node, Type parent) {

        if (node->token.literal.floatingPoint) {
            node->literal._uint = parseFloat(node->token);

            node->literal.type = Type::u64;
        } else {
            node->literal._uint = parseNum(node->token);

            node->literal.type = smallestNumType(node->literal._uint, parent, node->literal.negative);
        }

        if (node->literal.type == Type::error) {
            printf("ERROR: %s:%d:%d: literal '%s' doesn't fit in required type! ('%s')\n",node->token.file.name,node->token.file.line,node->token.file.col,node->token.literal.str,TypeMap[parent]);
            return false;
        }

        delete[] node->token.literal.str;
        
        return true;
    }

    char fromHexDigit(char c) {
        if (c <= '0' && c >= '9') return c-'0';
        if (c >= 'a' && c <= 'f') return (c-'a') + 10;
        if (c >= 'A' && c <= 'F') return (c-'A') + 10;
        printf("Error in fromHexDigit\n"); // shouldn't be possible
        exit(1);
    }

    bool parseLiteral(Node* node, Type parent) {

        switch (node->literal.value[0]) {
            case '"':
                node->literal.type = Type::string;
                node->literal.str.str = node->literal.value;
                node->literal.str.len = 0; // don't use it currently
                return true;

            case '\'':
                if (node->literal.value[1] == '\\') {
                    char* c = node->literal.value;
                    node->literal.chr = fromHexDigit(c[3]) + fromHexDigit(c[4]);
                } else
                    node->literal.chr = node->literal.value[1];
                node->literal.type = Type::chr;
                return true;

            default:
                return parseNumber(node,parent);
        }
    }

};



