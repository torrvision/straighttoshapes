###################
# UseCUBLAS.cmake #
###################

OPTION(WITH_CUBLAS "Vuild with CUBLAS support?" ${CUDA_FOUND})

IF(WITH_CUBLAS)
  IF("${CMAKE_SYSTEM}" MATCHES "Linux")
    FIND_LIBRARY(CUBLAS_LIBRARY cublas HINTS "${CUDA_TOOLKIT_ROOT_DIR}/targets/x86_64-linux/lib")
  ELSE()
    FIND_LIBRARY(CUBLAS_LIBRARY cublas)
  ENDIF()

  ADD_DEFINITIONS(-DWITH_CUBLAS)
ENDIF()
