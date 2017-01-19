####################################
# CMakeLists.txt for tests/scratch #
####################################

ADD_SUBDIRECTORY(boost)

IF(WITH_CUDA)
  ADD_SUBDIRECTORY(cuda)
ENDIF()

ADD_SUBDIRECTORY(tvgutil)
