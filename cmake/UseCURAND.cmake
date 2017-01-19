###################
# UseCURAND.cmake #
###################

OPTION(WITH_CURAND "Vaild with CURAND support?" ${CUDA_FOUND})

IF(WITH_CURAND)
  IF("${CMAKE_SYSTEM}" MATCHES "Linux")
    FIND_LIBRARY(CURAND_LIBRARY curand HINTS "/usr/local/cuda/targets/x86_64-linux/lib")
  ELSE()
    FIND_LIBRARY(CURAND_LIBRARY curand)
  ENDIF()

  ADD_DEFINITIONS(-DWITH_CURAND)
ENDIF()
