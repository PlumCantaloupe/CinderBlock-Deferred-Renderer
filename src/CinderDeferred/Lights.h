/*
 *  Lights.h
 *  Light Classes
 *
 *  Created by Anthony Scavarelli on 2014/20/05.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/Camera.h"
#include "cinder/Matrix44.h"

#include "CubeShadowMap.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//Light Cube Class
class Light_Point
{
    
public:
    CameraPersp                 mShadowCam;
    DeferredMaps::CubeShadowMap mShadowMap;
    gl::Fbo                     mDepthFBO;
    gl::Fbo                     mShadowsFbo;
    
    Matrix44f                   modelMatrix;
    Matrix44f                   modelMatrixAOE;
    //float                       modelMatrix[16];   //scale/translate matrix
    //float                       modelMatrixAOE[16];    //scale/translate matrix
    
private:
    Color                       mCol;
    float                       mIntensity;
    float                       mMaskRadius;
    bool                        mCastShadows;
    bool                        mProxyVisible;
    int                         mShadowMapRes;
    
    gl::VboMesh                 *mVBOMeshRef;
    
public:
	Light_Point(gl::VboMesh* vboMeshRef, Vec3f pos, Color col, float intensity, int shadowMapRes, BOOL castsShadows = false, BOOL proxyVisible = false)
    {
        mVBOMeshRef = vboMeshRef;
        mCol = col;
        mIntensity = intensity;
        mMaskRadius = intensity * 100.0f;
        mShadowMapRes = shadowMapRes;
        mCastShadows = castsShadows;
        mProxyVisible = proxyVisible;
        
        //create matrices
        float modelScale = 1.0f;
        modelMatrix[0] = modelScale;        modelMatrix[4] = 0.0f;              modelMatrix[8] = 0.0f;              modelMatrix[12] = pos.x;
        modelMatrix[1] = 0.0f;              modelMatrix[5] = modelScale;        modelMatrix[9] = 0.0f;              modelMatrix[13] = pos.y;
        modelMatrix[2] = 0.0f;              modelMatrix[6] = 0.0f;              modelMatrix[10] = modelScale;       modelMatrix[14] = pos.z;
        modelMatrix[3] = 0.0f;              modelMatrix[7] = 0.0f;              modelMatrix[11] = 0.0f;             modelMatrix[15] = 1.0f;
        
        modelMatrixAOE[0] = mMaskRadius;    modelMatrixAOE[4] = 0.0f;          modelMatrixAOE[8] = 0.0f;            modelMatrixAOE[12] = pos.x;
        modelMatrixAOE[1] = 0.0f;           modelMatrixAOE[5] = mMaskRadius;   modelMatrixAOE[9] = 0.0f;            modelMatrixAOE[13] = pos.y;
        modelMatrixAOE[2] = 0.0f;           modelMatrixAOE[6] = 0.0f;          modelMatrixAOE[10] = mMaskRadius;    modelMatrixAOE[14] = pos.z;
        modelMatrixAOE[3] = 0.0f;           modelMatrixAOE[7] = 0.0f;          modelMatrixAOE[11] = 0.0f;           modelMatrixAOE[15] = 1.0f;
        
        //now set up shadow parameters
        if (mCastShadows) {
            setUpShadowStuff();
        }
        
    }
    
    void updateShadowCam()
    {
        if(mCastShadows) {
            Vec3f eye = getPos();
            Vec3f target = Vec3f(eye.x, 0.0f, eye.z);
            mShadowCam.setPerspective( 90.0f, 1.0f, 1.0f, 100.0f );
            mShadowCam.lookAt( eye, target );
            mShadowCam.setCenterOfInterestPoint(target);
        }
    }
    
    void setUpShadowStuff()
    {
        //set up cube map for point shadows
        mShadowMap.setup( mShadowMapRes );
        
        //create FBO to hold depth values from cube map
        gl::Fbo::Format formatShadow;
        formatShadow.enableColorBuffer(false);
        formatShadow.enableDepthBuffer(true, true);
        formatShadow.setMinFilter(GL_LINEAR);
        formatShadow.setMagFilter(GL_LINEAR);
        formatShadow.setWrap(GL_CLAMP, GL_CLAMP);
        mDepthFBO   = gl::Fbo( mShadowMapRes, mShadowMapRes, formatShadow);
        
        gl::Fbo::Format format;
        format.setDepthInternalFormat( GL_DEPTH_COMPONENT24 );
        format.setColorInternalFormat( GL_RGBA8 );
        //format.setSamples( 4 ); // enable 4x antialiasing
        mShadowsFbo	= gl::Fbo( mShadowMapRes, mShadowMapRes, format );
        
        updateShadowCam();
    }
    
	void setPos(const Vec3f pos)
    {
        mShadowCam.lookAt( pos, Vec3f( pos.x, 0.0f, pos.z ) );
        modelMatrix[12] = pos.x;        modelMatrix[13] = pos.y;        modelMatrix[14] = pos.z;
        modelMatrixAOE[12] = pos.x;     modelMatrixAOE[13] = pos.y;     modelMatrixAOE[14] = pos.z;
        
        updateShadowCam();
    }
    
    Vec3f getPos() const
    {
        return Vec3f(modelMatrix[12], modelMatrix[13], modelMatrix[14]);
    }
    
	void setCol(const Color col)
    {
        mCol = col;
    }
    
    Color getColor() const
    {
        return mCol;
    }
    
    void setIntensity( const float intensity )
    {
        mIntensity = intensity;
    }
    
    float getIntensity() const
    {
        return mIntensity;
    }
    
    void setLightMaskRadius( const float radius )
    {
        mMaskRadius = radius;
    }
    
    float getLightMaskRadius() const
    {
        return mMaskRadius;
    }
    
	void renderProxy() const
    {
        if( mProxyVisible ) {
            gl::draw(*mVBOMeshRef);
        }
    }
    
    void renderProxyAOE() const
    {
        gl::draw(*mVBOMeshRef);
    }
    
    bool doesCastShadows() const {
        return mCastShadows;
    }
};

class Light_Spot
{
    
public:
    CameraPersp                 mShadowCam;
    DeferredMaps::CubeShadowMap mShadowMap;
    gl::Fbo                     mDepthFBO;
    gl::Fbo                     mShadowsFbo;
    
    Matrix44f                   modelMatrix;
    Matrix44f                   modelMatrixAOE;
    //float                       modelMatrix[16];   //scale/translate matrix
    //float                       modelMatrixAOE[16];    //scale/translate matrix
    
private:
    Color                       mCol;
    float                       mIntensity;
    float                       mLightAngle;
    float                       mMaskRadius;
    bool                        mCastShadows;
    bool                        mProxyVisible;
    int                         mShadowMapRes;
    Vec3f                       mTarget;
    
    gl::VboMesh                 *mVBOMeshRef;
    
public:
	Light_Spot(gl::VboMesh* vboMeshRef, Vec3f pos, Vec3f target, Color col, float intensity, float lightAngle, int shadowMapRes, BOOL castsShadows = false, BOOL proxyVisible = false)
    {
        mVBOMeshRef = vboMeshRef;
        mCol = col;
        mIntensity = intensity;
        mMaskRadius = intensity * 100.0f;
        mTarget = target;
        mLightAngle = lightAngle;
        mShadowMapRes = shadowMapRes;
        mCastShadows = castsShadows;
        mProxyVisible = proxyVisible;
        
        //create matrices
        float modelScale = 1.0f;
        modelMatrix[0] = modelScale;        modelMatrix[4] = 0.0f;              modelMatrix[8] = 0.0f;              modelMatrix[12] = pos.x;
        modelMatrix[1] = 0.0f;              modelMatrix[5] = modelScale;        modelMatrix[9] = 0.0f;              modelMatrix[13] = pos.y;
        modelMatrix[2] = 0.0f;              modelMatrix[6] = 0.0f;              modelMatrix[10] = modelScale;       modelMatrix[14] = pos.z;
        modelMatrix[3] = 0.0f;              modelMatrix[7] = 0.0f;              modelMatrix[11] = 0.0f;             modelMatrix[15] = 1.0f;
        
        modelMatrixAOE[0] = mMaskRadius;    modelMatrixAOE[4] = 0.0f;          modelMatrixAOE[8] = 0.0f;            modelMatrixAOE[12] = pos.x;
        modelMatrixAOE[1] = 0.0f;           modelMatrixAOE[5] = mMaskRadius;   modelMatrixAOE[9] = 0.0f;            modelMatrixAOE[13] = pos.y;
        modelMatrixAOE[2] = 0.0f;           modelMatrixAOE[6] = 0.0f;          modelMatrixAOE[10] = mMaskRadius;    modelMatrixAOE[14] = pos.z;
        modelMatrixAOE[3] = 0.0f;           modelMatrixAOE[7] = 0.0f;          modelMatrixAOE[11] = 0.0f;           modelMatrixAOE[15] = 1.0f;
        
        //now set up shadow parameters
        if (mCastShadows) {
            setUpShadowStuff();
        }
        
    }
    
    void setUpShadowStuff()
    {
        //set up cube map for point shadows
        mShadowMap.setup( mShadowMapRes );
        
        //create FBO to hold depth values from cube map
        gl::Fbo::Format formatShadow;
        formatShadow.enableColorBuffer(false);
        formatShadow.enableDepthBuffer(true, true);
        formatShadow.setDepthInternalFormat( GL_DEPTH_COMPONENT24 );
        formatShadow.setMinFilter(GL_LINEAR);
        formatShadow.setMagFilter(GL_LINEAR);
        formatShadow.setWrap(GL_CLAMP, GL_CLAMP);
        mDepthFBO   = gl::Fbo( mShadowMapRes, mShadowMapRes, formatShadow);
        
        gl::Fbo::Format format;
        format.enableDepthBuffer(true, false);
        format.setColorInternalFormat( GL_RGBA8 );
        mShadowsFbo	= gl::Fbo( mShadowMapRes, mShadowMapRes, format );
        
        updateShadowCam();
    }
    
	void setPos(const Vec3f pos)
    {
        modelMatrix[12] = pos.x;        modelMatrix[13] = pos.y;        modelMatrix[14] = pos.z;
        modelMatrixAOE[12] = pos.x;     modelMatrixAOE[13] = pos.y;     modelMatrixAOE[14] = pos.z;
        
        updateShadowCam();
    }
    
    Vec3f getPos() const
    {
        return Vec3f(modelMatrix[12], modelMatrix[13], modelMatrix[14]);
    }
    
    void setTarget(const Vec3f pos)
    {
        mTarget = pos;
        updateShadowCam();
    }
    
    void updateShadowCam()
    {
        if(mCastShadows) {
            mShadowCam.setPerspective( 90.0f, 1.0f, 1.0f, 100.0f );
            mShadowCam.lookAt( getPos(), mTarget );
            mShadowCam.setCenterOfInterestPoint(mTarget);
        }
    }
    
    Vec3f getTarget() const
    {
        return mTarget;
    }
    
    Vec3f getLightDirection() const
    {
        return (mTarget - getPos()).normalized();
    }
    
	void setCol(const Color col)
    {
        mCol = col;
    }
    
    Color getColor() const
    {
        return mCol;
    }
    
    void setIntensity( const float intensity )
    {
        mIntensity = intensity;
    }
    
    float getIntensity() const
    {
        return mIntensity;
    }
    
    void setLightMaskRadius( const float radius )
    {
        mMaskRadius = radius;
    }
    
    float getLightMaskRadius() const
    {
        return mMaskRadius;
    }
    
    void setLightAngle(const float angle)
    {
        mLightAngle = angle;
    }
    
    float getLightAngle() const
    {
        return mLightAngle;
    }
    
	void renderProxy() const
    {
        if( mProxyVisible ) {
            gl::draw(*mVBOMeshRef);
        }
    }
    
    void renderProxyAOE() const
    {
        gl::draw(*mVBOMeshRef);
    }
    
    bool doesCastShadows() const {
        return mCastShadows;
    }
};