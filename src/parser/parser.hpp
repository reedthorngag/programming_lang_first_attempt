#include <unordered_map>
#include <vector>

#include "../lexer/lexer.hpp"

using namespace Lexer;

namespace Parser {

    enum class NodeType {
        FUNCTION,
        BLOCK,
        SYMBOL,
        LITERAL,
        VALUE,
        ASSIGNMENT,
        OPERATION,
        INVOCATION,
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
        char* value;
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
        std::unordered_map<std::string, Symbol>* symbolMap;
    };


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
    bool symbolDeclaredInScope(char* name, Node* parent, Symbol* symbol);
    bool symbolDeclaredGlobal(char* name, Symbol* symbol);


    Node* buildFunctionNode();
    Node* buildIfNode();
    Node* buildWhileNode();
    bool buildDeclerationNode(Keyword type);

    Node* assignment(Token token);
    Node* operation(Node* lvalue, Token op);

}

