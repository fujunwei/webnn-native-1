// Copyright (C) 2018-2020 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef NGRAPH_C_API_H
#define NGRAPH_C_API_H

#include <stdint.h>
#include <stdio.h>

#include <c_api/ie_c_api.h>

#ifdef __cplusplus
#define NGRAPH_C_EXTERN extern "C"
#else
#define NGRAPH_C_EXTERN
#endif

#if defined(__GNUC__) && (__GNUC__ < 4)
#define NGRAPH_C_API(...) NGRAPH_C_EXTERN __VA_ARGS__
#else
#if defined(_WIN32)
#ifdef NGRAPH_c_wraper_EXPORTS
#define NGRAPH_C_API(...) \
  NGRAPH_C_EXTERN __declspec(dllexport) __VA_ARGS__ __cdecl
#else
#define NGRAPH_C_API(...) \
  NGRAPH_C_EXTERN __declspec(dllimport) __VA_ARGS__ __cdecl
#endif
#else
#define NGRAPH_C_API(...) \
  NGRAPH_C_EXTERN __attribute__((visibility("default"))) __VA_ARGS__
#endif
#endif

typedef struct ngraph_node ngraph_node_t;
typedef struct ngraph_function ngraph_function_t;

enum ngraph_interpolation_mode : uint32_t {
  NearestNeighbor = 0x00000000,
  Linear = 0x00000001,
};

enum ngraph_shape_calc_mode : uint32_t {
  Sizes = 0x00000000,
  Scales = 0x00000001,
};

typedef struct interpolate_attrs {
  ngraph_interpolation_mode mode;
  ngraph_shape_calc_mode shape_calculation_mode;
} interpolate_attrs_t;

enum ngraph_padding_mode : uint32_t {
  Constant = 0x00000000,
  Edge = 0x00000001,
  Reflection = 0x00000002,
  Symmetric = 0x00000003,
};

enum ngraph_auto_pad : uint32_t {
  Explicit = 0x00000000,
  SameUpper = 0x00000001,
  SameLower = 0x00000002,
};

NGRAPH_C_API(IEStatusCode)
ngraph_get_shape(const ngraph_node_t* node, dimensions_t* dimensions);
NGRAPH_C_API(IEStatusCode)
ngraph_get_name(const ngraph_node_t* node, char** name);

NGRAPH_C_API(IEStatusCode)
ngraph_input(const tensor_desc_t* tensorDesc, ngraph_node_t** node);
NGRAPH_C_API(void) ngraph_node_free(ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_constant(const tensor_desc_t* tensorDesc,
                const ie_blob_t* blob,
                ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_add(const ngraph_node_t* a,
           const ngraph_node_t* b,
           ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_output(const ngraph_node_t* result, ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
create_ngraph_function(ngraph_node_t** output,
                       uint32_t output_count,
                       ngraph_node_t** input,
                       uint32_t input_count,
                       ngraph_function_t** function);
NGRAPH_C_API(IEStatusCode) transpose_sinking(ngraph_function_t* ngraph_function);
NGRAPH_C_API(IEStatusCode)
create_network(ngraph_function_t* ngraph_function, ie_network_t** network);
NGRAPH_C_API(void) ngraph_function_free(ngraph_function_t* function);

NGRAPH_C_API(IEStatusCode)
ngraph_mul(const ngraph_node_t* a,
           const ngraph_node_t* b,
           ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_sub(const ngraph_node_t* a,
           const ngraph_node_t* b,
           ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_mat_mul(const ngraph_node_t* a,
               const ngraph_node_t* b,
               ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_leaky_relu(const ngraph_node_t* a,
                  const ngraph_node_t* b,
                  ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_transpose(const ngraph_node_t* a,
                 const ngraph_node_t* b,
                 ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_reshape(const ngraph_node_t* a,
               const ngraph_node_t* b,
               ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_max(const ngraph_node_t* a,
           const ngraph_node_t* b,
           ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_min(const ngraph_node_t* a,
           const ngraph_node_t* b,
           ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_power(const ngraph_node_t* a,
             const ngraph_node_t* b,
             ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_divide(const ngraph_node_t* a,
              const ngraph_node_t* b,
              ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_relu(const ngraph_node_t* input, ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_softmax(const ngraph_node_t* input, ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_sigmoid(const ngraph_node_t* input, ngraph_node_t** node);
NGRAPH_C_API(IEStatusCode)
ngraph_tanh(const ngraph_node_t* input, ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_concat(ngraph_node_t** inputs,
              uint32_t input_count,
              uint32_t axis,
              ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_interpolate(const ngraph_node_t* input,
                   const ngraph_node_t* sizes,
                   const ngraph_node_t* scales,
                   const ngraph_node_t* axes,
                   const interpolate_attrs_t* attrs,
                   ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_pad(const ngraph_node_t* input,
           const ngraph_node_t* begin,
           const ngraph_node_t* end,
           const ngraph_node_t* value,
           ngraph_padding_mode mode,
           ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_reduce_mean(const ngraph_node_t* input,
                   const ngraph_node_t* axes,
                   bool keep_dimensions,
                   ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_clamp(const ngraph_node_t* input,
             float min,
             float max,
             ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_batch_norm_inference(const ngraph_node_t* input,
                            const ngraph_node_t* scale,
                            const ngraph_node_t* bias,
                            const ngraph_node_t* mean,
                            const ngraph_node_t* variance,
                            double epsilon,
                            ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_average_pool(const ngraph_node_t* input,
                    size_t const* strides,
                    uint32_t strides_count,
                    size_t const* padding,
                    uint32_t padding_count,
                    size_t const* windowDimensions,
                    uint32_t windowDimensionsCount,
                    ngraph_auto_pad mode,
                    ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_max_pool(const ngraph_node_t* input,
                size_t const* strides,
                uint32_t strides_count,
                size_t const* padding,
                uint32_t padding_count,
                size_t const* windowDimensions,
                uint32_t windowDimensionsCount,
                ngraph_auto_pad mode,
                ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_convolution(const ngraph_node_t* input,
                   const ngraph_node_t* filter,
                   size_t const* strides,
                   uint32_t strides_count,
                   int32_t const* padding,
                   uint32_t padding_count,
                   size_t const* dilations,
                   uint32_t dilations_count,
                   ngraph_auto_pad mode,
                   ngraph_node_t** node);

NGRAPH_C_API(IEStatusCode)
ngraph_group_convolution(const ngraph_node_t* input,
                         const ngraph_node_t* filter,
                         size_t const* strides,
                         uint32_t strides_count,
                         int32_t const* padding,
                         uint32_t padding_count,
                         size_t const* dilations,
                         uint32_t dilations_count,
                         ngraph_auto_pad mode,
                         ngraph_node_t** node);

#endif  // NGRAPH_C_API_H
