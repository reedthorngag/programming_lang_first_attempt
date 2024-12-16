#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>

#ifndef _TYPES
#define _TYPES

enum Keyword {
    FUNC,
    VAR,
    CONST,
    GLOBAL,
    IF,
    ELSE,
    WHILE,
    RETURN,
    BREAK,
    CONTINUE
};

enum TokenType {
    ENDLINE,
    COMMA,
    SCOPE_START,
    SCOPE_END,
    GROUPING_START,
    GROUPING_END,
    KEYWORD,
    SYMBOL,
    TYPE,
    OPERATOR,
    LITERAL,
    FILE_END,
    ARRAY_START,
    ARRAY_END
};

struct File {
    char* name;
    int line;
    int col;
};

struct Token {
    TokenType type;
    union {
        char* value;
        struct {
            char* str;
            unsigned char base;
            bool floatingPoint;
            bool exponent;
            bool negativeExponent;
            bool negative;
        } literal;
        Keyword keyword;
    };
    File file;
};

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
    WHILE,
    BREAK,
    CONTINUE
};

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

enum Reg {
    NUL, // so functions that generally return a value in a register can return nothing
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
    XMM0,
    XMM1,
    XMM2,
    XMM3,
    XMM4,
    XMM5,
    XMM6,
    XMM7,
    XMM8,
    XMM9,
    XMM10,
    XMM11,
    XMM12,
    XMM13,
    XMM14,
    XMM15,
    STACK, // simplifies stuff for what reg params use
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
    char* value;
    bool negative;
    union {
        struct {
            char* str;
            int len;
        } str;
        char chr;
        uint64_t _uint;
        int64_t _int;
    };
};

enum OpType {
    ASSIGNMENT,
    SINGLE_OP_PREFIX,
    SINGLE_OP_POSTFIX,
    MATH,
    CAST
};

struct Operator {
    OpType type;
    char* value;
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

extern const char* NodeTypeMap[];

extern const char* TypeMap[];

extern const char* OpTypeMap[];

extern const char* SizeTypeMap[];

extern const char* KeywordTypeMap[];

extern const char* TokenTypeMap[];

extern std::unordered_map<std::string, Type> typeMap;

#endif




