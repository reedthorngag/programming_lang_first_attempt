

#ifndef DEBUGGING
#define DEBUGGING

#include "../parser/parser.hpp"

void print(Node* node, const char* id) {
    if (id) printf("ID: %s ",id);
    printf("Node state: %s",NodeTypeMap[(int)node->type]);
    switch (node->type) {
        case NodeType::FUNCTION:
        case NodeType::SYMBOL:
        case NodeType::INVOCATION:
            printf(" %s",node->symbol->name);
            break;
        case NodeType::OPERATION:
            printf(" %s %s",node->op.value,OpTypeMap[node->op.type]);
            break;
        case NodeType::LITERAL:
            printf(" %s",node->literal.value);
            if (node->literal.type) printf(" %s",TypeMap[node->literal.type]);
            break;
        default: 
            break;
    }
    printf(" token: %s:%d:%d\n",node->token.file.name,node->token.file.line,node->token.file.column);
}

void print(Token token, const char* id) {
    if (id) printf("ID: %s ",id);
    printf("Token state: %s:%d:%d %s",token.file.name,token.file.line,token.file.column,TokenTypeMap[token.type]);
    switch (token.type) {
        case TokenType::OPERATOR:
        case TokenType::LITERAL:
        case TokenType::TYPE:
        case TokenType::SYMBOL:
            printf(" %s",token.value);
            break;
        case TokenType::KEYWORD:
            printf(" %s",NodeTypeMap[token.keyword]);
            break;
        default:
            break;
    }
    printf("\n");
}

void print(Node* node) {
    print(node, nullptr);
}

void print(Token token) {
    print(token, nullptr);
}

#endif