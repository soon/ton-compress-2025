#!/usr/bin/env python

from collections import Counter
import re


cannot_be_replaced = {
    "define",
    "block",
    "compress",
    "decompress",
    "rtoken",
    "include",
    "misc",
    "stddef",
    "errno",
    "short",
    "stdint",
    "int64_t",
    "uint64_t",
    "offsetof",
    "malloc",
    "free",
    "uintptr_t",
    "USE_posix_memalign",  # todo check later
    "_Dec",
    "_Enc",
    "utils",
    "sz",
    "namespace",
    "long",
    "_WIN32",
    "Read",
    *(
        x[0]
        for x in [  # todo check later
            ("_M_X64", 1),
            ("_M_AMD64", 1),
            ("__x86_64__", 1),
            ("__AMD64__", 1),
            ("__amd64__", 1),
            ("_M_IX86", 1),
            ("__i386__", 1),
            ("_M_ARM64", 1),
            ("__aarch64__", 1),
            ("_M_ARM", 1),
            ("_M_ARM_NT", 1),
            ("__arm__", 1),
            ("MY_CPU_ARMT", 1),
            ("MY_CPU_ARM32", 1),
            ("__ia64__", 1),
            ("MY_CPU_IA64", 1),
            ("__mips64", 1),
            ("__mips64__", 1),
            ("__mips__", 1),
            ("__ppc__", 1),
            ("__powerpc__", 1),
            ("__PPC__", 1),
            ("_POWER", 1),
            ("__64BIT__", 1),
            ("__sparc__", 1),
            ("__sparc", 1),
            ("MY_CPU_SPARC", 1),
            ("__sparc_v9__", 1),
            ("__sparcv9", 1),
            ("__sparc_v8__", 1),
            ("__sparcv8", 1),
            ("__riscv", 1),
            ("__riscv__", 1),
            ("__loongarch__", 1),
            ("_LITTLE_ENDIAN", 1),
            ("__LITTLE_ENDIAN__", 1),
            ("__MIPSEL__", 1),
            ("__MIPSEL", 1),
            ("_MIPSEL", 1),
            ("__BFIN__", 1),
            ("__ORDER_LITTLE_ENDIAN__", 1),
            ("__BIG_ENDIAN__", 1),
            ("__MIPSEB__", 1),
            ("__MIPSEB", 1),
            ("_MIPSEB", 1),
            ("__m68k__", 1),
            ("__s390__", 1),
            ("__s390x__", 1),
            ("__zarch__", 1),
            ("__ORDER_BIG_ENDIAN__", 1),
            ("__xlC__", 1),
            ("__riscv_misaligned_fast", 1),
            ("__ARM_FEATURE_UNALIGNED", 1),
            ("__clang_minor__", 1),
            ("__clang_patchlevel__", 1),
            ("__apple_build_version__", 1),
            ("__GNUC_MINOR__", 1),
            ("__GNUC_PATCHLEVEL__", 1),
            ("__MINGW32__", 1),
            ("__MINGW64__", 1),
            ("__MCST__", 1),
            ("__LCC_MINOR__", 1),
            ("unroll", 1),
            ("vectorize", 1),
            ("no_vector", 1),
            ("string", 1),
            ("memmove", 1),
            ("_pad2_", 1),
            ("arm64_neon", 1),
            ("arm_neon", 1),
            ("uint32x4_t", 1),
            ("vsubq_u32", 1),
            ("vmaxq_u32", 1),
            ("smmintrin", 1),
            ("__m128i", 1),
            ("_mm_sub_epi32", 1),
            ("_mm_max_epu32", 1),
            ("vdupq_n_u32", 1),
            ("_mm_set_epi32", 1),
            ("Ptr", 1),
            ("ISeqInStream", 1),
            ("_pad_", 1),
            ("__builtin_clz", 1),
            ("_CountLeadingZeros", 1),
            ("_BitScanReverse", 1),
        ]
    ),
}


def count_tokens(source_code):
    matches = Counter(re.findall(r"\b[_a-zA-Z][_a-zA-Z0-9]*\b", source_code))
    space = sorted(
        [x for x in matches.items() if x[0] not in cannot_be_replaced],
        key=lambda x: x[1],
    )
    cut = 20
    print(*space[:cut], sep="\n")


# Example usage
if __name__ == "__main__":
    input_file = "7z_impl.cpp"

    # Read the C++ source code from a file
    with open(input_file, "r") as file:
        cpp_code = file.read()

    count_tokens(cpp_code)
