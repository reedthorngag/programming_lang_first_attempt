#include <fstream>
#include <stdio.h>

#include "lexer.hpp"
#include "parser.hpp"

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

    std::vector<Lexer::Token>* tokens = Lexer::lexerParse(inputFile, buf);
    if (!tokens) return 1;

    const char* TokenTypeMap[]{
        "ENDLINE",
        "BRACE",
        "BRACKET",
        "KEYWORD",
        "TYPE",
        "SYMBOL",
        "OPERATOR",
        "LITERAL"
    };

    const char* SymbolTypeMap[]{
        "FUNCTION",
        "VARIABLE",
        "CONSTANT",
        "IF",
        "ELSE",
        "WHILE"
    };

    printf("Input file: %s\n%s\n",inputFile,buf);

    printf("Tokens length: %d\n",(int)tokens->size());
    for (int i = 0; i < (int)tokens->size(); i++) {
        Lexer::Token token = tokens->at(i);
        printf("token: %s %s\n",TokenTypeMap[token.type], 
                token.type == Lexer::TokenType::SYMBOL ? token.value : 
                token.type == Lexer::TokenType::KEYWORD ? SymbolTypeMap[token.keyword] : 
                token.type == Lexer::TokenType::LITERAL ? token.value : 
                token.type == Lexer::TokenType::TYPE ? token.value : 
                token.type == Lexer::TokenType::OPERATOR ? token.c : ""
            );
    }

    if (Parser::parseTokens(tokens)) return 1;

    return 0;
}

