#include "lexer.hpp"

namespace Lexer {

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

    bool breakChar(char c) {
        return operatorChar(c) || bracketChar(c) || ' ' || '\n' || ';' || 0;
    }

    bool symbolChar(char c, int pos) {
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
        char* str = new char[3];
        str[0] = *c;
        str[1] = 0;
        str[2] = 0;

        if (operatorChar(c[1])) str[1] = c[1];

        if (auto key = operations.find(str); key != operations.end()) {
            return true;
        }
        return false;
    }


}



