#include <cstring>

#include "lexer.hpp"

namespace Lexer {

    bool log = true;

    std::unordered_map<std::string,Keyword> keywordMap = {
        {
            "func",Keyword::FUNC
        },
        {
            "var",Keyword::VAR
        },
        {
            "const",Keyword::CONST
        },
        {
            "global", Keyword::GLOBAL
        },
        {
            "if",Keyword::IF
        },
        {
            "else",Keyword::ELSE
        },
        {
            "while",Keyword::WHILE
        },
        {
            "return", Keyword::RETURN
        },
        {
            "break", Keyword::BREAK
        },
        {
            "continue", Keyword::CONTINUE
        }
        
    };

    const char* KeywordTypeMap[]{
        "FUNC",
        "VAR",
        "CONST",
        "GLOBAL",
        "IF",
        "ELSE",
        "WHILE",
        "RETURN",
        "BREAK",
        "CONTINUE"
    };

    std::unordered_map<std::string,bool> operations = {
        {"+",true},
        {"-",true},
        {"*",true},
        {"/",true},
        {"=",true},
        {"+=",true},
        {"-=",true},
        {"/=",true},
        {"*=",true},
        {"%=",true},
        {"<<=",true},
        {">>=",true},
        {"^=",true},
        {"&=",true},
        {"|=",true},
        {"%",true},
        {"&",true},
        {"!",true},
        {"~",true},
        {"|",true},
        {"^",true},
        {"&",true},
        {"||",true},
        {"&&",true},
        {"==",true},
        {"!=",true},
        {"<=",true},
        {">=",true},
        {">",true},
        {"<",true},
        {"++",true},
        {"--",true},
        {"<<",true},
        {"<<<",true},
        {">>",true},
        {">>>",true},
    };

    const char* maxStr[] {
        "127", "32_767", "2_147_483_647", "9_223_372_036_854_775_807",
        "255", "65_535","4_294_967_295", "18_446_744_073_709_551_615",
        "0","0","0"
    };

    const char* minStr[] {
        "-127", "-32_768", "-2_147_483_648", "-9_223_372_036_854_775_808",
        "0", "0", "0", "0",
        "0","0","0"
    };

    std::unordered_map<std::string, Number> numberTypes = {
        {"i8",Number{true,minStr[NumberType::i8],maxStr[NumberType::i8]}},
        {"i16",Number{true,minStr[NumberType::i16],maxStr[NumberType::i16]}},
        {"i32",Number{true,minStr[NumberType::i32],maxStr[NumberType::i32]}},
        {"i64",Number{true,minStr[NumberType::i64],maxStr[NumberType::i64]}},

        {"u8",Number{true,minStr[NumberType::u8],maxStr[NumberType::u8]}},
        {"u16",Number{true,minStr[NumberType::u16],maxStr[NumberType::u16]}},
        {"u32",Number{true,minStr[NumberType::u32],maxStr[NumberType::u32]}},
        {"u64",Number{true,minStr[NumberType::u64],maxStr[NumberType::u64]}},

        {"f16",Number{false}},
        {"f32",Number{false}},
        {"f64",Number{false}},
    };

    std::unordered_map<std::string, int> literalTypesMap = {
        {"MIN",0},
        {"MAX",1}
    };

    std::unordered_map<std::string, std::string> builtinLiteralTypes = {
        {"true","1"},
        {"false","0"}
    };


    char* ptr = nullptr;

    std::vector<Token>* tokens;

    File file;

    std::vector<Token>* parse(char* fileName, char* input) {

        tokens = new std::vector<Token>;
        ptr = input;
        
        file = File{fileName, 1, 1};

        while (*ptr) {
            if (log && *ptr != ' ') printf("At %s:%d:%d: character: '%c' 0x%x\n",file.name,file.line,file.col,((*ptr >= 20) ? *ptr : '?'),*ptr);
            switch (*ptr) {

                case ' ':
                case '\t':
                case '\r':
                    break;

                case '\n':
                    file.line++;
                    file.col = 0;
                    break;

                case ',':
                    tokens->push_back(Token{TokenType::COMMA,{.value = {nullptr}},file});
                    break;

                case ';':
                    tokens->push_back(Token{TokenType::ENDLINE,{.value = {nullptr}},file});
                    break;
                
                case '(':
                    tokens->push_back(Token{TokenType::GROUPING_START,{.value = {nullptr}},file});
                    break;
                case ')':
                    tokens->push_back(Token{TokenType::GROUPING_END,{.value = {nullptr}},file});
                    break;

                case '{':
                    tokens->push_back(Token{TokenType::SCOPE_START,{.value = {nullptr}},file});
                    break;
                case '}':
                    tokens->push_back(Token{TokenType::SCOPE_END,{.value = {nullptr}},file});
                    break;

                case '[':
                    tokens->push_back(Token{TokenType::ARRAY_START,{.value = {nullptr}},file});
                    break;
                case ']':
                    tokens->push_back(Token{TokenType::ARRAY_END,{.value = {nullptr}},file});
                    break;

                case '"':
                    if (!parseStr()) goto SyntaxError;
                    break;

                case '\'':
                    if (!parseChr()) goto SyntaxError;
                    break;

                case '/':
                    if (!(parseComment() || parseOperator())) goto SyntaxError;
                    break;
                
                case ':':
                    while (*++ptr==' ' || *ptr=='\n' || *ptr=='\t') file.col++;
                    file.col++;
                    if (!parseType()) goto SyntaxError;
                    break;

                default:
                    printf("\n");
                    if (!(
                        parseSymbol() ||
                        parseOperator() ||
                        parseLiteral()
                    )) {
SyntaxError:
                        printf("\rERROR: %s:%d:%d: syntax error, unparsable token!\n",file.name,file.line,file.col);
                        return nullptr;
                    }
            }

            // EOF
            if (!*ptr) break;

            ptr++;
            file.col++;
        }

        tokens->push_back(Token{TokenType::FILE_END,{.value = {nullptr}},file});
        return tokens;
    }

}