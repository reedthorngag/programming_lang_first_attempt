#include <unordered_map>
#include <vector>

#include "../../types.hpp"

namespace Compiler {

    std::unordered_map<std::string, Node*>* getDependencies(Node* node);
}

