#include <unordered_map>

#include "../parser/parser.hpp"

namespace Compiler {

    struct Local {

    };

    struct Context {
        Parser::Node* node;
        std::unordered_map<char*, Local> locals;
        
    };

    bool compile(std::unordered_map<std::string, Parser::Node*>* tree, std::ofstream* out);
}


