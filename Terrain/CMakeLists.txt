cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(Terrain)#设置工程名称

###配置环境###

#添加qt的配置文件//
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
##打开相关自动选项//

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})

#添加opencv的路径//
#find_package(Opencv REQUIRED)
#include_directories(${OpenCV_INCLUDE_DIRS})
#添加GDAL路径//
find_package(GDAL REQUIRED)
include_directories(${GDAL_INCLUDE_DIRS})
#include_directories(${CMAKE_SOURCE_DIR}/GDAL/include)
#link_directories(${CMAKE_SOURCE_DIR}/GDAL/lib)
#LASlib
include_directories(${CMAKE_SOURCE_DIR}/LASlib)
link_directories(${CMAKE_SOURCE_DIR}/lib)
############
FILE(GLOB HDRS_FILES "*.h" "*.hpp")
FILE(GLOB SRCS_FILES "*.c" "*.cpp")
FILE(GLOB UIS_FILES "*.ui")
#FILE(GLOB RCS_FILES "image/*.qrc")
FILE(GLOB RCS_FILES "../image/*.rc")
qt5_wrap_ui(WRAP_FILES ${UIS_FILES})
add_executable(Terrain ${SRCS_FILES} ${HDRS_FILES} ${WRAP_FILES} ${RCS_FILES})
target_link_libraries( ${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)
#target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS})
#target_link_libraries( ${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/GDAL/lib/gdal_i.lib)
target_link_libraries(${PROJECT_NAME} ${GDAL_LIBRARY})
target_link_libraries( ${PROJECT_NAME} LASlib)
#设置输出路径
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)







