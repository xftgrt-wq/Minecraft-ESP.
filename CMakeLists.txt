cmake_minimum_required(VERSION 3.22.1)
project(MinecraftESP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG 8c19383f417744d3165165de9afca42b60059174
)
FetchContent_MakeAvailable(preloader_android)

add_library(minecraft_esp SHARED
    src/main/cpp/native-lib.cpp
    src/main/cpp/esp_geometry.cpp
)

target_include_directories(minecraft_esp PRIVATE src/main/cpp)
target_compile_features(minecraft_esp PRIVATE cxx_std_20)
target_compile_options(minecraft_esp PRIVATE
    -Wall -Wextra -Werror -fvisibility=hidden
)
target_link_libraries(minecraft_esp PRIVATE preloader log android)
