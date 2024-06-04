#include <stdio.h>
#include <cstdint>
#include <cmath>

#include "../parser/parser.hpp"
#include "parseliteral.hpp"

using namespace Parser;

namespace TypeChecker {

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

    int64_t parseNum(char* c, int base) {

        int64_t out = 0;
        int len = 0;
        int n = 0;
        while (c[n]) if (c[n++] != '_') len++;
        
        n = 0;
        while (len--) {
            if (c[len] != '_') out += parseNum(c[len]) * pow(base,n++);
        }

        return out;
    }

    bool parseDec(Node* node) {

        int64_t i = parseNum(node->literal.value,10);
        node->literal.i = i;
        node->literal.type = Type::i64;

        return true;
    }

    bool parseBinary(Node* node) {

        int64_t i = parseNum(node->literal.value+2,2);
        node->literal.i = i;
        node->literal.type = Type::i64;

        return true;
    }

    bool parseHex(Node* node) {

        int64_t i = parseNum(node->literal.value+2,16);
        node->literal.i = i;
        node->literal.type = Type::i64;

        return true;
    }

    bool parseOct(Node* node) {

        int64_t i = parseNum(node->literal.value+2,8);
        node->literal.i = i;
        node->literal.type = Type::i64;

        return true;
    }

    bool parseNumber(Node* node) {

        char* value = node->literal.value;
        if (value[0] == '0') {
            switch (value[1]) {
                case 'x':
                    return parseHex(node);
                case 'b':
                    return parseHex(node);
                case 'o':
                    return parseHex(node);
                default:
                    break;
            }
        }
        return parseDec(node);
    }

    bool parseString(Node* node) {
        
        char* in = node->literal.value;
        int len = 0;
        while (in[len]) len++;

        char* buf = new char[len];
        len = 0;
        int i = 1;
        bool ignoreNext = false;
        do {
            if (ignoreNext) continue;
            ignoreNext = false;
            if (in[i] == '\\') ignoreNext = true;
            else {
                buf[len] = in[i];
                len++;
            }
        } while (ignoreNext || in[i++] != '\"');

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

    bool parseLiteral(Node* node) {

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
                return parseNumber(node);

            default:
                printf("ERROR: %s:%d:%d: '%s' invalid literal!\n",node->token.file,node->token.line,node->token.column,node->literal.value);
                return false;
        }
    }

};



