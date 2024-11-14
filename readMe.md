
# Alk programming language

This is currently in development and nowhere near being practical for use

language goals:
  * have my own language that I can adjust to how I like
  * fast compilation times
  * let you do pretty much whatever you want (while not giving you *too* many foot guns)
  * easily integrable with system libraries and stuff
  * clear and accurate error messages


current language vision:
  * typescript crossed with c++



```
keywords:
    func var const struct enum
    global  // this explicitly specifies the global scope for name overlaps in local scope (default is local scope)
    if else elif switch
    while for do?
    alloc free
    return continue break

operators:
    mathematical: + - / * % ++ --
    bitwise: & ^ | ~ << >>
    boolean: ! || && == != < > <= >=
    assignment: =
    conditional: ?: (maybe add || and && like js?)
combinations:
    += -= /= *= %=
    &= ^= |= <<= >>=

built in constants:
    type_MAX
    type_MIN
    true false (1 and 0 respectively)
e.g. i32_MAX

pointers and fields:
    create a reference: &variable
    dereference a pointer: *variable
    access a field: .
    access a pointers field: -> (or maybe just use . ?)

built in data types:
    i8, i16, i32, i64, i128
    u8, u16, u32, u64, u128
    f16, f32, f64
    char
    string - implemented as u8[] with null terminator
    bool
    function: ret_type(param_type,param_type)

valid number literals:
    10_000
    0xff_ff
    0o23_54
    0b1111_0000
    100:u8
    -100:i32
    0.542:f32

symbol name rules: (variable, function and type names)
    must start with upper or lower case character or underscore
    can contain uppercase, lowercase, numbers and underscores
    maximum length 64 characters (more than enough for everyone except java devs)


operator precedence: (from least to most)
    associate right to left:
        =           1
        += -=       4
        /= *= %=    4
        <<= >>=     4
        &= ^= |=    4
        ?:          5
    left to right:
        ||          6
        &&          7
        == !=       8
        < > <= >=   9
        |           10
        ^           11
        &           12
        << >> >>>   13
        + -         14
        * / %       15
        - ! ~       16
        var++ var-- 17 (effectively lower than pre-increment/dereference/cast because it always comes after)
        ++var --var 17
        (type) *var 17
        func()      18
        ( ) . ->    19

valid string literals:
    "string"
    "str\"ing"
    "stri
ng"
    "stri\
    ng" backslash means ignore any spaces, tabs or newlines that immediately follow


commenting:
    // for single line
    /* */ for partial line or multi line
    // */ is valid for partial line

valid line starts:
    declaration
    assignment
    function call


```