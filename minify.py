#!/usr/bin/env python

import re
from collections import Counter
import string

def apply_minify_remove_rules(code):
    code = re.sub(r'(// minify-remove:start).*?(// minify-remove:end)', '', code, flags=re.DOTALL)

    lines = code.split('\n')
    res = []
    skip = False

    for x in lines:
        if x.strip() == '// minify-remove':
            skip = True
        else:
            if skip:
                skip = False
            else:
                res.append(x)

    return '\n'.join(res)

keywords = {
    "break",
    "case",
    "return",
    "const",
    "unsigned",
    "void",
    "char",
    "while",
    "struct",
    "class",
    "public",
    "int",
    "static",
    "virtual",
    "bool",
    "auto",
    "template",
    "true",
}

requires_define = {
    "CellSlice",
    "CellBuilder",
    "size_t",
    "size",
    "store_ref",
    "write",
    "extra",
    "string",
    "data",
    "HashmapAugE",
    "store_long",
    "out_msg_descr",
    "BufferSlice",
    "t_Maybe_Ref_McBlockExtra",
    "in_msg_descr",
    "fetch_ref",
    "blen",
    "CurrencyCollection",
}

cannot_be_replaced = {
    "define",
    "block",
    "compress",
    "decompress",
    "rtoken",
    "include"
}

allowed_collisions = {
    "A": ["(c>='A'&&c<", "'a'-'A':c;}"],
}

def get_all_tokens(code: str):
    matches = Counter(re.findall(r'\b[_a-zA-Z][_a-zA-Z0-9]*\b', code))
    space = [(x, (len(x) - 1) * y) for x, y in matches.items()]
    space = sorted(space, key=lambda x: -x[1])
    return [x[0] for x in space if x[0] not in cannot_be_replaced]


TOKENS_ALPHABET = string.ascii_letters


def n_to_token(n):
    if n < 0: raise Exception()
    if n < len(TOKENS_ALPHABET):
        return TOKENS_ALPHABET[n]
    
    res = []
    while n > 0:
        res.append(TOKENS_ALPHABET[n % len(TOKENS_ALPHABET)])
        n //= len(TOKENS_ALPHABET)
    
    return ''.join(reversed(res))


def replace_token(code: str, token, replacement):    
    lines = code.split("\n")
    last_idx = -1
    for idx, x in enumerate(lines):
        if x.startswith("#include"):
            last_idx = idx
    includes = '\n'.join(lines[:last_idx + 1])
    rest = '\n'.join(lines[last_idx + 1:])

    collisions = []
    allowed = allowed_collisions.get(replacement, ())
    for x in re.finditer(rf'\b{re.escape(replacement)}\b', rest):
        collision = rest[x.span()[0] - 5:x.span()[1] + 5]
        if collision not in allowed:
            collisions.append(collision)
    
    if collisions:
        print(collisions, sep='\n')
        raise Exception(f'Replacing "{token}" with "{replacement}", but "{replacement}" already exists in the code')

    rest = re.sub(rf'\b{re.escape(token)}\b', replacement, rest)

    if token in keywords or token in requires_define:
        includes += f'\n#define {replacement} {token}'
    
    return '\n'.join([includes, rest])


def replace_tokens(code, n):
    tokens = get_all_tokens(code)

    tokens_to_replace = [x for x in tokens if len(x) > 1][:n]

    def find_replacement(start):
        for _ in range(1000):
            t = n_to_token(start)
            if t not in tokens or t in allowed_collisions:
                return start
            start += 1
        raise Exception("unable to find replacement")

    last_replacement_n = -1
    for token in tokens_to_replace:
        if len(token) <= 3:
            continue
        last_replacement_n = find_replacement(last_replacement_n + 1)
        replacement = n_to_token(last_replacement_n)
        print(f'Replacing "{token}" with "{replacement}"')
        code = replace_token(code, token, replacement)
    
    return code


