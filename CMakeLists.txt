cmake_minimum_required(VERSION 3.15)
project(coze_api)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Conan 生成的依赖配置
find_package(cjson REQUIRED)
find_package(CURL REQUIRED)

# 添加子目录
add_subdirectory(external)
add_subdirectory(coze_api)