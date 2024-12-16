#include <fstream>
#include <stdio.h>

#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "typechecker/typechecker.hpp"
#include "compiler/compiler.hpp"

void pad(Token token, int* line, int* col) {
    while (*line < token.file.line) {
        printf("\n");
        (*line)++;
        *col = 1;
    }
    while (*col < token.file.col) {
        printf(" ");
        (*col)++;
    }
}

void printVal(Token token, int* col) {
    char* ptr = token.value;
    while (*ptr) {
        printf("%c",*(ptr++));
        (*col)++;
    }
}

void printNode(Node* node, int depth) {
    int d = depth;
    while (d--) printf(" | ");
    printf("%s",NodeTypeMap[(int)node->type]);
    switch (node->type) {
        case NodeType::SYMBOL:
            printf(": %s: %s\n",node->symbol->name,TypeMap[node->symbol->t]);
            break;
        case NodeType::FUNCTION:
            printf(": %s\n",node->symbol->name);
            break;
        case NodeType::LITERAL:
            printf(": %s%s\n",(node->literal.negative ? "-": ""),node->literal.value);
            break;
        case NodeType::OPERATION:
            printf(": %s %s\n",node->op.value, OpTypeMap[node->op.type]);
            break;
        case NodeType::INVOCATION:
            printf(": %s %lld\n",node->symbol->name,(long long)node->symbol->func);
            break;
        default:
            printf("\n");
            break;
    }
    Node* child = node->firstChild;
    while (child) {
        printNode(child,depth+1);
        child = child->nextSibling;
    }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("No input file provided.\n");
        return 1;
    }

    char* inputFile = argv[1];

    std::ifstream file(inputFile, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        printf("Failed to open %s\n",inputFile);
        exit(1);
    }

    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buf = new char[size+1];
    buf[size] = 0;
    if (!file.read(buf,size)) {
        printf("Failed to read from %s\n",inputFile);
        exit(1);
    }

    file.close();

    std::vector<Token>* tokens = Lexer::parse(inputFile, buf);
    if (!tokens) return 1;

    const char* TokenTypeMap[]{
        "ENDLINE",
        "COMMA",
        "SCOPE_START",
        "SCOPE_END",
        "GROUPING_START",
        "GROUPING_END",
        "KEYWORD",
        "SYMBOL",
        "TYPE",
        "OPERATOR",
        "LITERAL",
        "EOF",
        "ARRAY_START",
        "ARRAY_END"
    };

    printf("Input file: %s\n%s\n",inputFile,buf);

    printf("Tokens length: %d\n",(int)tokens->size());

    int line = 0;
    int col = 1;
    for (int i = 0; i < (int)tokens->size(); i++) {
        Token token = tokens->at(i);
        printf("token: %s %s\n",TokenTypeMap[token.type], 
                token.type == TokenType::SYMBOL ? token.value : 
                token.type == TokenType::KEYWORD ? KeywordTypeMap[token.keyword] : 
                token.type == TokenType::LITERAL ? token.value : 
                token.type == TokenType::TYPE ? token.value : 
                token.type == TokenType::OPERATOR ? token.value : ""
            );
    }

    for (int i = 0; i < (int)tokens->size(); i++) {
        Token token = tokens->at(i);

        switch (token.type) {
            case TokenType::KEYWORD:
                pad(token,&line,&col);
                token.value = (char*)KeywordTypeMap[token.keyword];
                printVal(token,&col);
                if (tokens->at(i+1).type != TokenType::ENDLINE) {
                    printf(" ");
                    col++;
                }
                break;
            case TokenType::SYMBOL:
                pad(token,&line,&col);
                printVal(token,&col);
                if (tokens->at(i+1).type != TokenType::ENDLINE && 
                    tokens->at(i+1).type != TokenType::GROUPING_START &&
                    tokens->at(i+1).type != TokenType::GROUPING_END &&
                    tokens->at(i+1).type != TokenType::TYPE) {
                    printf(" ");
                    col++;
                }
                break;
            case TokenType::LITERAL:
                pad(token,&line,&col);
                printVal(token,&col);
                if (tokens->at(i+1).type != TokenType::ENDLINE &&
                    tokens->at(i+1).type != TokenType::GROUPING_END) {
                    printf(" ");
                    col++;
                }
                break;
            case TokenType::TYPE:
                printf(":");
                col++;
                pad(token,&line,&col);
                printVal(token,&col);
                if (tokens->at(i+1).type != TokenType::ENDLINE &&
                    tokens->at(i+1).type != TokenType::GROUPING_END) {
                    printf(" ");
                    col++;
                }
                break;
            case TokenType::OPERATOR:
                pad(token,&line,&col);
                printVal(token,&col);
                printf(" ");
                col++;
                break;
            case TokenType::GROUPING_START:
                pad(token,&line,&col);
                printf("(");
                col++;
                break;
            case TokenType::GROUPING_END:
                pad(token,&line,&col);
                printf(")");
                col++;
                break;
            case TokenType::SCOPE_START:
                pad(token,&line,&col);
                printf("{");
                col++;
                break;
            case TokenType::SCOPE_END:
                pad(token,&line,&col);
                printf("}\n");
                col = 1;
                line++;
                break;
            case TokenType::ARRAY_START:
                pad(token,&line,&col);
                printf("[");
                col++;
                break;
            case TokenType::ARRAY_END:
                pad(token,&line,&col);
                printf("]");
                col++;
                break;
            case TokenType::ENDLINE:
                pad(token,&line,&col);
                printf(";");
                col++;
                break;
            case TokenType::COMMA:
                pad(token,&line,&col);
                printf(",");
                col++;
                break;
            case TokenType::FILE_END:
                break;
        }

    }
    fflush(stdout);

    std::unordered_map<std::string, Node*>* tree = Parser::parseTokens(tokens);
    if (!tree) return 1;

    printf("syntax tree:\n");
    for (auto& pair : *tree) {
        printNode(pair.second,0);
        printf("\n");
    }

    printf("Parser completed successfully!\n");

    if (!TypeChecker::process(tree)) {
        printf("Type check failed!\n");
        return 1;
    } else {
        printf("Type check passed!\n");
    }

    if (auto key = tree->find("main");key != tree->end()) {
        switch (key->second->symbol->func->returnType) {
                case Type::i8:
                case Type::i16:
                case Type::i32:
                case Type::i64:
                case Type::u8:
                case Type::u16:
                case Type::u32:
                case Type::u64:
                case Type::null:
                case Type::boolean:
                    break;
                default:
                    printf("ERROR: Main function return type invalid!\n");
                    return 1;
            }
    } else {
        printf("ERROR: No main function!\n");
        return 1;
    }

    int len = 0;
    for (; inputFile[len++];);
    for (;inputFile[--len] != '.';);
    len++;

    char* outFileName = new char[len+4];
    outFileName[len] = 'a';
    outFileName[len+1] = 's';
    outFileName[len+2] = 'm';
    outFileName[len+3] = 0;
    
    for (;len--;) outFileName[len] = inputFile[len];

    printf("Out file: %s\n",outFileName);

    std::ofstream outFile(outFileName, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        printf("Failed to open %s\n",outFileName);
        exit(1);
    }
    
    if (!Compiler::compile(tree, &outFile)) {
        printf("Compile failed!\n");
        return 1;
    } else {
        printf("Compile succeeded!\n");
    }

    outFile.close();

    return 0;
}

