
#include "parser.hpp"

namespace Parser {

    std::unordered_map<std::string, bool> assignmentOps = {
        {"=",true},
        {"+=",true},
        {"-=",true},
        {"/=",true},
        {"*=",true},
        {"%=",true},
        {"<<=",true},
        {">>=",true},
        {"^=",true},
        {"&=",true},
        {"|=",true},
    };

    std::unordered_map<std::string, int> mathmaticalOps = {
        {"||",6},
        {"&&",7},
        {"==",8},
        {"!=",8},
        {"<=",9},
        {">=",9},
        {">",9},
        {"<",9},
        {"|",10},
        {"^",11},
        {"&",12},
        {"<<",13},
        {">>",13},
        {">>>",13},
        {"+",14},
        {"-",14},
        {"*",15},
        {"/",15},
        {"%",15},
    };

    std::unordered_map<std::string, int> singleOperandOps = {
        {"!",16},
        {"~",16},
        {"++",17},
        {"--",17},
    };


    bool assignment(Token token) {
        
        bool global = token.type == TokenType::KEYWORD && token.keyword == Keyword::GLOBAL;

        if (global) token = tokens->at(index++);

        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return false;
        }

        Symbol symbol;

        if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
            printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
            return false;
        }

        token = tokens->at(index++);

        if (token.type == TokenType::ENDLINE) return true;

        // TODO: fix this
        if (!parent) {
            printf("ERROR: %s:%d:%d: top level assignments currently unsupported!\n",token.file,token.line,token.column);
            return false;
        }

        if (token.type != TokenType::OPERATOR) {
            printf("ERROR: %s:%d:%d: expecting assignment or ';', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return false;
        }

        if (auto key = assignmentOps.find(token.value); key == assignmentOps.end()) {
            printf("ERROR: %s:%d:%d: expecting assignment operator (=, +=, *=, etc), found '%s'!\n",token.file,token.line,token.column,token.value);
            return false;
        }

        int i = 0;
        while (token.value[i+1]) i++;

        if (i) {
            token.value[i] = 0;
            Node* lvalue = new Node;
            lvalue->type = NodeType::SYMBOL;
            lvalue->symbol = symbol;

            Node* value = operation(lvalue,token);
            if (!value) return false;
            appendChild(parent,value);
            return true;
        }

        token = tokens->at(index++);

        Node* value = new Node;


        return true;
    }

    inline OpType getOpType(char* op) {
        if (auto key = assignmentOps.find(op); key == assignmentOps.end()) return OpType::ASSIGNMENT;
        if (auto key = mathmaticalOps.find(op); key == mathmaticalOps.end()) return OpType::MATH;
        if (auto key = singleOperandOps.find(op); key == singleOperandOps.end()) return OpType::SINGLEOPERAND;
    }

    Node* operation(Node* lvalue, Token op) {

        Node* node = new Node;
        node->type = NodeType::OPERATION;
        node->op = Operator{op.value,getOpType(op.value)};

        if (lvalue) appendChild(node, lvalue);

        // var + 5 * 3;
        // var * 5 + 3;

        Token token = tokens->at(index++);

        switch (token.type) {
            case TokenType::GROUPING_START:
                break;
            case TokenType::LITERAL:
                break;
            case TokenType::KEYWORD:
            case TokenType::SYMBOL:
                break;
            case TokenType::OPERATOR:
                break;
        }

    }
}
