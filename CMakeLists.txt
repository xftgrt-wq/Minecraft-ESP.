cmake_minimum_required(VERSION 3.22.1)
project(MinecraftESP LANGUAGES CXX)

add_library(minecraft_esp SHARED
    src/main/cpp/native-lib.cpp
)

target_compile_features(minecraft_esp PRIVATE cxx_std_17)
target_compile_options(minecraft_esp PRIVATE -Wall -Wextra -fvisibility=hidden)
target_link_libraries(minecraft_esp log android)
