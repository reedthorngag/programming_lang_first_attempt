#include <fstream>
#include <unordered_map>
#include <cstring>
#include <stack>
#include <sstream>
#include <vector>
#include <climits>

#include "compiler.hpp"
#include "../parser/parser.hpp"
#include "operations.hpp"

using namespace Parser;

namespace Compiler {

    bool buildScope(Context* parent); // who needs header files anyway?
    Reg callFunction(Node* funcCall, Context* context);
    Reg evaluate(Node* node, Context* context);

    std::ofstream* output;

    const std::unordered_map<std::string,Symbol*> globals;

    struct dataString {
        char* parentName;
        char* name;
        struct {
            char* str;
            int len;
        } value;
    };

    std::vector<dataString> strings;
    std::vector<Symbol*> globalSymbols;

    inline bool symbolDeclaredGlobal(char* name, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        auto key = globals.find(name);
        if (key != globals.end()) {
            if (symbol) *symbol = key->second;
            return true;
        }
        return false;
    }

    const char* toLocalLabel(const char* label) {
        int len = 0;
        while (label[len++]);
        char* str = new char[len+1];
        str[0] = '.';
        str++;
        while (len--) str[len] = label[len];
        str--;
        return str;
    }

    bool freeReg(Reg reg) {

        Value value = registers[reg].value;
        if (value.locked) return false;

        switch (value.type) {
            case ValueType::EMPTY:
                return true;

            case ValueType::GLOBAL:
                if (value.modified && value.preserveModified) {
                    std::stringstream fmt;
                    fmt << "[" << value.symbol->name << "]";
                    out("mov",
                            fmt.str(),
                            registers[reg].subRegs[TypeSizeMap[value.symbol->t]]
                        );
                }   
                value.symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;
            
            case ValueType::PARAMETER:
            case ValueType::LOCAL:
                if (value.modified && value.preserveModified) {
                    out("mov",
                            refLocalVar(value.local),
                            registers[reg].subRegs[TypeSizeMap[value.local->size]]
                        );
                }
                value.local->symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;

            default:
                return false;
        }

        return true;
    }

    Reg findFreeReg() {
        Reg reg = Reg::RAX;

        for (; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.type == ValueType::EMPTY) return reg;
        }

        int minRefCount = INT_MAX;
        Reg minRefCountReg = Reg::NUL;

        for (; reg != Reg::NUL; reg = (Reg)(reg-1)) {
            Value value = registers[reg].value;

            if (value.locked) continue;

            switch (value.type) {
                case ValueType::GLOBAL:
                    if (value.symbol->refCount < minRefCount) {
                        minRefCount = value.symbol->refCount;
                        minRefCountReg = reg;
                    }
                    break;
                
                case ValueType::PARAMETER:
                case ValueType::LOCAL:
                    if (value.local->symbol->refCount < minRefCount) {
                        minRefCount = value.local->symbol->refCount;
                        minRefCountReg = reg;
                    }
                    break;
                
                default:
                    break;
            }
        }

        freeReg(minRefCountReg); // dont need to check it succeeds as we already only get available regs

