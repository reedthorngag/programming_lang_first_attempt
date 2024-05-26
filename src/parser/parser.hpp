#include <unordered_map>
#include <vector>

#include "../lexer/lexer.hpp"

using namespace Lexer;

namespace Parser {

    enum class NodeType {
        FUNCTION,
        BLOCK,
        LITERAL,
        ASSIGNMENT,
        OPERATION,
        INVOCATION,
    };

    // think of a more accurate name
    enum class SymbolType {
        FUNC,
        VAR,
        CONST,
    };

    enum Type {
        i8, i16, i32, i64, i128,
        u8, u16, u32, u64, u128,
        f16, f32, f64,
        c, // fucking c++ wont let me use 'char' and it seems there is no way to fix 
           // it ffs, *this* is the sort of thing that makes me want to write my own language
        string,
        null
    };

    extern std::unordered_map<std::string, Type> typeMap;

    extern std::unordered_map<std::string, bool> assignmentOps;
    extern std::unordered_map<std::string, int> mathmaticalOps; // boolean and bitwise too

    struct Param {
        char* name;
        Type type;
    };

    struct Function {
        char* name;
        std::vector<Param> params;
        Type returnType;
    };

    // think of a better name
    struct Symbol {
        SymbolType type;
        char* value;
        union {
            Type t;
            Function* func;
        };
    };

    struct Literal {
        char* value;
    };

    struct Operator {
        char* value;
    };

    struct Node {
        NodeType type;
        Node* parent;
        Node* firstChild;
        Node* nextSibling;
        union {
            Function* function;
            Symbol symbol;
            Literal literal;
            Operator op;
        };
        std::unordered_map<std::string, Symbol>* symbolMap;
    };

    enum LvalueType {
        OPERATION,
        SYMBOL,
        LITERAL
    };

    struct Lvalue {
        LvalueType type;
        union {
            Node* node;
            Symbol* symbol;
        };
    };

    extern const char* TokenTypeMap[];

    extern std::unordered_map<std::string, Symbol> globals;

    extern std::vector<Node*> unresolvedReferences;

    extern Node* parent;
    extern int depth;

    extern std::vector<Token>* tokens;
    extern long long unsigned int index;


    std::unordered_map<std::string, Symbol>* parseTokens(std::vector<Token>* tokens);


    void appendChild(Node* parent, Node* child);

    bool symbolDeclared(char* name, Node* parent, Symbol* symbol);
    bool symbolDeclaredInScope(char* name, Node* parent, Symbol* symbol);
    bool symbolDeclaredGlobal(char* name, Symbol* symbol);


    Node* buildFunctionNode();
    Node* buildIfNode();
    Node* buildWhileNode();
    bool buildDeclerationNode(Keyword type);

    bool assignment(Token token);
    Node* operation(Lvalue lvalue, Token op);

}

