#!/usr/bin/env python

import re


def minify_cpp(source_code):
    """
    Minify a given C++ source code by removing comments, unnecessary whitespaces, and blank lines, 
    while keeping each #include and #define directive on a separate line, even if written with spaces.

    Parameters:
        source_code (str): The C++ source code as a string.

    Returns:
        str: Minified C++ source code.
    """
    # Remove multiline comments (/* */)
    source_code = re.sub(r'/\*.*?\*/', '', source_code, flags=re.DOTALL)

    # Remove single-line comments (//)
    source_code = re.sub(r'//.*', '', source_code)

    # Remove leading and trailing whitespaces from each line
    lines = [line.strip() for line in source_code.splitlines()]
    lines = [x for x in lines if x]
    lines = [x if x.endswith(';') or x.endswith(',') or x.endswith("{") or x == "}" else x + '\n' for x in lines]
    lines = ['\n' + x if x.startswith('#') else x for x in lines]

    res = ''.join(lines)
    res = res.replace("\n\n", "\n")
    res = res.replace("     ", " ")
    res = res.replace("    ", " ")
    res = res.replace("   ", " ")
    res = res.replace("  ", " ")
    res = res.replace("# ", "#")
    res = res.replace(", ", ",")
    res = res.replace("if (", "if(")
    res = res.replace("O (", "O(")
    res = res.replace("for (", "for(")
    res = res.replace("Q (", "Q(")
    res = res.replace("; ", ";")
    res = res.replace(") {", "){")
    res = res.replace(") ", ")")
    res = res.replace(" {", "{")
    res = res.replace(" < ", "<")

    res = re.sub(r'(?<!S)E\([^)]*\);', 'E(0);', res)
    res = re.sub(r'(?<!\*)(\s+\=\s+)', '=', res)

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
