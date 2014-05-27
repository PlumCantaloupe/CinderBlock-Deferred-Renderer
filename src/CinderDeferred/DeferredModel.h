/*
 *  DeferredModel.h
 *  Container for all models for use with deferred renderer
 *
 *  Created by Anthony Scavarelli on 2014/27/05.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Matrix44.h"
#include "cinder/Quaternion.h"

#include "DeferredMaterial.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//Light Cube Class
class DeferredModel
{
    public:
    DeferredMaterial material; //can use textures to replace these properties
    
    protected:
    gl::VboMesh *mVBOMeshRef;   //keeping as pointer so VBO's can be shared if many similar instances
    Matrix44f   mModelMatrix;   //in trying to be forward thinking we will think in terms of matrices not pos, scale, rotation. Use glMultMatrixf( mModelMatrix ) if you must use immediate mode
    
    //these textures are optional but if set will overwrite any corresponding material options
    gl::Texture *diffuseTex;
    gl::Texture *specularTex;
    gl::Texture *emissiveTex;
    gl::Texture *shininessTex;
    gl::Texture *additiveSpecularTex;
    gl::Texture *normalTex;
    
    public:
    void setup( gl::VboMesh *VBOMeshRef, const DeferredMaterial mat, const Matrix44f modelMatrix = Matrix44f::identity() )
    {
        mVBOMeshRef = VBOMeshRef;
        material = mat;
        mModelMatrix = modelMatrix;
    }
    
    Matrix44f& getModelMatrix()
    {
        return mModelMatrix;
    }
    
    void setPos( const Vec3f pos )
    {
        mModelMatrix[12] = pos.x;
        mModelMatrix[12] = pos.y;
        mModelMatrix[12] = pos.z;
    }
    
    Vec3f getPos() const
    {
        return Vec3f(mModelMatrix[12], mModelMatrix[13], mModelMatrix[14]);
    }
    
    void setRotation( const Quatf rotation )
    {
        
    }
    
    Quatf getRotation() const
    {
        return Quatf();
    }
    
    void setScale( const Vec3f scale )
    {
        mModelMatrix[0] = scale.x;
        mModelMatrix[5] = scale.y;
        mModelMatrix[10] = scale.z;
    }
    
    Vec3f getScale() const
    {
        return Vec3f();
    }
    
    void setDiffuseTexture( gl::Texture *tex )
    {
        diffuseTex = tex;
    }
    
    void setSpecularTexture( gl::Texture *tex )
    {
        specularTex = tex;
    }
    
    void setEmissionTexture( gl::Texture *tex )
    {
        emissiveTex = tex;
    }
    
    void setShininessTexture( gl::Texture *tex )
    {
        shininessTex = tex;
    }
    
    void setAdditiveSpecularTexture( gl::Texture *tex )
    {
        additiveSpecularTex = tex;
    }
    
    void setNormalTexture( gl::Texture *tex )
    {
        normalTex = tex;
    }
    
    /*
     uniform vec3 diffuse;
     uniform vec3 specular;
     uniform vec3 emissive;
     uniform float shininess;
     uniform float additiveSpecular;
     */
};