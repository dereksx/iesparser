cmake_minimum_required(VERSION 2.8.9)
project(iesparser)


#Options
option (USE_STATIC_BOOST "Use static Boost libraries" ON)


#Boost
set (Boost_MULTITHREADED TRUE)
if (USE_STATIC_BOOST)
    set (Boost_USE_STATIC_LIBS TRUE)
endif ()

set (BOOST_NEEDED_LIBS regex)
find_package (Boost 1.55 REQUIRED ${BOOST_NEEDED_LIBS})

include_directories (SYSTEM ${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})


#Sources
file (GLOB SOURCES "src/*.cpp")

#Headers
file (GLOB HEADERS "include/*.h")
include_directories("include")


#Generate the shared library from the sources
add_library(iesparser STATIC ${SOURCES} ${HEADERS})
