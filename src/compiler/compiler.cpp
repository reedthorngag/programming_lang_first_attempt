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

    std::unordered_map<std::string, Node*> typedFunctions;
    std::unordered_map<std::string, Symbol*> typedBuiltins;

    struct FuncCall{
        std::string name;
        Node* node;
    };

    std::vector<FuncCall> undefinedFunctions;

    struct dataString {
        char* name;
        struct {
            char* str;
            int len;
        } value;
    };

    std::vector<dataString> strings;
    std::vector<Symbol*> globalSymbols;

    unsigned int position = 0;
    unsigned int labelNum = 0;

    inline bool symbolDeclaredGlobal(char* name, Symbol** symbol) {
        if (symbolBuiltin(name, symbol)) return true;
        auto key = globals.find(name);
        if (key != globals.end()) {
            if (symbol) *symbol = key->second;
            return true;
        }
        return false;
    }

    Local* getLocal(char* name, Context* startContext) {
        Context* context = startContext;
        do {
            if (auto local = context->locals->find(name); local != context->locals->end()) {
                return local->second;
            }
            context = context->parent;
        } while (context);

        // this shouldn't happen as this should be called after
        // getting the symbol with symbolDeclaredInScope()
        return nullptr;
    }

    bool freeReg(Reg reg, bool dontEmpty) {

        //printf("attempting to free %d %s, %d %d\n",dontEmpty, registers[reg].subRegs[Size::QWORD],registers[reg].value.type,(int)registers[reg].value.modified);

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
                if (dontEmpty) {
                    if (value.preserveModified)
                        registers[reg].value.modified = false;
                    break;
                }
                value.symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;
            
            case ValueType::PARAMETER:
            case ValueType::LOCAL:
                if (value.modified && value.preserveModified) {
                    out("mov",
                            refLocalVar(value.local),
                            registers[reg].subRegs[value.local->size]
                        );
                }
                if (dontEmpty) {
                    if (value.preserveModified)
                        registers[reg].value.modified = false;
                    break;
                }
                value.local->symbol->location = (Parser::Reg)Reg::NUL;
                registers[reg].value = Value{};
                break;

            case ValueType::INTERMEDIATE:
                registers[reg].value = Value{};
                break;

            default:
                return false;
        }

        return true;
    }

    bool freeReg(Reg reg) {
        return freeReg(reg, false);
    }

    Reg findFreeReg() {

        // return first empty or no longer used intermediate reg
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.type == ValueType::EMPTY) return reg;
            if (registers[reg].value.type == ValueType::INTERMEDIATE && !registers[reg].value.locked) {
                registers[reg].value = Value{};
                return reg;
            }
        }

        unsigned int minRegPosition = UINT_MAX;
        Reg firstInReg = Reg::NUL;

        for (Reg reg = Reg::RAX; reg != Reg::NUL; reg = (Reg)(reg-1)) {
            Value value = registers[reg].value;

            if (value.locked) continue;

            if (registers[reg].position < minRegPosition) {
                minRegPosition = registers[reg].position;
                firstInReg = reg;
            }
        }

        freeReg(firstInReg);

        return firstInReg;
    }

    Reg operation(Node* node, Context* context) {
        
        //printf("operation: %d: %s\n",node->token.line,node->op.value);

        Reg firstArg = evaluate(node->firstChild, context);
        registers[firstArg].value.locked = true;

        Reg secondArg = Reg::NUL;
        if (node->firstChild->nextSibling) {
            secondArg = evaluate(node->firstChild->nextSibling, context);
            if (secondArg == Reg::NUL) return Reg::NUL;
        }

        registers[firstArg].value.locked = false;

        printf("op: %s\n",node->token.value);
        
        //printf("operation: %d: %s %d  %s  %s %d\n",node->token.line,registers[firstArg].subRegs[Size::QWORD],registers[firstArg].value.type,node->op.value,registers[secondArg].subRegs[Size::QWORD],registers[secondArg].value.type);
        return doOp(node, firstArg, secondArg);
    }

    Reg evaluate(Node* node, Context* context) {

        switch (node->type) {
            case NodeType::SYMBOL: {

                for (Reg r = Reg::RAX; r != Reg::RBP; r=(Reg)(r+1)) {
                    Value v = registers[r].value;
                    char* name = nullptr;
                    if (v.symbol) {
                        switch (v.type) {
                            case ValueType::GLOBAL:
                                name = v.symbol->name;
                                break;
                            case ValueType::PARAMETER:
                            case ValueType::LOCAL:
                                name = v.local->symbol->name;
                                break;
                            default: break;
                        }

                        // this actually tests the equality of the pointers, not the strings themselves,
                        // which works, because everything (pretty much) uses the same string and just
                        // passes around pointers. be aware of this when modifying stuff tho.
                        if (name && name == node->symbol->name) {
                            if (v.modified && !v.preserveModified) break;

                            if (v.locked) {
                                Reg reg = findFreeReg();
                                if (reg == Reg::NUL) { 
                                    printf("ERROR: %s:%d:%d: no registers available!\n",node->token.file,node->token.line,node->token.column);
                                    return Reg::RSI;
                                }

                                assign(reg,r);
                                registers[reg].value = Value{ValueType::INTERMEDIATE,{.symbol={nullptr}},false,false,false};
                                return reg;
                            }

                            return r;
                        }
                    }
                }

                Reg reg = findFreeReg();
                if (reg == Reg::NUL) { 
                    printf("ERROR: %s:%d:%d: no registers available!\n",node->token.file,node->token.line,node->token.column);
                    return Reg::RSI;
                }

                Symbol* symbol;

                if (symbolDeclaredInScope(node->symbol->name,context->node,&symbol)) {
                    Local* local = getLocal(node->symbol->name, context);

                    if (SizeByteMap[local->size] >= 4) { // 32 bit loads clear top 32 bits (e.g mov eax, eax clears top 32 bits)
                        out("mov", registers[reg].subRegs[local->size],refLocalVar(local));
                    } else {
                        std::stringstream ss;
                        ss << SizeTypeMap[local->size] << ' ' << refLocalVar(local) << "; var: " << node->symbol->name;
                        out("movzx qword",registers[reg].subRegs[Size::QWORD], ss.str().c_str());
                    }

                    registers[reg].value = Value{};
                    registers[reg].value.type = ValueType::LOCAL;
                    registers[reg].value.local = local;
                    registers[reg].position = position++;

                } else if (Compiler::symbolDeclaredGlobal(node->symbol->name,&symbol)) { // this covers builtins too
                    std::stringstream ss;
                    ss << "[" << node->symbol->name << "]";
                    out("mov",registers[reg].subRegs[TypeSizeMap[node->symbol->t]],ss.str());

                    if (SizeByteMap[TypeSizeMap[node->symbol->t]] >= 4) { // 32 bit loads clear top 32 bits
                        out("mov", registers[reg].subRegs[TypeSizeMap[node->symbol->t]],ss.str());
                    } else {
                        std::stringstream ss2;
                        ss2 << SizeTypeMap[TypeSizeMap[node->symbol->t]] << ' ' << ss.str();
                        out("movzx qword",registers[reg].subRegs[Size::QWORD], ss2.str().c_str());
                    }

                    registers[reg].value = Value{};
                    registers[reg].value.type = ValueType::GLOBAL;
                    registers[reg].value.symbol = symbol;
                    registers[reg].position = position++;
                }

                return reg;
            }
            
            case NodeType::LITERAL: {

                Reg reg = findFreeReg();
                if (reg == Reg::NUL) { 
                    printf("ERROR: %s:%d:%d: no registers available!\n",node->token.file,node->token.line,node->token.column);
                    exit(0);
                }

                switch (node->literal.type) {
                    case Type::string: {
                        std::stringstream ss;
                        ss << '_' << node->token.file << '_' << std::to_string(node->token.line) << '_' << std::to_string(node->token.column);
                        std::string* s = new std::string(ss.str().c_str());
                        for (int i = 0; i < (int)s->length(); i++) {
                            if ((*s)[i] == '.') (*s)[i] = '_';
                            if ((*s)[i] == '/') (*s)[i] = '_';
                        }
                        strings.push_back(dataString{(char*)s->c_str(),node->literal.str.str,node->literal.str.len});

                        out("mov", registers[reg].subRegs[Size::QWORD], *s);

                        registers[reg].value = Value{};
                        registers[reg].value.type = ValueType::INTERMEDIATE;
                        registers[reg].value.symbol = node->symbol;
                        registers[reg].value.preserveModified = false;
                        registers[reg].position = position++;

                        break;
                    }

                    case Type::chr: {
                        out("mov ", registers[reg].subRegs[Size::QWORD],std::to_string(node->literal.chr));

                        registers[reg].value = Value{};
                        registers[reg].value.type = ValueType::INTERMEDIATE;
                        registers[reg].value.symbol = node->symbol;
                        registers[reg].value.preserveModified = false;
                        registers[reg].position = position++;

                        break;
                    }

                    default:
                        out("mov", registers[reg].subRegs[TypeSizeMap[node->literal.type]],std::to_string(node->literal.u));
                        
                        registers[reg].value = Value{};
                        registers[reg].value.type = ValueType::INTERMEDIATE;
                        registers[reg].value.symbol = node->symbol;
                        registers[reg].value.preserveModified = false;
                        registers[reg].position = position++;

                        break;
                }
                
                return reg;
            }

            case NodeType::OPERATION:
                return operation(node, context);

            case NodeType::INVOCATION:
                return callFunction(node, context);

            default:
                break;   
        }

        return Reg::NUL;
    }

    bool createReturn(Node* node, Context* context) {

        if (!context) {
            printf("ERROR: %s:%d:%d: return only allowed in functions!",node->token.file,node->token.line,node->token.column);
            return false;
        }

        if (node->firstChild) {
            Reg reg = evaluate(node->firstChild, context);

            if (!reg) return false;
            out("mov",registers[Reg::RAX].subRegs[Size::QWORD],registers[reg].subRegs[Size::QWORD]);
        }

        do {
            if (context->locals->size()) {
                out("add", "rsp",std::to_string(context->spaceReq));
                out("    leave");
            }
            context = context->parent;
        } while (context);

        out("    ret\n");
        return true;
    }

    bool insertBreak(Node* node, Context* context) {

        while (context) {
            if (context->breakLabel) {
                out("jmp",*context->breakLabel);
                return true;
            }
            context = context->parent;
        }

        printf("ERROR: %s:%d:%d: Break not allowed here! Must be in while, for or switch!",node->token.file,node->token.line,node->token.column);
        return false;
    }

    bool insertContinue(Node* node, Context* context) {

        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.modified && registers[reg].value.preserveModified) {
                freeReg(reg, true);
            }
        }

        while (context) {
            if (context->continueLabel) {
                out("jmp",*context->continueLabel);
                return true;
            }
            context = context->parent;
        }

        printf("ERROR: %s:%d:%d: Continue not allowed here! Must be in while, for or switch!",node->token.file,node->token.line,node->token.column);
        return false;
    }

    Reg callFunction(Node* funcCall, Context* context) {

        //printf("saving registers before %s func call, line %d\n",funcCall->symbol->name,funcCall->token.line);
        // save registers, as the function call will batter them
        struct RegData {
            Reg reg;
            Value value;
            unsigned int position;
        };
        std::stack<RegData> pushedRegs;
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (!freeReg(reg, true)) {
                if (reg == Reg::RAX && funcCall->symbol->func->returnType != Type::null) {
                    printf("ERROR: %s:%d:%d: no registers available!\nCompile failed!",funcCall->token.file,funcCall->token.line,funcCall->token.column);
                    return Reg::RSI;
                }
                printf("saving %s\n",registers[reg].subRegs[Size::QWORD]);
                pushedRegs.push(RegData{reg,registers[reg].value,registers[reg].position});
                out("push", registers[reg].subRegs[Size::QWORD]);
                registers[reg].value = Value{};
            }
        }

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

            if (reg == Reg::NUL) return Reg::RSI;

            // TODO: optimize this to only push reg if necessary

            out("push",registers[reg].subRegs[3]);

            freeReg(reg);
        }

        // clear all register values, the function call will batter them
        // if they have been modified in between this and the last call, then they will be saved again
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (!freeReg(reg)) {
                registers[reg].value = Value{};
            }
        }

        for (Param p : *funcCall->symbol->func->params) {
            if ((Reg)p.reg == Reg::STACK)
                break;

            out("pop",registers[p.reg].subRegs[3]);

        }

        std::stringstream ss;
        ss << funcCall->symbol->name;
        for (Param p : *funcCall->symbol->func->params) {
            ss << '_' << TypeMap[p.type];
        }

        if (auto key = typedFunctions.find(ss.str()); key == typedFunctions.end()) {
            undefinedFunctions.push_back(FuncCall{*new std::string(ss.str()),funcCall});
        }

        out("call",ss.str());

        while (!pushedRegs.empty()) {
            RegData r = pushedRegs.top();
            pushedRegs.pop();
            out("pop",registers[r.reg].subRegs[Size::QWORD]);
            registers[r.reg].position = r.position;
            registers[r.reg].value = r.value;
            printf("retrieving %s\n",registers[r.reg].subRegs[Size::QWORD]);
        }

        return funcCall->symbol->func->returnType == Type::null ? Reg::NUL : Reg::RAX;
    }

    Context* functionSetup(Node* node) {

        int spaceReq = 0;

        Context* context = new Context{node,nullptr,new std::unordered_map<std::string, Local*>,0};

        for (auto& pair : *node->symbolMap) {
            //if (pair.second->refCount) { // causes segfault when func param isnt used, need to fix at some point
                spaceReq += SizeByteMap[TypeSizeMap[pair.second->t]];
                Local* l = new Local{pair.second,spaceReq,TypeSizeMap[pair.second->t]};
                context->locals->insert(std::make_pair(pair.first,l));
            //}
        }

        if (spaceReq == 0) return context;

        context->spaceReq = spaceReq;

        out("    push rbp");
        out("    mov rbp, rsp");
        out("sub","rsp",std::to_string(spaceReq));

        int offset = 0;
        for (auto& pair : *node->symbolMap) {
            if (pair.second->refCount) {
                offset += SizeByteMap[TypeSizeMap[pair.second->t]];
                std::stringstream ss;

                // if the symbol is a function param, put that instead of zero
                Param param = {};
                for (Param p : *node->symbol->func->params) {
                    if (p.name == pair.second->name) {
                        param = p;
                        break;
                    }
                }

                ss << "\tmov " << SizeString[TypeSizeMap[pair.second->t]] << " [rbp-" << offset << "], ";
                if (param.reg) ss << registers[param.reg].subRegs[TypeSizeMap[pair.second->t]];
                else ss << '0';
                out(ss.str());
            }
        }

        return context;
    }

    bool createFunction(Node* node) {

        std::stringstream ss;
        ss << node->symbol->name;

        for (Param p : *node->symbol->func->params) ss << '_' << TypeMap[p.type];

        typedFunctions.insert(std::make_pair(*new std::string(ss.str()), node));

        ss << ':';
        out(ss.str());

        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (!freeReg(reg)) {
                printf("ERROR: %s:%d:%d: register locked when it shouldn't be! can't free %s!\n",node->token.file,node->token.line,node->token.column,registers[reg].subRegs[Size::QWORD]);
                return false;
            }
        }

        Context* context = functionSetup(node);
        
        for (Param p : *node->symbol->func->params) {
            registers[p.reg].value = Value{ValueType::LOCAL,{.local = context->locals->find(p.name)->second},false,true,false};
            registers[p.reg].position = position++;
        }

        if (!buildScope(context)) return false;

        Node* ret = node->firstChild;
        if (ret) {
            while (ret->nextSibling) ret = ret->nextSibling;
            if (ret->type == NodeType::RETURN) return true; // return would already be added in buildScope func
        }

        if (context->locals->size()) {
            out("add", "rsp",std::to_string(context->spaceReq));
            out("    leave");
        }
        out("    ret\n");

        return true;
    }

    bool createScope(Node* node, Context* context) {

        Context scopeContext = Context{node,context,new std::unordered_map<std::string, Local*>,0};
        
        int parentOffset = 0;
        Context* tmpContext = context;
        while (tmpContext) {
            parentOffset += tmpContext->spaceReq;
            tmpContext = tmpContext->parent;
        }

        int spaceReq = 0;

        for (auto& pair : *node->symbolMap) {
            if (pair.second->refCount) {
                spaceReq += SizeByteMap[TypeSizeMap[pair.second->t]];
                Local* l = new Local{pair.second,spaceReq+parentOffset,TypeSizeMap[pair.second->t]};
                scopeContext.locals->insert(std::make_pair(pair.first,l));
            }
        }

        if (spaceReq) {

            scopeContext.spaceReq = spaceReq;

            out("sub","rsp",std::to_string(spaceReq));

            int offset = 0;
            for (auto& pair : *node->symbolMap) {
                if (pair.second->refCount) {
                    offset += SizeByteMap[TypeSizeMap[pair.second->t]];
                    std::stringstream ss;
                    ss << "\tmov " << SizeString[TypeSizeMap[pair.second->t]] << " [rbp-" << offset + parentOffset << "], 0";
                    out(ss.str());
                }
            }

        }

        if (!buildScope(&scopeContext)) return false;

        if (spaceReq) {
            out("add", "rsp",std::to_string(spaceReq));
        }

        return true;
    }

    Node* buildIf(Node* node, Context* context) {

        Reg reg = evaluate(node->firstChild, context);

        if (reg == Reg::NUL) return (Node*)-1;

        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            freeReg(reg, false);
        }

        unsigned int ifEndLabel = labelNum++;
        out("cmp",registers[reg].subRegs[Size::QWORD],"0");
        std::stringstream ss;
        ss << ".label_" << std::to_string(ifEndLabel);
        out("je", ss.str());

        Node* ifNode = node->nextSibling;

        if (!ifNode) {
            printf("ERROR: %s:%d:%d: missing if statement body!\n",node->token.file,node->token.line,node->token.column);
            return (Node*)-1;
        }

        if (ifNode->type != NodeType::SCOPE) {
            printf("ERROR: %s:%d:%d: expected '{', found '%s'!\n",ifNode->token.file,ifNode->token.line,ifNode->token.column,NodeTypeMap[(int)ifNode->type]);
            return (Node*)-1;
        }
        printf("label: %d\n",ifEndLabel);
        if (!createScope(ifNode, context)) return (Node*)-1;

        // preserve modified registers
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.modified && registers[reg].value.preserveModified) {
                freeReg(reg, false);
            }
        }

        Node* elseNode = node->nextSibling->nextSibling;

        unsigned int elseEndLabel;
        if (elseNode && elseNode->type == NodeType::ELSE) {
            elseEndLabel = labelNum++;
            std::stringstream ss2;
            ss2 << ".label_" << std::to_string(elseEndLabel);
            out("jmp", ss2.str());
        };
        

        std::stringstream ss2;
        ss2 << ".label_" << std::to_string(ifEndLabel) << ':';
        out(ss2.str());

        if (elseNode && elseNode->type == NodeType::ELSE) {

            for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
                freeReg(reg, false);
            }

            if (!createScope(elseNode, context)) return (Node*)-1;

            // preserve modified registers
            for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
                if (registers[reg].value.modified && registers[reg].value.preserveModified) {
                    freeReg(reg, true);
                }
            }

            std::stringstream ss2;
            ss2 << ".label_" << std::to_string(elseEndLabel) << ':';
            out(ss2.str());

            return elseNode;
        }

        return ifNode;
    }

    bool createWhile(Node* node, Context* context) {
        
        struct RegData {
            Reg reg;
            Value value;
            unsigned int position;
        };
        std::stack<RegData> pushedRegs;
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (!freeReg(reg, false)) {
                printf("saving %s\n",registers[reg].subRegs[Size::QWORD]);
                pushedRegs.push(RegData{reg,registers[reg].value,registers[reg].position});
                out("push", registers[reg].subRegs[Size::QWORD]);
                registers[reg].value = Value{};
            }
        }

        Node* whileBody = node->firstChild->nextSibling;

        if (!whileBody) {
            printf("ERROR: %s:%d:%d: missing while body!\n",node->token.file,node->token.line,node->token.column);
            return false;
        }

        if (whileBody->type != NodeType::SCOPE) {
            printf("ERROR: %s:%d:%d: expected '{', found '%s'!\n",whileBody->token.file,whileBody->token.line,whileBody->token.column,NodeTypeMap[(int)whileBody->type]);
            return false;
        }

        Context scopeContext = Context{whileBody,context,new std::unordered_map<std::string, Local*>,0,nullptr,nullptr};
        
        int parentOffset = 0;
        Context* tmpContext = context;
        while (tmpContext) {
            parentOffset += tmpContext->spaceReq;
            tmpContext = tmpContext->parent;
        }

        int spaceReq = 0;

        for (auto& pair : *whileBody->symbolMap) {
            if (pair.second->refCount) {
                spaceReq += SizeByteMap[TypeSizeMap[pair.second->t]];
                Local* l = new Local{pair.second,spaceReq+parentOffset,TypeSizeMap[pair.second->t]};
                scopeContext.locals->insert(std::make_pair(pair.first,l));
            }
        }

        if (spaceReq)
            out("sub","rsp",std::to_string(spaceReq));

        // label at the top of the while, jump here to continue
        unsigned int whileStartLabel = labelNum++;
        std::stringstream ss;
        ss << ".label_" << std::to_string(whileStartLabel);
        std::string startLabel = ss.str();
        ss << ':';
        out(ss.str());

        scopeContext.continueLabel = &startLabel;

        Reg reg = evaluate(node->firstChild, context);

        if (reg == Reg::NUL) return false;

        // jump if false to the end of the while body to exit
        unsigned int whileEndLabel = labelNum++;
        out("cmp",registers[reg].subRegs[Size::QWORD],"0");
        ss.str("");
        ss.clear();
        ss << ".label_" << std::to_string(whileEndLabel);
        out("je", ss.str());

        std::string endLabel = ss.str();

        scopeContext.breakLabel = &endLabel;

        // set any local variables to zero before the next iteration
        int offset = 0;
        for (auto& pair : *whileBody->symbolMap) {
            if (pair.second->refCount) {
                offset += SizeByteMap[TypeSizeMap[pair.second->t]];
                std::stringstream ss;
                ss << "\tmov " << SizeString[TypeSizeMap[pair.second->t]] << " [rbp-" << offset + parentOffset << "], 0";
                out(ss.str());
            }
        }

        if (!buildScope(&scopeContext)) return false;

        // preserve modified registers
        for (Reg reg = Reg::RAX; reg != Reg::RBP; reg = (Reg)(reg+1)) {
            if (registers[reg].value.modified && registers[reg].value.preserveModified) {
                freeReg(reg, true);
            }
        }

        out("jmp",startLabel);

        ss.str("");
        ss.clear();
        ss << endLabel << ':';
        out(ss.str());

        if (spaceReq) {
            out("add", "rsp",std::to_string(spaceReq));
        }

        // this must be after adding to rsp as they where pushed before
        // space for locals was allocated
        while (!pushedRegs.empty()) {
            RegData r = pushedRegs.top();
            pushedRegs.pop();
            out("pop",registers[r.reg].subRegs[Size::QWORD]);
            registers[r.reg].position = r.position;
            registers[r.reg].value = r.value;
            printf("retrieving %s\n",registers[r.reg].subRegs[Size::QWORD]);
        }
        

        return true;
    }

    bool buildScope(Context* context) {

        Node* child = context->node->firstChild;
        while (child) {
            switch (child->type) {
                case NodeType::IF:
                    child = buildIf(child, context);
                    if (child == (Node*)-1) return false;
                    break;

                case NodeType::WHILE:
                    if (!createWhile(child, context)) return false;
                    break;
                
                case NodeType::SCOPE:
                    if (!createScope(child, context)) return false;
                    break;

                case NodeType::INVOCATION:
                    if (callFunction(child, context) == Reg::RSI) return false;
                    break;

                case NodeType::OPERATION:
                    printf("operation: %s\n",child->token.value);
                    if (evaluate(child, context) == Reg::RSI) return false;
                    break;

                case NodeType::RETURN:
                    if (!createReturn(child, context)) return false;
                    break;

                case NodeType::BREAK:
                    if (!insertBreak(child, context)) return false;
                    break;

                case NodeType::CONTINUE:
                    if (!insertContinue(child, context)) return false;
                    break;

                default:
                    // TODO: think of a better word than node
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

        (*output) << "global _start\n_start:\n";

        std:: stringstream ss;

        auto key = Parser::globals.find(std::string("main"));

        if (key == Parser::globals.end()) {
            printf("ERROR: no main function!\n");
            return false;
        }

        ss << "main";
        for (Param p : *key->second->symbol->func->params) {
            ss << '_' << TypeMap[p.type];
        }

        out("call", ss.str()); // call the main function

        if (key->second->symbol->func->returnType == Type::null) {
            out("    xor rdi, rdi");
        } else out("    mov rdi, rax"); // rdi contains exit code

        out("    mov rax, 60\n    syscall\n"); // exit syscall

        for (auto& pair : *tree) {
            switch (pair.second->type) {
                case NodeType::FUNCTION:
                    if (!createFunction(pair.second)) return false;
                    break;

                case NodeType::SYMBOL:
                    globalSymbols.push_back(pair.second->symbol);
                    break;

                default:
                    // this can't happen (yet)
                    break;
            }
        }

        for (auto& pair : Parser::builtins) {
            std::stringstream ss2;
            ss2 << pair.second->name;
            for (Param p : *pair.second->func->params) {
                ss2 << '_' << TypeMap[p.type];
            }
            typedBuiltins.insert(std::make_pair(ss2.str(), pair.second));
        }

        std::unordered_map<std::string, int> externFunctions;

        bool error = false;
        std::stringstream externs;
        for (FuncCall f : undefinedFunctions) {

            if (auto key = typedBuiltins.find(f.name); key != typedBuiltins.end()) {
                if (auto key2 = externFunctions.find(f.name); key2 == externFunctions.end()) {
                    externs << "extern " << f.name << '\n';
                    externFunctions.insert(std::make_pair(f.name, 0));
                }

            } else if (auto key = typedFunctions.find(f.name); key == typedFunctions.end()) {
                printf("ERROR: %s:%d:%d: undefined function! function name: %s\n",f.node->token.file,f.node->token.line,f.node->token.column, f.name.c_str());
                error = true;
            }
        }
        out(externs.str());

        if (error) return false;

        out("\nsection .data");

        for (dataString str : strings) {
            std::stringstream ss;
            ss << str.name << ": db \"";
            char* buf = new char[str.value.len+1]{};
            int ptr = 0;
            for (int i = 0; i < str.value.len; i++, ptr++) {
                if (str.value.str[i] == '"') {
                    buf[ptr] = 0;
                    ss << buf << "\",0x22,\"";
                    ptr = -1; // ptr++ then increments it to 0 at the start of the next iteration
                } else {
                    buf[ptr] = str.value.str[i];
                }
            }

            buf[ptr] = 0;
            ss << buf << "\",0";

            out(ss.str());
        }

        return true;
    }
};
