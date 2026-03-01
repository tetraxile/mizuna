add_compile_options(afl PUBLIC -Wno-deprecated-literal-operator)

target_sources(afl PRIVATE tinygltf.cpp)
