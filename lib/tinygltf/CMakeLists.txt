add_compile_options(mizuna PUBLIC -Wno-deprecated-literal-operator)

target_sources(mizuna PRIVATE tinygltf.cpp)