def minify_cpp(source_code):
    with open("zpaq_impl.cpp", 'r') as f:
        zpaq_code = f.read()
    
    # load zpaq
    source_code = source_code.replace('#include "zpaq_impl.cpp"', zpaq_code)

    # remove disabled code
    source_code = apply_minify_remove_rules(source_code)

    # Remove multiline comments (/* */)
    source_code = re.sub(r'/\*.*?\*/', '', source_code, flags=re.DOTALL)

    # Remove single-line comments (//)
    source_code = re.sub(r'//.*', '', source_code)

    # Remove leading and trailing whitespaces from each line
    lines = [line.strip() for line in source_code.splitlines()]
    lines = [x for x in lines if x]
    lines = [x if x.endswith(';') or x.endswith(',') or x.endswith("{") or x == "}" else x + '\n' for x in lines]
    lines = ['\n' + x if x.startswith('#') else x for x in lines]


    def replace_exhaust(s1, s2):
        nonlocal res
        l = len(res)
        res = res.replace(s1, s2)
        while len(res) < l:
            l = len(res)
            res = res.replace(s1, s2)

    res = ''.join(lines)
    replace_exhaust("CHECK(", "(")

    replace_exhaust("\n\n", "\n")
    replace_exhaust("  ", " ")
    replace_exhaust("# ", "#")
    replace_exhaust(", ", ",")
    replace_exhaust("if (", "if(")
    replace_exhaust("O (", "O(")
    replace_exhaust("for (", "for(")
    replace_exhaust("Q (", "Q(")
    replace_exhaust("; ", ";")
    replace_exhaust(") {", "){")
    replace_exhaust(") ", ")")
    replace_exhaust(" {", "{")
    replace_exhaust(" < ", "<")
    replace_exhaust(" + ", "+")
    replace_exhaust("template <", "template<")
    replace_exhaust(" == ", "==")
    replace_exhaust(" &", "&")
    replace_exhaust("& ", "&")
    replace_exhaust(" : ", ":")
    replace_exhaust(">\nstruct", ">struct")
    replace_exhaust("else\n{", "else{")
    replace_exhaust(")\n)", "))")
    replace_exhaust(" || ", "||")
    replace_exhaust(")\n.", ").")
    replace_exhaust(")\n&&", ")&&")
    replace_exhaust("): ", "):")
    replace_exhaust("( ", "(")
    replace_exhaust(" )", ")")
    replace_exhaust("}\n}", "}}")
    replace_exhaust("{ ", "{")
    replace_exhaust("} ", "}")
    replace_exhaust("\n{", "{")
    replace_exhaust("private:", "")
    replace_exhaust("public:\n", "public:")
    replace_exhaust("override", "")
    replace_exhaust("#include ", "#include")

    replace_exhaust(" ^= ", "^=")
    replace_exhaust(" <= ", "<=")
    replace_exhaust("return (", "return(")
    replace_exhaust("while (", "while(")
    replace_exhaust("; break", ";break")
    replace_exhaust(" :\n", ":")
    replace_exhaust(": ", ":")
    replace_exhaust(" += ", "+=")
    replace_exhaust(" -= ", "-=")
    replace_exhaust("] > ", "]>")

    replace_exhaust("{}\n", "{}")
    replace_exhaust(";(\n", ";(")
    replace_exhaust("}(\n", "}(")
    replace_exhaust("\n\n", "\n")

    res = re.sub(r'\bE\([^)]*\);', 'E(0);', res)
    res = re.sub(r'(?<!\*)(\s+\=\s+)', '=', res)

    res = replace_tokens(res, 130)

    return res

# Example usage
if __name__ == "__main__":
    input_file = "solution.cpp"
    output_file = "solution.min.cpp"

    # Read the C++ source code from a file
    with open(input_file, 'r') as file:
        cpp_code = file.read()

    # Minify the C++ source code
    minified_code = minify_cpp(cpp_code)

    # Save the minified code to a new file
    with open(output_file, 'w') as file:
        file.write(minified_code)

    print(f"Minified code saved to {output_file}")
