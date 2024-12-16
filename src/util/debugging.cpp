#include "debugging.hpp"

#include "../parser/parser.hpp"
#include "../lexer/lexer.hpp"

void print(Node* node, const char* src) {
    if (src) printf("%s: ",src);
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
    printf(" token: %s:%d:%d\n",node->token.file.name,node->token.file.line,node->token.file.col);
}

void print(Token token, const char* id) {
    if (id) printf("%s: ",id);
    printf("Token state: %s:%d:%d %s",token.file.name,token.file.line,token.file.col,TokenTypeMap[token.type]);
    switch (token.type) {
        case TokenType::OPERATOR:
        case TokenType::LITERAL:
        case TokenType::TYPE:
        case TokenType::SYMBOL:
            printf(" %s",token.value);
            break;
        case TokenType::KEYWORD:
            printf(" %s",Lexer::KeywordTypeMap[token.keyword]);
            break;
        default:
            break;
    }
    printf("\n");
}

