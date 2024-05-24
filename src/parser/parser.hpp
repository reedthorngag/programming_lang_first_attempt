#include <unordered_map>
#include <vector>

#include "../lexer/lexer.hpp"

using namespace Lexer;

namespace Parser {

    enum class NodeType {
        FUNCTION,
        BLOCK,
        DECLERATION,
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
        string
    };

    // think of a better name
    struct Symbol {
        SymbolType type;
        char* value;
        Type t;
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
        Node* firatChild;
        Node* nextSibling;
        union {
            Symbol symbol;
            Literal literal;
            Operator op;
            std::unordered_map<std::string, Symbol>* symbolMap;
        };
    };

    std::unordered_map<std::string, Node*>* parseTokens(std::vector<Token>* tokens);
}


