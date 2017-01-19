######################################
# CMakeLists.txt for modules/darknet #
######################################

###########################
# Specify the target name #
###########################

SET(targetname darknet)

################################
# Specify the libraries to use #
################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDA.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUBLAS.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCUDNN5.cmake)
INCLUDE(${PROJECT_SOURCE_DIR}/cmake/UseCURAND.cmake)

#############################
# Specify the project files #
#############################

##
SET(toplevel_sources
src/activation_kernels.cu
src/activation_layer.c
src/activations.c
src/avgpool_layer.c
src/avgpool_layer_kernels.cu
src/batchnorm_layer.c
src/blas.c
src/blas_kernels.cu
src/box.c
src/col2im.c
src/col2im_kernels.cu
src/connected_layer.c
src/convolutional_kernels.cu
src/convolutional_layer.c
src/cost_layer.c
src/crnn_layer.c
src/crop_layer.c
src/crop_layer_kernels.cu
src/cuda.c
src/deconvolutional_kernels.cu
src/deconvolutional_layer.c
src/detection_layer.c
src/dropout_layer.c
src/dropout_layer_kernels.cu
src/gemm.c
src/gru_layer.c
src/im2col.c
src/im2col_kernels.cu
src/layer.c
src/list.c
src/local_layer.c
src/matrix.c
src/maxpool_layer.c
src/maxpool_layer_kernels.cu
src/network.c
src/network_kernels.cu
src/normalization_layer.c
src/option_list.c
src/parser.c
src/rnn_layer.c
src/rnn_vid.c
src/route_layer.c
src/shortcut_layer.c
src/softmax_layer.c
src/softmax_layer_kernels.cu
src/utils.c
)

SET(toplevel_headers
include/darknet/activation_layer.h
include/darknet/activations.h
include/darknet/avgpool_layer.h
include/darknet/batchnorm_layer.h
include/darknet/blas.h
include/darknet/box.h
include/darknet/col2im.h
include/darknet/connected_layer.h
include/darknet/convolutional_layer.h
include/darknet/cost_layer.h
include/darknet/crnn_layer.h
include/darknet/crop_layer.h
include/darknet/cuda.h
include/darknet/deconvolutional_layer.h
include/darknet/detection_layer.h
include/darknet/dropout_layer.h
include/darknet/gemm.h
include/darknet/gru_layer.h
include/darknet/im2col.h
include/darknet/layer.h
include/darknet/list.h
include/darknet/local_layer.h
include/darknet/matrix.h
include/darknet/maxpool_layer.h
include/darknet/network.h
include/darknet/normalization_layer.h
include/darknet/option_list.h
include/darknet/parser.h
include/darknet/rnn_layer.h
include/darknet/route_layer.h
include/darknet/server.h
include/darknet/shortcut_layer.h
include/darknet/softmax_layer.h
include/darknet/stb_image.h
include/darknet/stb_image_write.h
include/darknet/utils.h
)

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

INCLUDE_DIRECTORIES(include/darknet)

##########################################
# Specify the target and where to put it #
##########################################

INCLUDE(${PROJECT_SOURCE_DIR}/cmake/SetCUDALibTarget.cmake)

#############################
# Specify things to install #
#############################

INSTALL(TARGETS ${targetname} DESTINATION lib)
