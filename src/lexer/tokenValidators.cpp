#include "lexer.hpp"
#include "util.hpp"

namespace Lexer {

    const char* minMaxLiteralStr(char* str) {

        int len = strlen(str);

        if (len < 6) return nullptr;

        int i = 2;
        while (str[i] != '_' && ++i < len);

        if (i == len) return nullptr;

        char* type = newString(str, i);

        if (auto key = numberTypes.find(type); key != numberTypes.end()) {

            char* minMaxStr = newString(str+i+1,len-(i+1));

            if (auto key2 = literalTypesMap.find(minMaxStr); key2 != literalTypesMap.end()) {

                return (key2->second ? key->second.maxStr : key->second.minStr);
            }

        }

        return nullptr;

    }

    bool validateSymbol() {

        char* ptr = Lexer::ptr;
        int len;

        do {
            if (!symbolChar(ptr[len], len)) {
                if (!len) return false;

                if (!breakChar(ptr[len])) return false;

                char* str = newString(ptr, len);

                char* minMaxStr = (char*)minMaxLiteralStr(str);

                if (auto key = builtinLiteralTypes.find(str); key != builtinLiteralTypes.end())
                    tokens->push_back(Token{TokenType::LITERAL,{.value={str}},file,false});

                else if (minMaxStr)
                    tokens->push_back(Token{TokenType::LITERAL,{.value={(char*)str}},file,false});
                
                else if (auto key = keywordMap.find(str); key != keywordMap.end())
                    tokens->push_back(Token{TokenType::KEYWORD,{.keyword={key->second}},file,false});
                
                else
                    tokens->push_back(Token{TokenType::SYMBOL,{.value={str}},file,false});
                

                Lexer::ptr += len;
                file.column += len;
            }
        } while (++len <= MAX_SYMBOL_LEN);

        return false;
    }

    
}