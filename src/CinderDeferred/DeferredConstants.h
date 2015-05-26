#pragma once

#include "cinder/app/AppBasic.h"

#include "DeferredMaterial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace DeferredConstants {
    static const float  LIGHT_BRIGHTNESS_DEFAULT = 1.0f;    //brightness of lights
    
    //some commonly used base materials (TODO)
    static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_RED             = DeferredMaterial( Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), 10.0f, 10.0f );
    static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_GREEN           = DeferredMaterial( Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), 10.0f, 10.0f );
    static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_BLUE            = DeferredMaterial( Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), 10.0f, 10.0f );
    static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_METAL_ROUGH     = DeferredMaterial( Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), 10.0f, 10.0f );
    static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_METAL_SMOOTH    = DeferredMaterial( Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0), 10.0f, 10.0f );
    
    typedef enum
    {
        SHOW_FINAL_VIEW,
        SHOW_DIFFUSE_VIEW,
        SHOW_NORMALMAP_VIEW,
        SHOW_DEPTH_VIEW,
        SHOW_SSAO_VIEW,
        SHOW_SSAO_BLURRED_VIEW,
        SHOW_LIGHT_VIEW,
        SHOW_SHADOWS_VIEW,
        NUM_RENDER_VIEWS
    } DEFERRED_RENDER_MODE;
}