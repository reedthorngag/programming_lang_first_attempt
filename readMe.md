
language goals:
>   1. have my own language that I can adjust to how I like
>   2. fast compilation times
>   3. let you do pretty much whatever the fuck you want (while not giving you *too* many foot guns)
>   4. easily integratable with system libraries and stuff


current language vision:
    typescript crossed with c++


keywords:
    func var const struct enum
    global  // this explicitly specifies the global scope for name overlaps in local scope
    if else elif switch
    while for do?
    alloc free
    return continue break

operators:
    mathmatical: + - / * % ++ --
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
    acess a pointers field: -> (or maybe just use .?)

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
        = 
        += -= 
        /= *= %=
        <<= >>= 
        &= ^= |=
        ?:
    left to right:
        ||
        &&
        == !=
        < > <= >=
        |
        ^
        &
        << >> >>>
        + -
        * / %
        - ! ~
        var++ var--
        (type) *var ++var --var  (so you can do ++(type)var or (type)++var, may change this is future though)
        func()
        ( ) . ->

valid string literals:
    "string"
    "str\"ing"
    "stri
ng"
    "stri\
    ng" backslash means ignore any spaces, tabs or newlines that immediatly follow


commenting:
    // for single line
    /* */ for partial line or multi line
    // */ is valid for partial line

valid line starts:
    decleration
    assignment
    function call

changes that indicate new token:
    symbol name char -> anything else
    operator char -> anything else

