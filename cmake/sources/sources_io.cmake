####################################################################
# Copyright © 2018 Computational Molecular Biology Group,          #
#                  Freie Universität Berlin (GER)                  #
#                                                                  #
# Redistribution and use in source and binary forms, with or       #
# without modification, are permitted provided that the            #
# following conditions are met:                                    #
#  1. Redistributions of source code must retain the above         #
#     copyright notice, this list of conditions and the            #
#     following disclaimer.                                        #
#  2. Redistributions in binary form must reproduce the above      #
#     copyright notice, this list of conditions and the following  #
#     disclaimer in the documentation and/or other materials       #
#     provided with the distribution.                              #
#  3. Neither the name of the copyright holder nor the names of    #
#     its contributors may be used to endorse or promote products  #
#     derived from this software without specific                  #
#     prior written permission.                                    #
#                                                                  #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND           #
# CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,      #
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF         #
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE         #
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR            #
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,         #
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER #
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,      #
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    #
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF      #
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                       #
####################################################################

# VERSION 1 - Functional!
SET(SOURCES_DIR "${READDY_GLOBAL_DIR}/readdy/main/io")

# hdf5
#SET(HDF5_USE_STATIC_LIBRARIES OFF)
FIND_PACKAGE(HDF5 COMPONENTS HL REQUIRED)
MESSAGE(STATUS "HDF5_FOUND: ${HDF5_FOUND}, version ${HDF5_VERSION}")

# includes
SET(IO_INCLUDE_DIRS "${COMMON_INCLUDE_DIRS};${HDF5_INCLUDE_DIRS};${HDF5_HL_INCLUDE_DIR}" CACHE INTERNAL "IO Include dirs")

# libraries
SET(READDY_IO_LIBRARIES "${READDY_COMMON_LIBRARIES};${HDF5_LIBRARIES};${HDF5_HL_LIBRARIES}" CACHE INTERNAL "IO Libraries")
LIST(REMOVE_DUPLICATES READDY_IO_LIBRARIES)

# sources
LIST(APPEND READDY_IO_SOURCES "${SOURCES_DIR}/BloscFilter.cpp")
LIST(APPEND READDY_IO_SOURCES "${SOURCES_DIR}/blosc_filter.h")
LIST(APPEND READDY_IO_SOURCES "${SOURCES_DIR}/blosc_filter.c")
LIST(APPEND READDY_IO_SOURCES "${SOURCES_DIR}/blosc_filter.h")
LIST(APPEND READDY_IO_SOURCES "${SOURCES_DIR}/blosc_plugin.c")

# all sources
LIST(APPEND READDY_ALL_SOURCES ${READDY_IO_SOURCES})

