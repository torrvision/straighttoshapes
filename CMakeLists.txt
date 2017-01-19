#######################################
# Top-level CMakeLists.txt for spaint #
#######################################

cmake_minimum_required(VERSION 2.8)

PROJECT(straighttoshapes)

#################################
# Add additional compiler flags #
#################################

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-local-typedefs -ansi")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# If on Mac OS X 10.9 (Mavericks), disable the annoying deprecation warnings (e.g. for GLU).
IF(${CMAKE_SYSTEM} MATCHES "Darwin-13.")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
ENDIF()

#########################################
# Specify the default install directory #
#########################################

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install CACHE PATH "" FORCE)
ENDIF()

##########################
# Specify subdirectories #
##########################

ADD_SUBDIRECTORY(modules)

ADD_SUBDIRECTORY(apps)

OPTION(BUILD_TESTS "Build the tests?" OFF)
IF(BUILD_TESTS)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF()
