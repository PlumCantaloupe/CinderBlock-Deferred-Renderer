#pragma once
#include "cinder/CinderResources.h"


/****** Deferred resources *****/
//images
#define NOISE_SAMPLER                   CINDER_RESOURCE( ../resources/DeferredResources/textures/, random.png, 128, IMAGE )

//shaders
#define SSAO_VERT                       CINDER_RESOURCE( ../resources/DeferredResources/shaders/, SSAO_glsl.vert, 129, GLSL )
#define SSAO_FRAG                       CINDER_RESOURCE( ../resources/DeferredResources/shaders/, SSAO_glsl.frag, 130, GLSL )

#define DEFER_VERT                      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Deferred_glsl.vert, 131, GLSL )
#define DEFER_FRAG                      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Deferred_glsl.frag, 132, GLSL )

#define BBlender_VERT                   CINDER_RESOURCE( ../resources/DeferredResources/shaders/, BasicBlender_glsl.vert, 133, GLSL )
#define BBlender_FRAG                   CINDER_RESOURCE( ../resources/DeferredResources/shaders/, BasicBlender_glsl.frag, 134, GLSL )

#define BLUR_H_VERT                     CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Blur_h_glsl.vert, 135, GLSL )
#define BLUR_H_FRAG                     CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Blur_h_glsl.frag, 136, GLSL )

#define BLUR_V_VERT                     CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Blur_v_glsl.vert, 137, GLSL )
#define BLUR_V_FRAG                     CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Blur_v_glsl.frag, 138, GLSL )

#define LIGHT_VERT                      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Light_glsl.vert, 139, GLSL )
#define LIGHT_FRAG                      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, Light_glsl.frag, 140, GLSL )

#define ALPHA_RGB_VERT                  CINDER_RESOURCE( ../resources/DeferredResources/shaders/, AlphaToRGB_glsl.vert, 141, GLSL )
#define ALPHA_RGB_FRAG                  CINDER_RESOURCE( ../resources/DeferredResources/shaders/, AlphaToRGB_glsl.frag, 142, GLSL )

#define RES_SHADER_CUBESHADOW_VERT      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, CubeShadowMap_glsl.vert, 143, GLSL )
#define RES_SHADER_CUBESHADOW_FRAG      CINDER_RESOURCE( ../resources/DeferredResources/shaders/, CubeShadowMap_glsl.frag, 144, GLSL )

#define RES_SHADER_FXAA_VERT			CINDER_RESOURCE( ../resources/DeferredResources/shaders/, FXAA_glsl.vert, 145, GLSL )
#define RES_SHADER_FXAA_FRAG			CINDER_RESOURCE( ../resources/DeferredResources/shaders/, FXAA_glsl.frag, 146, GLSL )
/****** Deferred resources *****/

//models
#define RES_TEX_EARTH                   CINDER_RESOURCE( ../resources/, Earth_from_NASA.jpg, 147, IMAGE )






