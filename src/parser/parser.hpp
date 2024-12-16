#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../lexer/lexer.hpp"
#include "../compiler/compiler.hpp"
#include "../types.hpp"
#include "../util/debugging.hpp"

#ifndef _PARSER
#define _PARSER

using namespace Lexer;

namespace Parser {

    extern bool log;

    const long long TypeMask[] {
        0,
        0b1,
        0b10,
        0b100,
        0b1000,

        0b10000,
        0b100000,
        0b1000000,
        0b10000000,

        0b1000000000,
        0b10000000000,
        0b100000000000,

        0,
        0b1000000000000,
        0,
        0,
    };

    const long long TypeCompatibility[] {
        0,
        TypeMask[i8],
        TypeMask[i16] | TypeMask[i8] | TypeMask[u8],
        TypeMask[i32] | TypeMask[i16] | TypeMask[i8] | TypeMask[u8] | TypeMask[u16],
        TypeMask[i64] | TypeMask[i32] | TypeMask[i16] | TypeMask[i8] | TypeMask[u8] | TypeMask[u16] | TypeMask[u32],
        TypeMask[u8],
        TypeMask[u16] | TypeMask[u8],
        TypeMask[u32] | TypeMask[u16] | TypeMask[u8],
        TypeMask[u64] | TypeMask[u32] | TypeMask[u16] | TypeMask[u8] | TypeMask[string],
        TypeMask[f16],
        TypeMask[f32] | TypeMask[f16],
        TypeMask[f64] | TypeMask[f32] | TypeMask[f16],
        TypeMask[chr],
        TypeMask[string] | TypeMask[u64],
        TypeMask[boolean],
        TypeMask[null],
    };

    const uint64_t NumTypeMax[] {
        127, 32767, 2147483647, 9223372036854775807,
        255, 65535,4294967295, 18446744073709551615LLU
    };

    const int64_t NumTypeMin[] {
        -127, -32768, -2147483648, -9223372036854775807LL,
        0, 0, 0, 0, 0
    };

    extern std::unordered_map<std::string, bool> assignmentOps;
    extern std::unordered_map<std::string, int> mathmaticalOps; // boolean and bitwise too

    extern std::unordered_map<std::string, Symbol*> builtins;

    extern std::unordered_map<std::string, Node*>* globals;

    extern std::vector<Node*> unresolvedReferences;

    extern Node* parent;
    extern int depth;

    extern std::vector<Token>* tokens;
    extern long long unsigned int index;

    enum Order {
        LtoR,
        RtoL
    };

    struct Precedence {
        Order evalOrder;
        int precedence;
    };

    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* tokens);


    void appendChild(Node* parent, Node* child);

    bool symbolDeclared(char* name, Node* parent, Symbol** symbol);
    bool symbolBuiltin(char* name, Symbol** symbol);
    bool symbolDeclaredInScope(char* name, Node* parent, Symbol** symbol);
    bool symbolDeclaredGlobal(char* name, Symbol** symbol);

    void generateParamMapping(Node* node);

    Node* evaluateValue(Token token);
    Node* functionCall(Symbol* symbol);
    Node* processCast();

    Node* buildFunctionNode();
    Node* buildIfNode();
    Node* buildElseNode();
    Node* buildWhileNode();
    bool processReturn();
    bool buildDeclarationNode(Keyword type);

    Node* assignment(Token token);
    Node* processKeyword(Token token);
    Node* processSymbol(Token token);
    Node* processGrouping();
    Node* processPrefixOperator(Token token);

    extern OpType getOpType(char* op);
    Precedence getPrecedence(Token token);
    Node* operation(Node* lvalue, Token op);

};

#endif