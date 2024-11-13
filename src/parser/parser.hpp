#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../lexer/lexer.hpp"
#include "../compiler/compiler.hpp"

#ifndef _PARSER
#define _PARSER

using namespace Lexer;

namespace Parser {

    enum class NodeType {
        FUNCTION,
        SCOPE,
        SYMBOL,
        LITERAL,
        OPERATION,
        INVOCATION,
        IF,
        ELSE,
        RETURN,
        WHILE
    };

    extern const char* NodeTypeMap[];

    extern const char* TypeMap[];

    extern const char* OpTypeMap[];

    extern const char* SizeTypeMap[];

    enum Type {
        error, // used to indicate there is a type error, not used as a type
        i8,
        i16,
        i32,
        i64,

        u8,
        u16,
        u32,
        u64,

        f16,
        f32,
        f64,
        chr, // c++ wont let me use 'char' and it seems there is no way to fix 
           // it smh, *this* is the sort of thing that makes me want to write my own language
        string,
        boolean,
        null
    };

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

    struct Number {
        const Type type;
        const int64_t min;
        const uint64_t max;
    };

    const Number TypeConstraints[] {
        Number{},
        Number{i8,NumTypeMin[NumberType::i8],NumTypeMax[NumberType::i8]},
        Number{i16,NumTypeMin[NumberType::i16],NumTypeMax[NumberType::i16]},
        Number{i32,NumTypeMin[NumberType::i32],NumTypeMax[NumberType::i32]},
        Number{i64,NumTypeMin[NumberType::i64],NumTypeMax[NumberType::i64]},

        Number{u8,NumTypeMin[NumberType::u8],NumTypeMax[NumberType::u8]},
        Number{u16,NumTypeMin[NumberType::u16],NumTypeMax[NumberType::u16]},
        Number{u32,NumTypeMin[NumberType::u32],NumTypeMax[NumberType::u32]},
        Number{u64,NumTypeMin[NumberType::u64],NumTypeMax[NumberType::u64]},
    };

    extern std::unordered_map<std::string, Type> typeMap;

    extern std::unordered_map<std::string, bool> assignmentOps;
    extern std::unordered_map<std::string, int> mathmaticalOps; // boolean and bitwise too

    enum Reg {
        NUL,
        RAX,
        RBX,
        RCX,
        RDX,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15,
        RSI,
        RDI,
        RBP,
        RSP,
        STACK // simplifies stuff for what reg params use
    };

    struct Param {
        char* name;
        Type type;
        Reg reg;
    };

    struct Function {
        std::vector<Param>* params;
        Type returnType;
    };

    // think of a more accurate name
    enum class SymbolType {
        FUNC,
        VAR,
        CONST,
    };

    // think of a better name
    struct Symbol {
        SymbolType type;
        char* name;
        union {
            Type t;
            Function* func;
        };
        int refCount;
        Reg location;
    };

    struct Literal {
        Type type;
        union {
            char* value;
            struct {
                char* str;
                int len;
            } str;
            char chr;
            uint64_t u;
            int64_t i;
        };
    };

    enum OpType {
        ASSIGNMENT,
        SINGLE_OP_PREFIX,
        SINGLE_OP_POSTFIX,
        MATH
    };

    struct Operator {
        char* value;
        OpType type;
    };

    struct Node {
        NodeType type;
        Node* parent;
        Node* firstChild;
        Node* nextSibling;
        union {
            Symbol* symbol;
            Literal literal;
            Operator op;
        };
        Token token;
        std::unordered_map<std::string, Symbol*>* symbolMap;
    };

    extern std::unordered_map<std::string, Symbol*> builtins;

    extern const char* TokenTypeMap[];

    extern std::unordered_map<std::string, Node*> globals;

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

    Node* buildFunctionNode();
    Node* buildIfNode();
    Node* buildElseNode();
    Node* buildWhileNode();
    bool processReturn();
    bool buildDeclarationNode(Keyword type);

    Node* assignment(Token token);
    Node* processGrouping();
    Precedence getPrecedence(Token token);
    Node* operation(Node* lvalue, Token op);

};

#endif