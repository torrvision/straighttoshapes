#ifndef CUDA_H
#define CUDA_H

extern int gpu_index;

#ifdef WITH_CUDA

#define BLOCK 512

#include "cuda_runtime.h"
#include "curand.h"
#include "cublas_v2.h"

#ifdef WITH_CUDNN5
#include "cudnn.h"
#endif

void check_error(cudaError_t status);
cublasHandle_t blas_handle();
float *cuda_make_array(float *x, size_t n);
int *cuda_make_int_array(size_t n);
void cuda_push_array(float *x_gpu, float *x, size_t n);
void cuda_pull_array(float *x_gpu, float *x, size_t n);
void cuda_free(float *x_gpu);
void cuda_random(float *x_gpu, size_t n);
float cuda_compare(float *x_gpu, float *x, size_t n, char *s);
dim3 cuda_gridsize(size_t n);

#ifdef WITH_CUDNN5
cudnnHandle_t cudnn_handle();
#endif

#endif
#endif
