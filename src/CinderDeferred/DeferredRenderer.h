//
//  DeferredRenderer.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

/*
 TODO:
 - Want framework independence
 - VboMesh Class
 - Fbo Class
 - Shader Class
 - Conversion function (e.g. Vec3f to float[3] and vice versa)
 */

#pragma once

#include "Resources.h"

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/ImageIo.h"
#include "cinder/Camera.h"

#include "boost/function.hpp"
#include "boost/bind.hpp"
#include "boost/lambda/lambda.hpp"

#include "Lights.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DeferredRenderer
{
public:
    enum {
        FBO_DEFERRED,
        FBO_SSAO,
        FBO_PINGPONG_H,
        FBO_PINGPONG_V,
        FBO_LIGHTS,
        FBO_SHADOWS,
        FBO_COMPOSITED,
        FBO_OVERLAY,
        FBO_PARTICLES
    };
    
    boost::function<void(gl::GlslProg*)> fRenderShadowCastersFunc;
    boost::function<void(gl::GlslProg*)> fRenderNotShadowCastersFunc;
    boost::function<void()> fRenderOverlayFunc;
    boost::function<void()> fRenderParticlesFunc;
    Camera              *mCam;
    
    Matrix44f           mLightFaceViewMatrices[6];
	
    gl::Texture			mRandomNoise;
    
    gl::Fbo				mDeferredFBO;
    gl::Fbo				mSSAOMap;
    gl::Fbo				mPingPongBlurH;
    gl::Fbo				mPingPongBlurV;
    gl::Fbo				mLightGlowFBO;
    gl::Fbo				mAllShadowsFBO;
	gl::Fbo				mFinalSSFBO;
    gl::Fbo				mOverlayFBO;
    gl::Fbo				mParticlesFBO;
    
	gl::GlslProg		mPointShadowShader;
    gl::GlslProg		mSpotShadowShader;
	
    gl::GlslProg		mSSAOShader;
    gl::GlslProg		mDeferredShader;
    gl::GlslProg		mBasicBlender;
    gl::GlslProg		mHBlurShader;
    gl::GlslProg		mVBlurShader;
    gl::GlslProg		mLightPointShader;
    gl::GlslProg		mAlphaToRBG;
	gl::GlslProg		mFXAAShader;
    
    vector<Light_Point*>    mPointLights;
    vector<Light_Spot*>     mSpotLights;
    
    Vec2i               mFBOResolution;
    int                 mShadowMapRes;
    
    gl::VboMesh         mCubeVBOMesh;    //pass cube Vbo to all point lights to save on draw calls
    gl::VboMesh         mConeVBOMesh;    //pass cube Vbo to all spot lights to save on draw calls
    gl::VboMesh         mSphereVBOMesh;
    
    enum
    {
        SHOW_FINAL_VIEW,
        SHOW_DIFFUSE_VIEW,
        SHOW_NORMALMAP_VIEW,
        SHOW_DEPTH_VIEW,
        SHOW_POSITION_VIEW,
        SHOW_ATTRIBUTE_VIEW,
        SHOW_SSAO_VIEW,
        SHOW_SSAO_BLURRED_VIEW,
        SHOW_LIGHT_VIEW,
        SHOW_SHADOWS_VIEW,
        NUM_RENDER_VIEWS
    };
    
private:
    BOOL    mUseSSAO;
    BOOL    mUseShadows;
    BOOL    mUseFXAA;
    
public:
    vector<Light_Point*>* getPointLightsRef();
    const int getNumPointLights();
    vector<Light_Spot*>* getSpotLightsRef();
    const int getNumSpotLights();
    
    void setup( const boost::function<void(gl::GlslProg*)> renderShadowCastFunc,
               const boost::function<void(gl::GlslProg*)> renderObjFunc,
               const boost::function<void()> renderOverlayFunc,
               const boost::function<void()> renderParticlesFunc,
               Camera    *cam,
               Vec2i     FBORes = Vec2i(512, 512),
               int       shadowMapRes = 512,
               BOOL      useSSAO = true,
               BOOL      useShadows = true,
               BOOL      useFXAA = true);
    
    Light_Point* addPointLight(const Vec3f position, const Color color, const float intensity, const bool castsShadows = false, const bool visible = false);
    Light_Spot* addSpotLight(const Vec3f position, const Vec3f target, const Color color, const bool castsShadows = false, const bool visible = true);
    void prepareDeferredScene();
    void createShadowMaps();
    void renderShadowsToFBOs();
    void renderDeferredFBO();
    void renderSSAOToFBO();
    void renderLightsToFBO();
    void pingPongBlurSSAO();
    void renderFullScreenQuad( const int renderType, const BOOL autoPrepareScene = true  );
    void renderQuad( const int renderType, Rectf renderQuad, const BOOL autoPrepareScene = true  );
    void drawLightMeshes(gl::GlslProg* shader = NULL, BOOL deferShaderUsed = false);
    void drawScene();
    void renderLights();
    void initTextures();
    void initShaders();
    void initFBOs();
    
#pragma mark - static VBO primitive functions
    
    static void getCubeVboMesh( gl::VboMesh *vboMesh, const Vec3f &c, const Vec3f &size );
    
    //modfied from Stephen Schieberl's MeshHelper class https://github.com/bantherewind/Cinder-MeshHelper
    static void getSphereVboMesh( gl::VboMesh *vboMesh, const Vec3f &center, const float radius, const Vec2i resolution = Vec2i(6, 6) );
    
    //ogre3D implementation
    static void getConeVboMesh( gl::VboMesh *vboMesh, const Vec3f &pointPos, const float &coneHeight, const float &coneRadius, const int numSegments );
    
};