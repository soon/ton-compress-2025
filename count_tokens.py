#!/usr/bin/env python

from collections import Counter
import re


def count_tokens(source_code):
    matches = Counter(re.findall(r'\b[_a-zA-Z][_a-zA-Z0-9]*\b', source_code))

    space = [(x, (len(x) - 1) * y) for x, y in matches.items() if len(x) > 1]
    space = sorted(space, key=lambda x: -x[1])
    cut = 30
    print(sum(x[1] for x in space[:cut]))
    print(*space[:cut], sep='\n')


# Example usage
if __name__ == "__main__":
    input_file = "solution.min.cpp"

    # Read the C++ source code from a file
    with open(input_file, 'r') as file:
        cpp_code = file.read()

    count_tokens(cpp_code)
