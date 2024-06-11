
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
        node->token = tokens->at(index-1);

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
                    (*symbol.refCount)++;
                    global = false;
                    param->symbol = symbol;
                    param->token = token;
                }
                    [[fallthrough]];
                case TokenType::LITERAL: {
                    
                    if (!(int)param->type) {
                        param->type = NodeType::LITERAL;
                        param->literal = Literal{Type::null,{.value = {token.value}}};
                        param->token = token;
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
                default:
                    if (tokens->at(index-2).type == TokenType::GROUPING_END) {
                        index -= 2;
                        break;
                    }
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

        if (global) {
            if (!symbolDeclaredGlobal(token.value,&symbol)) {
                printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
                return nullptr;
            }
        } else if (!symbolDeclaredInScope(token.value,parent,&symbol) && !symbolDeclaredGlobal(token.value,&symbol)) {
            printf("ERROR: %s:%d:%d: '%s' undefined name!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }
        (*symbol.refCount)++;

        Node* lvalue = new Node{};
        lvalue->type = NodeType::SYMBOL;
        lvalue->symbol = symbol;
        lvalue->token = token;

        token = tokens->at(index++);

        if (token.type == TokenType::ENDLINE) return lvalue;
        if (token.type == TokenType::GROUPING_START) {
            if (symbol.type != SymbolType::FUNC && (!symbolDeclaredGlobal(lvalue->token.value,&symbol) || symbol.type != SymbolType::FUNC)) {
                printf("ERROR: %s:%d:%d: '%s' isn't a function!\n",lvalue->token.file,lvalue->token.line,lvalue->token.column,lvalue->token.value);
                return nullptr;
            }
            (*lvalue->symbol.refCount)--;
            delete lvalue;
            (*symbol.refCount)++;
            return functionCall(symbol);
        }

        if (token.type != TokenType::OPERATOR) {
            printf("ERROR: %s:%d:%d: expecting assignment or ';', found %s!\n",token.file,token.line,token.column,TokenTypeMap[token.type]);
            return nullptr;
        }

        if (auto key = assignmentOps.find(token.value); key == assignmentOps.end()) {
            printf("ERROR: %s:%d:%d: expecting assignment operator (=, +=, *=, etc), found '%s'!\n",token.file,token.line,token.column,token.value);
            return nullptr;
        }

        if (symbol.type == SymbolType::FUNC && (!symbolDeclaredGlobal(lvalue->token.value,&symbol) || symbol.type == SymbolType::FUNC)) {
            printf("ERROR: %s:%d:%d: '%s' is a function!\n",lvalue->token.file,lvalue->token.line,lvalue->token.column,lvalue->token.value);
            return nullptr;
        }

        if (symbol.type == SymbolType::CONST && tokens->at(index-2).type != TokenType::TYPE) {
            printf("ERROR: %s:%d:%d: '%s' is a constant and can't be assigned to!\n",token.file,token.line,token.column,lvalue->token.value);
            return nullptr;
        }

        Node* opNode = operation(lvalue,token);
        if (!opNode) return nullptr;

        token = tokens->at(index-1);
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
        Node* node = nullptr;

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
                (*symbol.refCount)++;

                if (tokens->at(index).type == TokenType::GROUPING_START) {
                    index++;
                    return functionCall(symbol);
                }
                node = new Node{};
                node->type = NodeType::SYMBOL;
                node->symbol = symbol;
                node->token = token;
            }
                [[fallthrough]];
            case TokenType::LITERAL: {
                if (!node) {
                    node = new Node{};
                    node->type = NodeType::LITERAL;
                    node->literal = Literal{Type::null,{.value = {token.value}}};
                    node->token = token;
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
        //printf("%s %s: %lld %d %s\n",op.value,NodeTypeMap[(int)lvalue->type],(long long)lvalue,(int)lvalue->type,lvalue->type == NodeType::LITERAL ? lvalue->literal.value : "");

        Node* node = new Node{};
        node->type = NodeType::OPERATION;
        node->op = Operator{op.value,getOpType(op.value)};
        node->token = op;

        if (lvalue) appendChild(node, lvalue);
        
        Precedence lOp = getPrecedence(op);

        Token token = tokens->at(index++);

        Node* rvalue = new Node{};

        bool global = false;
        switch (token.type) {
            case TokenType::GROUPING_START: {
                delete rvalue;
                rvalue = processGrouping();
                goto processNext;
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
                (*symbol.refCount)++;
                global = false;
                rvalue->symbol = symbol;
                rvalue->token = token;
            }
                [[fallthrough]];
            case TokenType::LITERAL: {
                
                if (!(int)rvalue->type) {
                    rvalue->type = NodeType::LITERAL;
                    rvalue->literal = Literal{Type::null,{.value = {token.value}}};
                    rvalue->token = token;
                }
                processNext: // couldnt think of a better name

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
