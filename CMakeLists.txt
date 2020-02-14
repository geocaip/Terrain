cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(Terrain)
add_subdirectory (LASlib)
add_subdirectory (Terrain)
add_subdirectory (dem_knl)
add_subdirectory (dsm_knl)
add_subdirectory (Points_Block)