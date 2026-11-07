cmake_minimum_required(VERSION 3.22.1)
project(MinecraftESP LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 0.2.2
)
FetchContent_MakeAvailable(preloader_android)

add_library(minecraft_esp SHARED
    src/main/cpp/native-lib.cpp
)

target_compile_features(minecraft_esp PRIVATE cxx_std_20)
target_compile_options(minecraft_esp PRIVATE -Wall -Wextra -fvisibility=hidden)
target_link_libraries(minecraft_esp PRIVATE preloader log android)
