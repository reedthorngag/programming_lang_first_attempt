#include "types.hpp"

const char* TokenTypeMap[]{
    "';'", // ENDLINE
    "','", // COMMA
    "'{'", // SCOPE_START
    "'}'", // SCOPE_END
    "'('", // GROUPING_START
    "')'", // GROUPING_END
    "keyword", // KEYWORD
    "symbol", // SYMBOL
    "type", // TYPE
    "operator", // OPERATOR
    "literal", // LITERAL
    "EOF", // FILE_END
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

const char* NodeTypeMap[]{
    "FUNCTION",
    "SCOPE",
    "SYMBOL",
    "LITERAL",
    "OPERATION",
    "INVOCATION",
    "IF",
    "ELSE",
    "RETURN",
    "WHILE",
    "BREAK",
    "CONTINUE"
};

const char* TypeMap[] {
    "error",
    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64",
    "f16",
    "f32",
    "f64",
    "chr",
    "str",
    "bool",
    "null"
};

const char* OpTypeMap[]{
    "ASSIGNMENT",
    "SINGLE_OP_PREFIX",
    "SINGLE_OP_POSTFIX",
    "MATH",
    "CAST"
};

const char* SizeTypeMap[]{
    "byte",
    "word",
    "dword",
    "qword"
};

std::unordered_map<std::string, Type> typeMap = {
    {"null",Type::null},

    {"i8",Type::i8},
    {"i16",Type::i16},
    {"i32",Type::i32},
    {"i64",Type::i64},

    {"u8",Type::u8},
    {"u16",Type::u16},
    {"u32",Type::u32},
    {"u64",Type::u64},

    {"f16",Type::f16},
    {"f32",Type::f32},
    {"f64",Type::f64},

    {"char",Type::chr},
    {"string",Type::string},
    {"null",Type::null},
    {"bool",Type::boolean}
};

