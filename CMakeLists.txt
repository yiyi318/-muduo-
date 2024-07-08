cmake_minimum_required(VERSION 2.5)

project(mymuduo)

#mymuduo最终编译成so动态库，设置动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")
#设置调试信息
set(CAMKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11" )

#配置最总的可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")


#配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/include)


#定义参与编译的源代码文件
aux_source_directory(./src SRC_LIST)

#编译动态库
add_library(mymuduo SHARED ${SRC_LIST})

set_target_properties(mymuduo PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
)

