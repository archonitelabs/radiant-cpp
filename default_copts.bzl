""" Default copts for various compilers"""

RAD_CPP14 = select({
    "//:msvc": ["/std:c++14"],
    "//:gcc": ["-std=c++14"],
    "//:clang": ["-std=c++14"],
})

RAD_CPP17 = select({
    "//:msvc": ["/std:c++17"],
    "//:gcc": ["-std=c++17"],
    "//:clang": ["-std=c++17"],
})

RAD_CPP20 = select({
    "//:msvc": ["/std:c++20"],
    "//:gcc": ["-std=c++20"],
    "//:clang": ["-std=c++20"],
})

RAD_ASAN_COPTS = [
    "-fsanitize=address",
    "-DADDRESS_SANITIZER",
    "-g",
    "-fno-omit-frame-pointer",
]

RAD_ASAN_LINKOPTS = [
    "-fsanitize=address",
]

RAD_GCC_COPTS = [
    "-Wall",
    "-Wextra",
    "-Wcast-qual",
    "-Wconversion",
    "-Wconversion-null",
    "-Wmissing-declarations",
    "-Woverlength-strings",
    "-Wpointer-arith",
    "-Wundef",
    "-Wunused-local-typedefs",
    "-Wunused-result",
    "-Wvarargs",
    "-Wvla",
    "-Wwrite-strings",
    "-Wno-multichar",
    "-Werror",
    "-Wpedantic",
]

RAD_GCC_LINKOPTS = []

RAD_DEFAULT_COPTS = select({
    "//:msvc": [
        "/W4",
        "/WX",
        "/DNOMINMAX",
        "/Zc:__cplusplus",
    ],
    "//:gcc": RAD_GCC_COPTS + RAD_ASAN_COPTS,
    "//:clang": RAD_GCC_COPTS,
})

RAD_DEFAULT_LINKOPTS = select({
    "//:msvc": ["/NATVIS:radiant/Rad.natvis"],
    "//:gcc": RAD_GCC_LINKOPTS + RAD_ASAN_LINKOPTS,
    "//:clang": RAD_GCC_LINKOPTS,
})
