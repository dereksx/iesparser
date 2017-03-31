cmake_minimum_required(VERSION 2.8.9)
project(iesparser)

#Sources
file (GLOB SOURCES "src/*.cpp")

#Headers
file (GLOB HEADERS "include/*.h")
include_directories("include")
 
#Generate the shared library from the sources
add_library(iesparser STATIC ${SOURCES} ${HEADERS})
