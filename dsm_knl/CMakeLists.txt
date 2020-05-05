cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
#GDAL
project(dsm_knl)
find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIRS})
#opencv
find_package(Opencv REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#include_directories(${CMAKE_SOURCE_DIR}/GDAL/include)
#link_directories(${CMAKE_SOURCE_DIR}/GDAL/lib)
#LASlib
include_directories(${CMAKE_SOURCE_DIR}/LASlib)
link_directories(${CMAKE_SOURCE_DIR}/lib)

FILE(GLOB_RECURSE SRCS_FILES *.cpp *.c *.cc *.cxx)
add_executable (dsm_knl ${SRCS_FILES})

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
target_link_libraries(${PROJECT_NAME} ${GDAL_LIBRARY})
#target_link_libraries( ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/GDAL/lib/gdal_i.lib)
target_link_libraries( ${PROJECT_NAME} LASlib)
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS})