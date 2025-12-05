cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=DEBUG-S . -B output -DCMAKE_CXX_FLAGS_DEBUG_INIT="-fsanitize=undefined -fsanitize-trap" -DCMAKE_SHARED_LINKER_FLAGS_DEBUG_INIT="-l:libstdc++.a"

cd output
cmake --build .