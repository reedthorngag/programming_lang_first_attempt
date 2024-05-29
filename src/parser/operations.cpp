
#include "parser.hpp"

namespace Parser {

    std::unordered_map<std::string, bool> assignmentOps = {
        {"=",1},
        {"+=",2},
        {"-=",2},
        {"/=",2},
        {"*=",2},
        {"%=",2},
        {"<<=",2},
        {">>=",2},
        {"^=",2},
        {"&=",2},
        {"|=",2},
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

    Node* functionCall(Symbol symbol) {
        Node* node = new Node{};
        node->type = NodeType::INVOCATION;
        node->symbol = symbol;

        bool inGrouping = false;
        bool global = false;
        Token token = tokens->at(index++);

        while (token.type != TokenType::GROUPING_END) {

            Node* param = new Node{};
            switch (token.type) {
                case TokenType::COMMA:
                    break;
                case TokenType::GROUPING_START:
                    inGrouping = true;
                    break;
                case TokenType::KEYWORD:
                    if (token.keyword != Keyword::GLOBAL) {
                        printf("ERROR: %s:%d:%d: unexpected keyword!\n",token.file,token.line,token.column);
                        return nullptr;
                    }
                    global = true;
                    token = tokens->at(index++);
                    if (token.type != TokenType::SYMBOL) {
                        printf("ERROR: %s:%d:%d: expecting name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                        return nullptr;
                    }
                    [[fallthrough]];
                case TokenType::SYMBOL: {
                    param->type = NodeType::SYMBOL;
                    Symbol symbol{};

                    if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
                        printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                        return nullptr;
                    }
                    global = false;
                    param->symbol = symbol;
                }
                    [[fallthrough]];
                case TokenType::LITERAL: {
                    
                    if (!(int)param->type) {
                        param->type = NodeType::LITERAL;
                        param->literal = Literal{token.value};
                    }

                    token = tokens->at(index++);

                    if (token.type == TokenType::COMMA || token.type == TokenType::GROUPING_END) {
                        inGrouping = inGrouping && token.type != TokenType::GROUPING_END;
                        if (inGrouping) {
                            printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                            return nullptr;
                        }
                        appendChild(node,param);
                        index--;
                        break;
                    }

                    if (token.type != TokenType::OPERATOR) {
                        printf("ERROR: %s:%d:%d: expecting operator, comma or close bracket, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                        return nullptr;
                    }

                    appendChild(node,operation(param,token));
                    break;
                }
                case TokenType::OPERATOR:
                    printf("not yet implemeted\n");
                    break;
                default:
                    printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
            }
            token = tokens->at(index++);
        }

        return node;
    }

    Node* assignment(Token token) {
        
        bool global = token.type == TokenType::KEYWORD && token.keyword == Keyword::GLOBAL;

        if (global) token = tokens->at(index++);

        if (token.type != TokenType::SYMBOL) {
            printf("ERROR: %s:%d:%d: expecting name, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        Symbol symbol;

        if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
            printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        Node* lvalue = new Node{};
        lvalue->type = NodeType::SYMBOL;
        lvalue->symbol = symbol;

        token = tokens->at(index++);

        if (token.type == TokenType::ENDLINE) return lvalue;
        if (token.type == TokenType::GROUPING_START) {
            delete lvalue;
            return functionCall(symbol);
        }

        // TODO: fix this
        if (!parent) {
            printf("ERROR: %s:%d:%d: top level assignments currently unsupported!\n",token.file,token.line,token.column);
            return nullptr;
        }

        if (token.type != TokenType::OPERATOR) {
            printf("ERROR: %s:%d:%d: expecting assignment or ';', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (auto key = assignmentOps.find(token.value); key == assignmentOps.end()) {
            printf("ERROR: %s:%d:%d: expecting assignment operator (=, +=, *=, etc), found '%s'!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        Node* opNode = operation(lvalue,token);
        if (!opNode) return nullptr;

        token = tokens->at(index);
        if (token.type != TokenType::ENDLINE) {
            printf("ERROR: %s:%d:%d: expecting ';', found %s!\n",token.file,token.line,token.column,token.type == TokenType::SYMBOL ? token.value : TokenTypeMap[token.type]);
            return nullptr;
        }
        return opNode;
    }

    inline OpType getOpType(char* op) {
        if (auto key = assignmentOps.find(op); key == assignmentOps.end()) return OpType::ASSIGNMENT;
        if (auto key = mathmaticalOps.find(op); key == mathmaticalOps.end()) return OpType::MATH;
        if (auto key = singleOperandOps.find(op); key == singleOperandOps.end()) return OpType::SINGLEOPERAND;
        return OpType::MATH; // keep the compiler happy
    }

    enum Order {
        LtoR,
        RtoL
    };

    struct Precedence {
        Order evalOrder;
        int precedence;
    };

    Precedence getPrecedence(Token token) {

        if (token.type != TokenType::OPERATOR) {
            printf("ERROR: %s:%d:%d: expecting operator, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return Precedence{};
        }

        if (auto key = singleOperandOps.find(token.value);key != singleOperandOps.end()) return Precedence{LtoR,key->second};
        if (auto key = assignmentOps.find(token.value);key != assignmentOps.end()) return Precedence{RtoL,key->second};
        if (auto key = mathmaticalOps.find(token.value);key != mathmaticalOps.end()) return Precedence{LtoR,key->second};
        
        printf("ERROR: %s:%d:%d: '%s' operator not yet implemented!\n",token.file,token.line,token.column,token.value);
        return Precedence{};
    }

    Node* processGrouping() {
        printf("here\n");

        Node* node;

        Token token = tokens->at(index++);

        bool global = false;
        switch (token.type) {
            case TokenType::GROUPING_START: {
                node = processGrouping();
                if (tokens->at(index).type == TokenType::OPERATOR) {
                    token = tokens->at(index++);
                    return operation(node,token);
                }
                return node;
            }
            case TokenType::KEYWORD:
                if (token.keyword != Keyword::GLOBAL) {
                    printf("ERROR: %s:%d:%d: unexpected keyword!\n",token.file,token.line,token.column);
                    return nullptr;
                }
                global = true;
                token = tokens->at(index++);
                if (token.type != TokenType::SYMBOL) {
                    printf("ERROR: %s:%d:%d: expecting symbol, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
                }
                [[fallthrough]];
            case TokenType::SYMBOL: {
                Symbol symbol{};

                if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
                    printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                    return nullptr;
                }

                if (tokens->at(index).type == TokenType::GROUPING_START) {
                    index++;
                    return functionCall(symbol);
                }
                node = new Node{};
                node->type = NodeType::SYMBOL;
                node->symbol = symbol;
            }
                [[fallthrough]];
            case TokenType::LITERAL: {
                if (!node) {
                    node = new Node{};
                    node->type = NodeType::LITERAL;
                    node->literal = Literal{token.value};
                }

                token = tokens->at(index++);

                if (token.type == TokenType::GROUPING_END)
                    return node;

                if (token.type != TokenType::OPERATOR) {
                    printf("ERROR: %s:%d:%d: expecting operator or ')', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
                }

                node = operation(node,token);
                if (!node) return nullptr;

                token = tokens->at(index-1);
                if (token.type != TokenType::GROUPING_END) {
                    printf("ERROR: %s:%d:%d: expecting operator or ')', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
                }

                return node;
            }
            case TokenType::OPERATOR:
                printf("not yet implemeted\n");
                break;
            default:
                printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                return nullptr;
        }
        return nullptr;
    }

    Node* operation(Node* lvalue, Token op) {
        printf("%s\n",op.value);

        Node* node = new Node{};
        node->type = NodeType::OPERATION;
        node->op = Operator{op.value,getOpType(op.value)};

        if (lvalue) appendChild(node, lvalue);
        
        Precedence lOp = getPrecedence(op);

        // var + 5 * 3; - top node + sub node *
        // var * 5 + 3; - top node + sub node *
        

        Token token = tokens->at(index++);

        Node* rvalue = new Node{};

        bool global = false;
        switch (token.type) {
            case TokenType::GROUPING_START: {
                Node* child = processGrouping();
                printf("herre\n");
                appendChild(node,child);
                return node;
            }
            
            case TokenType::KEYWORD:
                if (token.keyword != Keyword::GLOBAL) {
                    printf("ERROR: %s:%d:%d: unexpected keyword!\n",token.file,token.line,token.column);
                    return nullptr;
                }
                global = true;
                token = tokens->at(index++);
                if (token.type != TokenType::SYMBOL) {
                    printf("ERROR: %s:%d:%d: expecting symbol, found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                    return nullptr;
                }
                [[fallthrough]];
            case TokenType::SYMBOL: {
                rvalue->type = NodeType::SYMBOL;
                Symbol symbol{};

                if (!(!global && symbolDeclaredInScope(token.value,parent,&symbol)) && !(global && symbolDeclaredGlobal(token.value,&symbol))) {
                    printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                    return nullptr;
                }
                global = false;
                rvalue->symbol = symbol;
            }
                [[fallthrough]];
            case TokenType::LITERAL: {
                
                if (!(int)rvalue->type) {
                    rvalue->type = NodeType::LITERAL;
                    rvalue->literal = Literal{token.value};
                }

                token = tokens->at(index++);

                if (token.type == TokenType::ENDLINE || token.type == TokenType::COMMA || token.type == TokenType::GROUPING_END) {
                    appendChild(node,rvalue);
                    return node;
                }

                Precedence rOp = getPrecedence(token);
                if (!rOp.precedence) return nullptr;

                if (lOp.precedence == rOp.precedence && lOp.evalOrder == RtoL) rOp.precedence++;
                if (lOp.precedence >= rOp.precedence) {
                    appendChild(node,rvalue);
                    return operation(node,token);
                } else {
                    Node* child = operation(rvalue,token);
                    if (!child) return nullptr;
                    // wtf? next line is causing crash
                    appendChild(node,child);
                    return node;
                }
            }
            case TokenType::OPERATOR:
                printf("not yet implemeted\n");
                break;
            default:
                printf("ERROR: %s:%d:%d: unexpected %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
                return nullptr;
        }
        return nullptr;
    }
}
