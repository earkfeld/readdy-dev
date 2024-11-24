cmake_minimum_required(VERSION 3.12)

project(SimpleAction LANGUAGES CXX)

# Locate ReaDDy library
find_path(READDY_INCLUDE_DIR NAMES readdy/readdy.h DOC "The ReaDDy include directory")
find_library(READDY_LIBRARY NAMES readdy DOC "The ReaDDy library")
find_library(READDY_CPU_LIBRARY NAMES readdy_kernel_cpu
        HINTS "${CMAKE_PREFIX_PATH}/readdy/readdy_plugins"
        DOC "The ReaDDy CPU kernel")
include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(READDY REQUIRED_VARS READDY_LIBRARY READDY_INCLUDE_DIR READDY_CPU_LIBRARY)
if(READDY_FOUND)
    set(READDY_LIBRARIES ${READDY_LIBRARY} ${READDY_CPU_LIBRARY})
    set(READDY_INCLUDE_DIRS ${READDY_INCLUDE_DIR})
    if(NOT TARGET READDY::READDY)
        add_library(READDY::READDY UNKNOWN IMPORTED)
        set_target_properties(READDY::READDY PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${READDY_INCLUDE_DIR}")
        set_property(TARGET READDY::READDY APPEND PROPERTY IMPORTED_LOCATION "${READDY_LIBRARY}")
    endif()
    if(NOT TARGET READDY::READDY_CPU)
        add_library(READDY::READDY_CPU UNKNOWN IMPORTED)
        set_target_properties(READDY::READDY_CPU PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${READDY_INCLUDE_DIR}")
        set_property(TARGET READDY::READDY_CPU APPEND PROPERTY IMPORTED_LOCATION "${READDY_CPU_LIBRARY}")
    endif()
endif()

mark_as_advanced(READDY_INCLUDE_DIR READDY_LIBRARY)

# Locate pybind11
find_package(pybind11 REQUIRED)

# Define the target for the SimpleAction library
add_library(SimpleAction MODULE SimpleAction.cpp)

# Include directories
target_include_directories(SimpleAction PRIVATE ${READDY_INCLUDE_DIRS})

# Link libraries
target_link_libraries(SimpleAction PRIVATE ${READDY_LIBRARIES} pybind11::module)

# Set C++ standard
set_target_properties(SimpleAction PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Ensure the output has the correct file extension for Python modules
set_target_properties(SimpleAction PROPERTIES PREFIX "" SUFFIX ".so")


#find_path(READDY_INCLUDE_DIR NAMES readdy/readdy.h DOC "The ReaDDy include directory")
#find_library(READDY_LIBRARY NAMES readdy DOC "The ReaDDy library")
#find_library(READDY_CPU_LIBRARY NAMES readdy_kernel_cpu
#        HINTS "${CMAKE_PREFIX_PATH}/readdy/readdy_plugins"
#        DOC "The ReaDDy CPU kernel")
#include(FindPackageHandleStandardArgs)
#
#find_package_handle_standard_args(READDY REQUIRED_VARS READDY_LIBRARY READDY_INCLUDE_DIR READDY_CPU_LIBRARY)
#if(READDY_FOUND)
#    set(READDY_LIBRARIES ${READDY_LIBRARY} ${READDY_CPU_LIBRARY})
#    set(READDY_INCLUDE_DIRS ${READDY_INCLUDE_DIR})
#    if(NOT TARGET READDY::READDY)
#        add_library(READDY::READDY UNKNOWN IMPORTED)
#        set_target_properties(READDY::READDY PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${READDY_INCLUDE_DIR}")
#        set_property(TARGET READDY::READDY APPEND PROPERTY IMPORTED_LOCATION "${READDY_LIBRARY}")
#    endif()
#    if(NOT TARGET READDY::READDY_CPU)
#        add_library(READDY::READDY_CPU UNKNOWN IMPORTED)
#        set_target_properties(READDY::READDY_CPU PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${READDY_INCLUDE_DIR}")
#        set_property(TARGET READDY::READDY_CPU APPEND PROPERTY IMPORTED_LOCATION "${READDY_CPU_LIBRARY}")
#    endif()
#endif()
#
#mark_as_advanced(READDY_INCLUDE_DIR READDY_LIBRARY)