### Versions for build tool testing ###
## Version 2 (Not working)
#set(SOURCES_DIR "${READDY_GLOBAL_DIR}/readdy/main/io")
#
## Try to find HDF5 relative to Python interpreter
#execute_process(
#        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_path('include'))"
#        OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#execute_process(
#        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"
#        OUTPUT_VARIABLE PYTHON_LIB_DIR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#
## Message for debug
#message(STATUS "Python Include Dir: ${PYTHON_INCLUDE_DIR}")
#message(STATUS "Python Lib Dir: ${PYTHON_LIB_DIR}")
#
## Try to find HDF5 using these paths
#find_path(HDF5_INCLUDE_DIR hdf5.h HINTS "${PYTHON_INCLUDE_DIR}" "${PYTHON_INCLUDE_DIR}/hdf5")
#find_library(HDF5_LIBRARY NAMES hdf5 HINTS "${PYTHON_LIB_DIR}")
#
#find_path(HDF5_HL_INCLUDE_DIR hdf5_hl.h HINTS "${PYTHON_INCLUDE_DIR}" "${PYTHON_INCLUDE_DIR}/hdf5")
#find_library(HDF5_HL_LIBRARY NAMES hdf5_hl HINTS "${PYTHON_LIB_DIR}")
#
## fallback: try system or conan config package
#if(NOT HDF5_INCLUDE_DIR OR NOT HDF5_LIBRARY OR NOT HDF5_HL_LIBRARY)
#    message(STATUS "HDF5 not found in Python paths, trying find_package(HDF5 CONFIG REQUIRED)")
#    find_package(HDF5 CONFIG REQUIRED COMPONENTS HL)
#endif()
#
## Set include directories
#set(IO_INCLUDE_DIRS
#        "${COMMON_INCLUDE_DIRS}"
#        "${HDF5_INCLUDE_DIR}"
#        "${HDF5_HL_INCLUDE_DIR}"
#        ${HDF5_INCLUDE_DIRS}  # from find_package
#        CACHE INTERNAL "IO Include dirs"
#)
#
## Set libraries
#set(READDY_IO_LIBRARIES
#        "${READDY_COMMON_LIBRARIES}"
#        "${HDF5_LIBRARY}"
#        "${HDF5_HL_LIBRARY}"
#        ${HDF5_LIBRARIES}  # from find_package
#        ${HDF5_HL_LIBRARIES}
#        CACHE INTERNAL "IO Libraries"
#)
#list(REMOVE_DUPLICATES READDY_IO_LIBRARIES)
#
## Sources
#list(APPEND READDY_IO_SOURCES
#        "${SOURCES_DIR}/BloscFilter.cpp"
#        "${SOURCES_DIR}/blosc_filter.h"
#        "${SOURCES_DIR}/blosc_filter.c"
#        "${SOURCES_DIR}/blosc_plugin.c"
#)
#
## All sources
#list(APPEND READDY_ALL_SOURCES ${READDY_IO_SOURCES})
#
#
## VERSION 3 (Not working)
## Attempt to find HDF5 headers and libs near Python
#execute_process(
#        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_path('include'))"
#        OUTPUT_VARIABLE PYTHON_INCLUDE_DIR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#execute_process(
#        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"
#        OUTPUT_VARIABLE PYTHON_LIB_DIR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#
#find_path(HDF5_INCLUDE_DIR hdf5.h HINTS "${PYTHON_INCLUDE_DIR}" "${PYTHON_INCLUDE_DIR}/hdf5")
#find_library(HDF5_LIBRARY NAMES hdf5 HINTS "${PYTHON_LIB_DIR}")
#
#find_path(HDF5_HL_INCLUDE_DIR hdf5_hl.h HINTS "${PYTHON_INCLUDE_DIR}" "${PYTHON_INCLUDE_DIR}/hdf5")
#find_library(HDF5_HL_LIBRARY NAMES hdf5_hl HINTS "${PYTHON_LIB_DIR}")
#
## Try Conan or system package if not found near Python
#if(NOT HDF5_INCLUDE_DIR OR NOT HDF5_LIBRARY OR NOT HDF5_HL_LIBRARY)
#    message(STATUS "HDF5 not found in Python paths, trying find_package(HDF5 CONFIG REQUIRED)")
#    find_package(HDF5 CONFIG REQUIRED COMPONENTS HL)
#endif()
#
## Setup includes safely
#set(IO_INCLUDE_DIRS "${COMMON_INCLUDE_DIRS}")
#
#if(HDF5_INCLUDE_DIR)
#    list(APPEND IO_INCLUDE_DIRS "${HDF5_INCLUDE_DIR}")
#endif()
#if(HDF5_HL_INCLUDE_DIR)
#    list(APPEND IO_INCLUDE_DIRS "${HDF5_HL_INCLUDE_DIR}")
#endif()
#if(HDF5_INCLUDE_DIRS)
#    list(APPEND IO_INCLUDE_DIRS "${HDF5_INCLUDE_DIRS}")
#endif()
#
## Setup libs safely
#set(READDY_IO_LIBRARIES "${READDY_COMMON_LIBRARIES}")
#if(HDF5_LIBRARY)
#    list(APPEND READDY_IO_LIBRARIES "${HDF5_LIBRARY}")
#endif()
#if(HDF5_HL_LIBRARY)
#    list(APPEND READDY_IO_LIBRARIES "${HDF5_HL_LIBRARY}")
#endif()
#if(HDF5_LIBRARIES)
#    list(APPEND READDY_IO_LIBRARIES "${HDF5_LIBRARIES}")
#endif()
#if(HDF5_HL_LIBRARIES)
#    list(APPEND READDY_IO_LIBRARIES "${HDF5_HL_LIBRARIES}")
#endif()
#
#list(REMOVE_DUPLICATES IO_INCLUDE_DIRS)
#list(REMOVE_DUPLICATES READDY_IO_LIBRARIES)
#
## Set cache entries
#set(IO_INCLUDE_DIRS "${IO_INCLUDE_DIRS}" CACHE INTERNAL "IO Include dirs")
#set(READDY_IO_LIBRARIES "${READDY_IO_LIBRARIES}" CACHE INTERNAL "IO Libraries")
#
## Append source files
#list(APPEND READDY_IO_SOURCES
#        "${SOURCES_DIR}/BloscFilter.cpp"
#        "${SOURCES_DIR}/blosc_filter.h"
#        "${SOURCES_DIR}/blosc_filter.c"
#        "${SOURCES_DIR}/blosc_plugin.c"
#)
#
#list(APPEND READDY_ALL_SOURCES ${READDY_IO_SOURCES})