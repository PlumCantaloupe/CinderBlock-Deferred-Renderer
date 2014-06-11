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
}