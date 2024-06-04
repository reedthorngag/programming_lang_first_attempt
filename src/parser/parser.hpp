#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../lexer/lexer.hpp"

#ifndef _PARSER
#define _PARSER

using namespace Lexer;

namespace Parser {

    enum class NodeType {
        FUNCTION,
        BLOCK,
        SYMBOL,
        LITERAL,
        OPERATION,
        INVOCATION,
    };

    extern const char* NodeTypeMap[];

    extern const char* TypeMap[];

    enum Type {
        error, // used to indicate there is a type error, not used as a type
        i8,
        i16,
        i32,
        i64,
        i128,

        u8,
        u16,
        u32,
        u64,
        u128,

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
        0b100000000,
        0b1000000000,

        0b10000000000,
        0b100000000000,
        0b1000000000000,

        0,
        0,
        0,
        0,
    };

    const long long TypeCompatibility[] {
        0,
        TypeMask[i8],
        TypeMask[i16] | TypeMask[i8],
        TypeMask[i32] | TypeMask[i16] | TypeMask[i8],
        TypeMask[i64] | TypeMask[i32] | TypeMask[i16] | TypeMask[i8],
        TypeMask[i128] | TypeMask[i64] | TypeMask[i32] | TypeMask[i16] | TypeMask[i8],
        TypeMask[u8],
        TypeMask[u16] | TypeMask[u8],
        TypeMask[u32] | TypeMask[u16] | TypeMask[u8],
        TypeMask[u64] | TypeMask[u32] | TypeMask[u16] | TypeMask[u8],
        TypeMask[u128] | TypeMask[u64] | TypeMask[u32] | TypeMask[u16] | TypeMask[u8],
        TypeMask[f16],
        TypeMask[f32] | TypeMask[f16],
        TypeMask[f64] | TypeMask[f32] | TypeMask[f16],
        TypeMask[chr],
        TypeMask[string],
        TypeMask[boolean],
        TypeMask[null],
    };

    extern std::unordered_map<std::string, Type> typeMap;

    extern std::unordered_map<std::string, bool> assignmentOps;
    extern std::unordered_map<std::string, int> mathmaticalOps; // boolean and bitwise too

    struct Param {
        char* name;
        Type type;
    };

    struct Function {
        std::vector<Param> params;
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
        SINGLEOPERAND,
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
            Symbol symbol;
            Literal literal;
            Operator op;
        };
        Token token;
        std::unordered_map<std::string, Symbol>* symbolMap;
    };

    extern std::unordered_map<std::string, Symbol> builtins;

    extern const char* TokenTypeMap[];

    extern std::unordered_map<std::string, Node*> globals;

    extern std::vector<Node*> unresolvedReferences;

    extern Node* parent;
    extern int depth;

    extern std::vector<Token>* tokens;
    extern long long unsigned int index;


    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* tokens);


    void appendChild(Node* parent, Node* child);

    bool symbolDeclared(char* name, Node* parent, Symbol* symbol);
    inline bool symbolBuiltin(char* name, Symbol* symbol);
    bool symbolDeclaredInScope(char* name, Node* parent, Symbol* symbol);
    bool symbolDeclaredGlobal(char* name, Symbol* symbol);


    Node* buildFunctionNode();
    Node* buildIfNode();
    Node* buildWhileNode();
    bool buildDeclerationNode(Keyword type);

    Node* assignment(Token token);
    Node* operation(Node* lvalue, Token op);

};

#endif