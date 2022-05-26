cmake_minimum_required(VERSION 3.19)
project(unit-static C)
add_library(${PROJECT_NAME} STATIC all.c)
include_directories(${PROJECT_NAME} PUBLIC .)
