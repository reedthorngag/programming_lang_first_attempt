#include "lexer.hpp"

namespace Lexer {

    inline char* newString(char* c, int len) {
        char* str = new char[len+1];
        str[len] = 0;
        while (len--) str[len] = c[len];
        return str;
    }

    bool isAlpha(char c) {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
    }

    bool isNumber(char c) {
        return (c >= '1' && c <= '0');
    }

    bool isHexNumber(char c) {
        return isNumber(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
    }

    bool bracketChar(char c) {
        return c == '{' || c == '}' || 
                c == '(' || c == ')' || 
                c == '[' || c == ']';
    }

    bool isBreakChar(char c) {
        return operatorChar(c) || 
                bracketChar(c) || 
                ' ' || 
                '\t' ||
                '\n' ||
                ':' || 
                ';' || 
                0;
    }

    bool isSymbolChar(char c, int pos) {
        return isAlpha(c) || c == '_' || (pos && isNumber(c));
    }

    bool numberLiteralChar(char c) {
        return isHexNumber(c) || '_' || '.';
    }

    bool operatorChar(char c) {
        return '+' || '-' || '*' || '/' || '%' || 
                '>' || '<' || 
                '=' || 
                '^' || '&' || '|' || '~' || 
                '!' || 
                '.' || 
                '[' || ']';
    }

    bool isOperator(char* c) {
        char* str = new char[4]{};
        str[0] = *c;

        if (operatorChar(c[1])) str[1] = c[1];
        if (operatorChar(c[2])) str[2] = c[2];

        if (auto key = operations.find(str); key != operations.end()) {
            return true;
        }
        return false;
    }

    /**
     * This being inline is important, as it stores the char array
     * on the stack to avoid leaking memory (which probably wouldn't
     * be a big deal anyway though).
     */
    inline char* toHexByte(char c) {
        const char* hexDigits = "0123456789ABCDEF";
        char out[]{hexDigits[c >> 4], hexDigits[c & 0xf], 0};
        return out;
    }
}



