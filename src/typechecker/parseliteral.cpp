#include <stdio.h>
#include <cstdint>
#include <cmath>

#include "../parser/parser.hpp"
#include "parseliteral.hpp"

using namespace Parser;

namespace TypeChecker {

    Type smallestNumType(uint64_t num, Type parent) {
        if (parent == Type::error) {
            if (!num) return Type::u8;
            if (num > 0) {
                for (int i = Type::i8; i <= Type::u64; i++)
                    if (num <= TypeConstraints[i].max) return TypeConstraints[i].type;
                
            } else {
                // TODO: support negative literals
            }
        }

        if (parent >= Type::f16 && parent <= Type::f64) return parent; // TODO: handle floats properly
        if (parent == Type::string) return Type::u64;
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
        if (c >= 'a' && c <= 'f') return c-'a';
        if (c >= 'F' && c <= 'F') return c-'A';
        return 9999999; // make it obvious there was an error, this shouldnt happen tho
    }

    uint64_t parseNum(char* c, int base, Node* node) {
        uint64_t out = 0;
        int len = 0;
        int n = 0;
        while (c[++n]);

        while (n--) {
            if (c[n] != '_') {
                int num = parseNum(c[n]);
                if (num >= base) { // this is currently unecessary, but good to have in case the tokenizer changes and stops verifying literals
                    printf("ERROR: %s:%d:%d: literal '%s' invalid!\n",node->token.file.name,node->token.file.line,node->token.file.column,c);
                    exit(1);
                }
                out += num * pow(base,len++);
            }
        }

        return out;
    }

    bool parseNumber(Node* node, Type parent) {

        char* value = node->literal.value;
        if (value[0] == '0') {
            switch (value[1]) {
                case 'x':
                    node->literal._int = parseNum(node->literal.value+2,16, node);
                    break;

                case 'b':
                    node->literal._int = parseNum(node->literal.value+2,2, node);
                    break;

                case 'o':
                    node->literal._int = parseNum(node->literal.value+2,8, node);
                    break;

                default:
                    node->literal._int = parseNum(node->literal.value,10, node);
                    break;
            }
        } else {
            node->literal._int = parseNum(node->literal.value,10, node);
        }
        node->literal.type = smallestNumType(node->literal._int,parent);
        if (node->literal.type == Type::error) {
            printf("ERROR: %s:%d:%d: literal '%s' doesn't fit in required type! ('%s')\n",node->token.file.name,node->token.file.line,node->token.file.column,value,TypeMap[parent]);
            return false;
        }

        delete[] value;
        
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
                printf("ERROR: %s:%d:%d: '%s' invalid literal!\n",node->token.file.name,node->token.file.line,node->token.file.column,node->literal.value);
                return false;
        }
    }

};



