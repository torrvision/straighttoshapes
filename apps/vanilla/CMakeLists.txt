###################################
# CMakeLists.txt for apps/vanilla #
###################################

###########################
# Specify the target name #
###########################

SET(targetname vanilla)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseBoost.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUBLAS.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDNN5.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCURAND.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseEigen.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseOpenCV.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseOpenMP.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseTorch.cmake)

#############################
# Specify the project files #
#############################

SET(capture_sources
capture/Capture.cpp
)

SET(capture_headers
capture/Capture.h
)

SET(core_sources
core/Box.cpp
core/Datum.cpp
core/Detection.cpp
core/DetectionComparator.cpp
core/DetectionSettings.cpp
core/Shape.cpp
core/Size.cpp
core/VOCBox.cpp
core/VOCObject.cpp
)

SET(core_headers
core/Box.h
core/Datum.h
core/Detection.h
core/DetectionComparator.h
core/DetectionSettings.h
core/MovingAverage.h
core/MovingVectorAverage.h
core/Object.h
core/Shape.h
core/Size.h
core/TupleComparator.h
core/VOCBox.h
core/VOCObject.h
)

##
SET(data_sources
data/TrainingDataGenerator.cpp
data/DataTransformation.cpp
data/DataTransformationFactory.cpp
data/InputDataAssembler.cpp
)

SET(data_headers
data/TrainingDataGenerator.h
data/DataTransformation.h
data/DataTransformationFactory.h
data/InputDataAssembler.h
)

##
SET(dataset_sources
dataset/Dataset.cpp
dataset/COCODatasetInstance.cpp
dataset/COCOAnnotation.cpp
dataset/VOCAnnotation.cpp
dataset/VOCDataset.cpp
dataset/VOCDatasetDetection.cpp
dataset/VOCDatasetSegmentation.cpp
dataset/VOCDatasetSBD.cpp
dataset/VOCDatasetUtil.cpp
dataset/VOCDetectionAnnotation.cpp
dataset/VOCSegmentationAnnotation.cpp
)

SET(dataset_headers
dataset/Dataset.h
dataset/COCODatasetInstance.h
dataset/COCOAnnotation.h
dataset/VOCAnnotation.h
dataset/VOCDataset.h
dataset/VOCDatasetDetection.h
dataset/VOCDatasetSegmentation.h
dataset/VOCDatasetSBD.h
dataset/VOCDatasetUtil.h
dataset/VOCDetectionAnnotation.h
dataset/VOCSegmentationAnnotation.h
)

##
SET(toplevel_sources
main.cpp
DarknetUtil.cpp
Demo.cpp
DetectionUtil.cpp
Evaluator.cpp
Tester.cpp
Trainer.cpp
Util.cpp
)

SET(toplevel_headers
DarknetUtil.h
Demo.h
DetectionUtil.h
Evaluator.h
Tester.h
Trainer.h
Util.h
)

#################################################################
# Collect the project files into sources, headers and templates #
#################################################################

SET(sources
${capture_sources}
${core_sources}
${data_sources}
${dataset_sources}
${toplevel_sources}
)

SET(headers
${capture_headers}
${core_headers}
${data_headers}
${dataset_headers}
${toplevel_headers}
)

#############################
# Specify the source groups #
#############################

SOURCE_GROUP("" FILES ${toplevel_sources} ${toplevel_headers})
SOURCE_GROUP(core FILES ${core_sources} ${core_headers})
SOURCE_GROUP(data FILES ${data_sources} ${data_headers})
SOURCE_GROUP(dataset FILES ${dataset_sources} ${dataset_headers})

##########################################
# Specify additional include directories #
##########################################

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/darknet/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/evaluation/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/tvgplot/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/tvgshape/include)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/modules/tvgutil/include)

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDAAppTarget.cmake)

#################################
# Specify the libraries to link #
#################################

TARGET_LINK_LIBRARIES(${targetname} darknet evaluation tvgplot tvgshape tvgutil)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkBoost.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkCUBLAS.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkCUDNN5.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkCURAND.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkOpenCV.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/LinkTorch.cmake)

#########################################
# Copy resource files to the build tree #
#########################################

ADD_CUSTOM_COMMAND(TARGET ${targetname} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/apps/vanilla/resources" "$<TARGET_FILE_DIR:${targetname}>/resources")

IF(MSVC_IDE)
  ADD_CUSTOM_COMMAND(TARGET ${targetname} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/apps/vanilla/resources" "${PROJECT_BINARY_DIR}/apps/spaintgui/resources")
ENDIF()


#############################
# Specify things to install #
#############################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/InstallApp.cmake)
