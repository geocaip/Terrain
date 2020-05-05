cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(dsm_knl_tri)#设置工程名称
#GDAL
find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIRS})
#LASlib
include_directories(${CMAKE_SOURCE_DIR}/LASlib)
link_directories(${CMAKE_SOURCE_DIR}/lib)
#Triangle
include_directories(${CMAKE_SOURCE_DIR}/Triangle/include)
link_directories(${CMAKE_SOURCE_DIR}/Triangle/lib)

FILE(GLOB_RECURSE SRCS_FILES *.cpp *.c *.cc *.cxx)
add_executable (dsm_knl_tri ${SRCS_FILES})

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
#target_link_libraries( ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/GDAL/lib/gdal_i.lib)
target_link_libraries(${PROJECT_NAME} ${GDAL_LIBRARY})
target_link_libraries( ${PROJECT_NAME} LASlib)
target_link_libraries( ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/Triangle/lib/triangle.lib)