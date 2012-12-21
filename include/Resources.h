#pragma once
#include "cinder/CinderResources.h"

//images
#define NOISE_SAMPLER		CINDER_RESOURCE( ../resources/textures/, random.png, 128, IMAGE )

//shaders
#define SSAO_VERT			CINDER_RESOURCE( ../resources/shaders/, SSAO_glsl.vert, 129, GLSL )
#define SSAO_FRAG           CINDER_RESOURCE( ../resources/shaders/, SSAO_glsl.frag, 130, GLSL )
#define SSAO_FRAG_LIGHT		CINDER_RESOURCE( ../resources/shaders/, SSAOL_glsl.frag, 131, GLSL )

#define DEFER_VERT			CINDER_RESOURCE( ../resources/shaders/, Deferred_glsl.vert, 132, GLSL )
#define DEFER_FRAG			CINDER_RESOURCE( ../resources/shaders/, Deferred_glsl.frag, 133, GLSL )

#define BBlender_VERT		CINDER_RESOURCE( ../resources/shaders/, BasicBlender_glsl.vert, 134, GLSL )
#define BBlender_FRAG		CINDER_RESOURCE( ../resources/shaders/, BasicBlender_glsl.frag, 135, GLSL )

#define BLUR_H_VERT			CINDER_RESOURCE( ../resources/shaders/, Blur_h_glsl.vert, 136, GLSL )
#define BLUR_H_FRAG			CINDER_RESOURCE( ../resources/shaders/, Blur_h_glsl.frag, 137, GLSL )

#define BLUR_V_VERT			CINDER_RESOURCE( ../resources/shaders/, Blur_v_glsl.vert, 138, GLSL )
#define BLUR_V_FRAG			CINDER_RESOURCE( ../resources/shaders/, Blur_v_glsl.frag, 139, GLSL )

#define LIGHT_VERT          CINDER_RESOURCE( ../resources/shaders/, Light_glsl.vert, 140, GLSL )
#define LIGHT_FRAG          CINDER_RESOURCE( ../resources/shaders/, Light_glsl.frag, 141, GLSL )

#define ALPHA_RGB_VERT      CINDER_RESOURCE( ../resources/shaders/, AlphaToRGB_glsl.vert, 142, GLSL )
#define ALPHA_RGB_FRAG      CINDER_RESOURCE( ../resources/shaders/, AlphaToRGB_glsl.frag, 143, GLSL )

#define RES_SHADER_CUBESHADOW_VERT      CINDER_RESOURCE( ../resources/shaders/, CubeShadowMap_glsl.vert, 144, GLSL )
#define RES_SHADER_CUBESHADOW_FRAG      CINDER_RESOURCE( ../resources/shaders/, CubeShadowMap_glsl.frag, 145, GLSL )


