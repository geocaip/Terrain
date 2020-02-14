
project(LASlib)
FILE(GLOB HDRS_FILES "*.h" "*.hpp")
FILE(GLOB SRCS_FILES "*.c" "*.cpp")
add_library(LASlib STATIC ${HDRS_FILES} ${SRCS_FILES})
#设置输出路径
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)