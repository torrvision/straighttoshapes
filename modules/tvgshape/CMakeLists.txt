#######################################
# CMakeLists.txt for modules/tvgshape #
#######################################

###########################
# Specify the target name #
###########################

SET(targetname tvgshape)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseBoost.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseOpenCV.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseTorch.cmake)

#############################
# Specify the project files #
#############################

##
SET(toplevel_sources
src/BinaryMaskShapeDescriptorCalculator.cpp
src/RadialShapeDescriptorCalculator.cpp
src/ShapeDescriptorUtil.cpp
)

SET(toplevel_headers
include/tvgshape/BinaryMaskShapeDescriptorCalculator.h
include/tvgshape/RadialShapeDescriptorCalculator.h
include/tvgshape/ShapeDescriptorCalculator.h
include/tvgshape/ShapeDescriptorUtil.h
)

IF(WITH_TORCH)
  SET(toplevel_sources ${toplevel_sources} src/AEShapeDescriptorCalculator.cpp)
  SET(toplevel_headers ${toplevel_headers} include/tvgshape/AEShapeDescriptorCalculator.h)
ENDIF()

#################################################################
# Collect the project files into sources, headers and templates #
#################################################################

SET(sources
${toplevel_sources}
)

SET(headers
${toplevel_headers}
)

#############################
# Specify the source groups #
#############################

SOURCE_GROUP("" FILES ${toplevel_sources} ${toplevel_headers})

##########################################
# Specify additional include directories #
##########################################

INCLUDE_DIRECTORIES(include/tvgshape)

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetLibTarget.cmake)

#############################
# Specify things to install #
#############################

INSTALL(TARGETS ${targetname} DESTINATION lib)
