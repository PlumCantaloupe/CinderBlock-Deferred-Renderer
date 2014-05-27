/*
 *  DeferredModel.h
 *  Container for all models for use with deferred renderer
 *
 *  Created by Anthony Scavarelli on 2014/27/05.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct DeferredMaterial
{
    DeferredMaterial(const Color diffuse, const Color specular, const Color emissive, const float shiney, const float addSpec)
    {
        diffuseCol = diffuse;
        specularCol = specular;
        emissiveCol = emissive;
        shininess = shiney;
        additiveSpecular = addSpec;
    }
    
    Color diffuseCol;
    Color specularCol;
    Color emissiveCol;
    float shininess;
    float additiveSpecular;
};

//some commonly used base materials (WIP)
static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_RED             = DeferredMaterial( Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), 0.0f, 0.0f );
static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_GREEN           = DeferredMaterial( Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), 0.0f, 0.0f );
static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_BLUE            = DeferredMaterial( Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), 0.0f, 0.0f );
static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_METAL_ROUGH     = DeferredMaterial( Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), 0.0f, 0.0f );
static const DeferredMaterial DEFERRED_MATERIAL_PLASTIC_METAL_SMOOTH    = DeferredMaterial( Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), Color(1.0, 0.0, 0.0), 0.0f, 0.0f );