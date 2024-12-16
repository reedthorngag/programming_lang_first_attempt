
#ifndef DEBUGGING
#define DEBUGGING

#include "../parser/parser.hpp"
#include "../lexer/lexer.hpp"

void print(Node* node, const char* src);

void print(Token token, const char* id);

void print(Node* node) {
    print(node, nullptr);
}

void print(Token token) {
    print(token, nullptr);
}

#endif