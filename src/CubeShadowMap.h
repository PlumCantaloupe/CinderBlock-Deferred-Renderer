/*
 *  CubeShadowMap.h
 *  Point Lights
 *
 *  Created by Anthony Scavarelli on 2012/12/15. 
 *  Based on David Wicks CubeMapping sample in Cinder_0.8.4
 *
 */

#ifndef CinderCubeMapShadows_CubeShadowMap_h
#define CinderCubeMapShadows_CubeShadowMap_h

#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using ci::Surface8u;
using boost::shared_ptr;

class CubeShadowMap
{
public:
    enum
    {
        X_FACE_POS,
        X_FACE_NEG,
        Y_FACE_POS,
        Y_FACE_NEG,
        Z_FACE_POS,
        Z_FACE_NEG
    };
    
	unsigned int textureObject;
    
public:
	//this should be overloaded or generalized to allow different types of texture inputs
    CubeShadowMap()
    {}
    
	void setup( GLsizei texSize )
    {
        //create a texture object
        glGenTextures(1, &textureObject);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
        
        //parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_DEPTH_COMPONENT, texSize, texSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    
	void bind( const int loc = 0)
    {
        //glActiveTexture(GL_TEXTURE0 + loc );
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, textureObject);
    }
    
    void bindDepthFB( const int face )
    {
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureObject, 0);
		//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, tex_depth_cube, 0);
    }

	void unbind( const int loc = 0 )
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, loc );
    }
};

#endif
