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
    DeferredMaterial()
    {}
    
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