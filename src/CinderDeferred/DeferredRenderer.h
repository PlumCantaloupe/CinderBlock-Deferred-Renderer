//
//  DeferredRenderer.h
//  CinderDeferredRendering
//
//  Created by Anthony Scavarelli on 2012-12-31.
//
//

/*
 TODO:
 - Platform agnostic
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
#include "DeferredModel.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DeferredResources {
    public:
    //static shaders so they can be used anywhere
    gl::GlslProg		SHADER_SSAO;
    gl::GlslProg		SHADER_DEFERRED;
    gl::GlslProg		SHADER_BLENDER;
    gl::GlslProg		SHADER_BLUR_X;
    gl::GlslProg		SHADER_BLUR_Y;
    gl::GlslProg		SHADER_LIGHT_POINT;
    gl::GlslProg		SHADER_LIGHT_SPOT;
    gl::GlslProg		SHADER_ALPHA_TO_RBG;
    gl::GlslProg		SHADER_FXAA;
    gl::GlslProg		SHADER_DEPTH_WRITE;
    gl::GlslProg		SHADER_BASIC_TEXTURE;
    gl::GlslProg		SHADER_POINT_SHADOWS;
    gl::GlslProg		SHADER_SPOT_SHADOWS;

    gl::VboMesh         VBO_FULLSCREEN_QUAD;

    DeferredResources()
    {
        initStatics();
    }
    
    static DeferredResources& getSingleton()
    {
        static DeferredResources instance;
        return instance;
    }
    
    void initStatics()
    {
        //shaders
        SHADER_DEFERRED         = gl::GlslProg( loadResource( RES_GLSL_DEFER_VERT ), loadResource( RES_GLSL_DEFER_FRAG ) );
        SHADER_BLENDER          = gl::GlslProg( loadResource( RES_GLSL_BASIC_BLENDER_VERT ), loadResource( RES_GLSL_BASIC_BLENDER_FRAG ) );
        
        SHADER_SSAO             = gl::GlslProg( loadResource( RES_GLSL_SSAO_VERT ), loadResource( RES_GLSL_SSAO_FRAG ) );
        
        SHADER_BLUR_X           = gl::GlslProg( loadResource( RES_GLSL_BLUR_H_VERT ), loadResource( RES_GLSL_BLUR_H_FRAG ) );
        SHADER_BLUR_Y           = gl::GlslProg( loadResource( RES_GLSL_BLUR_V_VERT ), loadResource( RES_GLSL_BLUR_V_FRAG ) );
        
        SHADER_LIGHT_POINT      = gl::GlslProg( loadResource( RES_GLSL_LIGHT_POINT_VERT ), loadResource( RES_GLSL_LIGHT_POINT_FRAG ) );
        SHADER_LIGHT_SPOT       = gl::GlslProg( loadResource( RES_GLSL_LIGHT_SPOT_VERT ), loadResource( RES_GLSL_LIGHT_SPOT_FRAG ) );
        SHADER_ALPHA_TO_RBG     = gl::GlslProg( loadResource( RES_GLSL_ALPHA_RGB_VERT ), loadResource( RES_GLSL_ALPHA_RGB_FRAG ) );
        
        SHADER_POINT_SHADOWS    = gl::GlslProg( loadResource( RES_GLSL_POINTSHADOW_VERT ), loadResource( RES_GLSL_POINTSHADOW_FRAG ) );
        SHADER_SPOT_SHADOWS     = gl::GlslProg( loadResource( RES_GLSL_SPOTSHADOW_VERT ), loadResource( RES_GLSL_SPOTSHADOW_FRAG ) );
        SHADER_DEPTH_WRITE      = gl::GlslProg( loadResource( RES_GLSL_DEPTHWRITE_VERT ), loadResource( RES_GLSL_DEPTHWRITE_FRAG ) );
        SHADER_BASIC_TEXTURE    = gl::GlslProg( loadResource( RES_GLSL_BASIC_VERT ), loadResource( RES_GLSL_BASIC_FRAG ) );
        
        SHADER_FXAA             = gl::GlslProg( loadResource( RES_GLSL_FXAA_VERT ), loadResource( RES_GLSL_FXAA_FRAG ) );
        
        //mesh
        VBO_FULLSCREEN_QUAD     = DeferredModel::getFullScreenVboMesh();
    };
};

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
    
    boost::function<void()> fRenderOverlayFunc;
    boost::function<void()> fRenderParticlesFunc;
    Camera              *mCam;
    CameraOrtho         mOrthoCam;
    
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
    
    vector<Light_Point*>    mPointLights;
    vector<Light_Spot*>     mSpotLights;
    
    vector<DeferredModel*>  mModels;
    
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
        SHOW_SSAO_VIEW,
        SHOW_SSAO_BLURRED_VIEW,
        SHOW_LIGHT_VIEW,
        SHOW_SHADOWS_VIEW,
        NUM_RENDER_VIEWS
    };
    
protected:
    int mDeferFlags;
    
public:
    //using bit-shifting so multiple flags can be set simultaneously
    enum {
        SHADOWS_ENABLED_FLAG    = (1 << 0),
        SSAO_ENABLED_FLAG       = (1 << 1),
        FXAA_ENABLED_FLAG       = (1 << 2),
        MSAA_4X_ENABLED_FLAG    = (1 << 3),
        MSAA_8X_ENABLED_FLAG    = (1 << 4),
        MSAA_16X_ENABLED_FLAG   = (1 << 5)
    };
    
    enum {
        SHADER_TYPE_NONE,
        SHADER_TYPE_DEFERRED,
        SHADER_TYPE_LIGHT,
        SHADER_TYPE_SHADOW,
        SHADER_TYPE_DEPTH,
        NUM_SHADER_TYPES
    };
    
    vector<Light_Point*>* getPointLightsRef();
    const int getNumPointLights();
    vector<Light_Spot*>* getSpotLightsRef();
    const int getNumSpotLights();
    
    void setup( Camera    *cam,
                Vec2i     FBORes = Vec2i(512, 512),
                int       shadowMapRes = 512,
                int       deferFlags = SHADOWS_ENABLED_FLAG | SSAO_ENABLED_FLAG | FXAA_ENABLED_FLAG,
                const     boost::function<void()> renderOverlayFunc = NULL,
                const     boost::function<void()> renderParticlesFunc = NULL );
    
    Light_Point* addPointLight(const Vec3f position, const Color color, const float intensity, const bool castsShadows, const bool visible = false);
    Light_Spot* addSpotLight(const Vec3f position, const Vec3f target, const Color color, const float intensity, const float lightAngle, const bool castsShadows, const bool visible);
    DeferredModel* addModel( gl::VboMesh& VBOMeshRef, const DeferredMaterial mat, const BOOL isShadowsCaster, const Matrix44f modelMatrix = Matrix44f::identity() );
    void addModel( DeferredModel *model );
    
    //todo .. add remove functionality (using procedurally unique ids)
    
    void prepareDeferredScene();
    void createShadowMaps();
    void renderShadowsToFBOs();
    void renderDeferredFBO();
    void renderSSAOToFBO();
    void renderLightsToFBO();
    void pingPongBlurSSAO();
    void renderFullScreenQuad( const int renderType, const BOOL autoPrepareScene = true  );
    void renderQuad( const int renderType, Rectf renderQuad, const BOOL autoPrepareScene = true  );
    void drawLightPointMeshes(  int shaderType = SHADER_TYPE_NONE, gl::GlslProg* shader = NULL );
    void drawLightSpotMeshes(  int shaderType = SHADER_TYPE_NONE, gl::GlslProg* shader = NULL );
    void drawScene( int shaderType, gl::GlslProg *shader );
    void renderLights();
    void initTextures();
    void initFBOs();
    
    void drawModels(int shaderType, gl::GlslProg* shader, BOOL drawShadowCasters);
};