        return reg;
    }

    void pushReg(Reg reg) {

        switch (registers[reg].value.type) {
            case GLOBAL:
                out("push", registers[reg].subRegs[3]);
                registers[reg].value.symbol->location = Parser::Reg::STACK;
                registers[reg].value = {};
                break;
            case PARAMETER:
            case LOCAL:
                out("push", registers[reg].subRegs[3]);
                registers[reg].value.local->symbol->location = Parser::Reg::STACK;
                registers[reg].value = {};
                break;
            default:
                break;
        }
    }

    Reg operation(Node* node, Context* context) {

        Reg firstArg = evaluate(node->firstChild, context);
        registers[firstArg].value.locked = true;

        Reg secondArg = Reg::NUL;
        if (node->firstChild->nextSibling) {
            secondArg = evaluate(node->firstChild->nextSibling, context);
        }
        
        return doOp(node, firstArg, secondArg);
    }

    Reg evaluate(Node* node, Context* context) {

        switch (node->type) {
            case NodeType::SYMBOL: {

                Reg reg = findFreeReg();
                bool onStack = false;
                if (reg == Reg::NUL) { 
                    onStack = true;
                    pushReg(Reg::RAX);
                    reg = Reg::RAX;
                }

                Symbol* symbol;

                if (symbolDeclaredInScope(node->symbol->name,context->node,&symbol)) {
                    Local* local = context->locals->find(node->symbol->name)->second;

                    out("mov",registers[reg].subRegs[local->size],refLocalVar(local));

                    registers[reg].value.type = ValueType::LOCAL;
                    registers[reg].value.local = local;

                } else if (Compiler::symbolDeclaredGlobal(node->symbol->name,&symbol)) { // this covers builtins too
                    std::stringstream ss;
                    ss << "[" << node->symbol->name << "]";
                    out("mov",registers[reg].subRegs[TypeSizeMap[node->symbol->t]],ss.str());

                    registers[reg].value.type = ValueType::GLOBAL;
                    registers[reg].value.symbol = symbol;
                }

                if (onStack) {
                    out("pop", "rax");
                    return Reg::STACK;
                }
                
                return reg;
            }
            
            case NodeType::LITERAL: {

                Reg reg = findFreeReg();
                bool onStack = false;
                if (reg == Reg::NUL) {
                    onStack = true;
                    out("sub","rsp","8");
                    out("push","rax");
                    reg = Reg::RAX;
                }

                switch (node->literal.type) {
                    case Type::string: {
                        std::stringstream ss;
                        ss << node->token.file << node->token.line << node->token.column;
                        strings.push_back(dataString{context->node->symbol->name,(char*)ss.str().data(),node->literal.str.str,node->literal.str.len});
                        break;
                    }

                    case Type::chr: {
                        std::stringstream ss;
                        ss << '\'' << node->literal.chr << '\'';
                        out("mov", registers[reg].subRegs[0],ss.str());
                        break;
                    }

                    default:
                        out("mov", registers[reg].subRegs[TypeSizeMap[node->literal.type]],std::to_string(node->literal.u));
                        break;
                }

                if (onStack) {
                    out("mov", "rax","[rsp+8]");
                    out("pop", "rax");
                    return Reg::STACK;
                }
                
                return reg;
            }

            case NodeType::INVOCATION:
                return callFunction(node, context);

            default:
                break;   
        }

        return Reg::NUL;
    }

    Reg callFunction(Node* funcCall, Context* context) {

        std::stack<Node*> params;

        Node* paramNode = funcCall->firstChild;
        while (paramNode) {
            params.push(paramNode);
            paramNode = paramNode->nextSibling;
        }
        int p = funcCall->symbol->func->params->size();
        Param param;
        while (!params.empty()) {
            paramNode = params.top();
            params.pop();

            param = funcCall->symbol->func->params->at(--p);

            Reg reg = evaluate(paramNode,context);

            out("push",registers[reg].subRegs[3]);
        }

        for (Param p : *funcCall->symbol->func->params) {
            printf("hello: %s\n",registers[p.reg].subRegs[3]);
            if ((Reg)p.reg == Reg::STACK)
                break;
            
            if (registers[param.reg].value.type != ValueType::EMPTY) {
                printf("WARN: %s:%d:%d: register value not empty!\n",paramNode->token.file,paramNode->token.line,paramNode->token.column);
            }

            out("pop",registers[p.reg].subRegs[3]);
        }

        out("call",funcCall->symbol->name);

        return funcCall->symbol->func->returnType == Type::null ? Reg::NUL : Reg::RAX;
    }

    Context* functionSetup(Node* node) {

        int spaceReq = 0;

        Context* context = new Context{node,new std::unordered_map<std::string, Local*>};

        for (auto& pair : *node->symbolMap) {
            if (pair.second->refCount) {
                spaceReq += SizeByteMap[TypeSizeMap[pair.second->t]];
                Local* l = new Local{pair.second,spaceReq,TypeSizeMap[pair.second->t]};
                context->locals->insert(std::make_pair(pair.first,l));
            }
        }

        if (spaceReq == 0) return context;

        out("    push rbp");
        out("    mov rbp, rsp");
        out("sub","rsp",std::to_string(spaceReq));

        return context;
    }

    bool createFunction(Node* node) {
        (*output) << node->symbol->name << ":\n";

        Context* context = functionSetup(node);

        if (buildScope(context)) return false;

        if (context->locals->size()) {
            out("    mov rsp, rbp");
            out("    pop rbp");
        }
        out("    ret\n");

        return true;
    }

    // TODO: rename to something that makes more sense
    bool buildScope(Context* context) {

        Node* child = context->node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::BLOCK:
                    break;
                case NodeType::INVOCATION:
                    callFunction(child, context);
                    break;
                case NodeType::OPERATION:
                    evaluate(child, context);
                    break;
                default:
                    printf("ERROR: %s:%d:%d: unexpected node: %s!\n",child->token.file,child->token.line,child->token.column,NodeTypeMap[(int)child->type]);
                    return false;
            }
            child = child->nextSibling;
        }

        return true;
    }


    bool compile(std::unordered_map<std::string, Node*>* tree, std::ofstream* output) {
        Compiler::output = output;

        out("BITS 64");
        out("section .text");

        (*output) << "_start:\n";
        out("    call main");

        auto key = tree->find("main");
        if (key->second->symbol->func->returnType == Type::null) {
            out("    xor rax, rax");
        }

        out("    ret\n");

        for (auto& pair : *tree) {
            switch (pair.second->type) {
                case NodeType::FUNCTION:
                    if (!createFunction(pair.second)) return false;
                    break;

                case NodeType::SYMBOL:
                    globalSymbols.push_back(pair.second->symbol);
                    break;

                default:
                    // this can't happen
                    break;
            }
        }


        return true;
    }
};
