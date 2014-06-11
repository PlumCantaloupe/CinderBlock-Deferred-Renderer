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
    BOOL _isShadowCaster;
    
    gl::VboMesh mVBOMeshRef;   //keeping as pointer so VBO's can be shared if many similar instances
    Matrix44f   mModelMatrix;   //in trying to be forward thinking we will think in terms of matrices not pos, scale, rotation. Use glMultMatrixf( mModelMatrix ) if you must use immediate mode
    
    //these textures are optional but if set will overwrite any corresponding material options
    gl::Texture *diffuseTex;
    gl::Texture *specularTex;
    gl::Texture *emissiveTex;
    gl::Texture *shininessTex;
    gl::Texture *additiveSpecularTex;
    gl::Texture *normalTex;
    
    public:
    DeferredModel()
    {
        diffuseTex = NULL;
        specularTex = NULL;
        emissiveTex = NULL;
        shininessTex = NULL;
        additiveSpecularTex = NULL;
        normalTex = NULL;
    }
    
    void setup( gl::VboMesh VBOMeshRef, const DeferredMaterial mat, const BOOL isShadowsCaster = true, const Matrix44f modelMatrix = Matrix44f::identity() )
    {
        mVBOMeshRef = VBOMeshRef;
        material = mat;
        _isShadowCaster = isShadowsCaster;
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
        //TODO
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
    
    void render()
    {
        gl::draw( mVBOMeshRef );
    }
    
    /*
     uniform vec3 diffuse;
     uniform vec3 specular;
     uniform vec3 emissive;
     uniform float shininess;
     uniform float additiveSpecular;
     */
    
#pragma mark - static VBO primitive functions
    
    static gl::VboMesh getFullScreenVboMesh()
    {
        vector<uint32_t> indices;
        vector<Vec3f> normals;
        vector<Vec3f> positions;
        vector<Vec2f> texCoords;
        
        positions.push_back( Vec3f(-1, -1, 0) );   //left top
        positions.push_back( Vec3f(-1, 1, 0) );    //left bottom
        positions.push_back( Vec3f(1, -1, 0) );    //right top
        positions.push_back( Vec3f(1, 1, 0) );     //right bottom
        
        normals.push_back( Vec3f(0,0,-1) );
        normals.push_back( Vec3f(0,0,-1) );
        normals.push_back( Vec3f(0,0,-1) );
        normals.push_back( Vec3f(0,0,-1) );
        
        indices.push_back( 0 );
        indices.push_back( 1 );
        indices.push_back( 2 );
        indices.push_back( 2 );
        indices.push_back( 1 );
        indices.push_back( 3 );
        
        texCoords.push_back( Vec2f(0,0) );
        texCoords.push_back( Vec2f(0,1) );
        texCoords.push_back( Vec2f(1,0) );
        texCoords.push_back( Vec2f(1,1) );
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        layout.setStaticTexCoords2d();
        
        gl::VboMesh vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
        vboMesh.bufferPositions( positions );
        vboMesh.bufferNormals( normals );
        vboMesh.bufferIndices( indices );
        vboMesh.bufferTexCoords2d( 0, texCoords );
        
        indices.clear();
        normals.clear();
        positions.clear();
        texCoords.clear();
        
        return vboMesh;
    }
    
    static gl::VboMesh getPlaneVboMesh( const Vec3f &c, const float size )
    {
        vector<uint32_t> indices;
        vector<Vec3f> normals;
        vector<Vec3f> positions;
        vector<Vec2f> texCoords;
        
        positions.push_back( c + Vec3f(-size/2, 0, -size/2) );  //left top
        positions.push_back( c + Vec3f(-size/2, 0, size/2) );   //left bottom
        positions.push_back( c + Vec3f(size/2, 0, -size/2) );   //right top
        positions.push_back( c + Vec3f(size/2, 0, size/2) );    //right bottom
        
        normals.push_back( Vec3f(0,1,0) );
        normals.push_back( Vec3f(0,1,0) );
        normals.push_back( Vec3f(0,1,0) );
        normals.push_back( Vec3f(0,1,0) );
        
        indices.push_back( 0 );
        indices.push_back( 1 );
        indices.push_back( 2 );
        indices.push_back( 2 );
        indices.push_back( 1 );
        indices.push_back( 3 );
        
        texCoords.push_back( Vec2f(0,0) );
        texCoords.push_back( Vec2f(0,1) );
        texCoords.push_back( Vec2f(1,0) );
        texCoords.push_back( Vec2f(1,1) );
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        layout.setStaticTexCoords2d();
        
        gl::VboMesh vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
        vboMesh.bufferPositions( positions );
        vboMesh.bufferNormals( normals );
        vboMesh.bufferIndices( indices );
        vboMesh.bufferTexCoords2d( 0, texCoords );
        
        indices.clear();
        normals.clear();
        positions.clear();
        texCoords.clear();
        
        return vboMesh;
    }
    
    static gl::VboMesh getCubeVboMesh( const Vec3f &c, const Vec3f &size )
    {
        float sx = size.x * 0.5f;
        float sy = size.y * 0.5f;
        float sz = size.z * 0.5f;
        Vec3f vertices[24]={
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),    Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	// +X
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	// +Y
            Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),     Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// +Z
            Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// -X
            Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+1.0f*sz),	// -Y
            Vec3f(c.x+1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+-1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+-1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz),	Vec3f(c.x+1.0f*sx,c.y+1.0f*sy,c.z+-1.0f*sz)     // -Z
        };
        
        Vec3f normals[24]={ Vec3f(1,0,0),   Vec3f(1,0,0),   Vec3f(1,0,0),   Vec3f(1,0,0),
                            Vec3f(0,1,0),	Vec3f(0,1,0),	Vec3f(0,1,0),	Vec3f(0,1,0),
                            Vec3f(0,0,1),	Vec3f(0,0,1),	Vec3f(0,0,1),	Vec3f(0,0,1),
                            Vec3f(-1,0,0),	Vec3f(-1,0,0),	Vec3f(-1,0,0),	Vec3f(-1,0,0),
                            Vec3f(0,-1,0),	Vec3f(0,-1,0),  Vec3f(0,-1,0),  Vec3f(0,-1,0),
                            Vec3f(0,0,-1),	Vec3f(0,0,-1),	Vec3f(0,0,-1),	Vec3f(0,0,-1)
        };
        
        uint32_t indices[6*6] = {   0, 1, 2, 0, 2, 3,
                                    4, 5, 6, 4, 6, 7,
                                    8, 9,10, 8, 10,11,
                                    12,13,14,12,14,15,
                                    16,17,18,16,18,19,
                                    20,21,22,20,22,23
        };
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        
        gl::VboMesh vboMesh = gl::VboMesh( 24, 36, layout, GL_TRIANGLES );
        vboMesh.bufferPositions(std::vector<Vec3f>(vertices, vertices + sizeof(vertices)/sizeof(vertices[0])));
        vboMesh.bufferNormals(std::vector<Vec3f>(normals, normals + sizeof(normals)/sizeof(normals[0])));
        vboMesh.bufferIndices(std::vector<uint32_t>(indices, indices + sizeof(indices)/sizeof(indices[0])));
        
        return vboMesh;
    }
    
    //modfied from Stephen Schieberl's MeshHelper class https://github.com/bantherewind/Cinder-MeshHelper
    static gl::VboMesh getSphereVboMesh( const Vec3f &center, const float radius, const Vec2i resolution )
    {
        vector<uint32_t> indices;
        vector<Vec3f> normals;
        vector<Vec3f> positions;
        vector<Vec2f> texCoords;
        
        float step = (float)M_PI / (float)resolution.y;
        float delta = ((float)M_PI * 2.0f) / (float)resolution.x;
        
        int32_t p = 0;
        for ( float phi = 0.0f; p <= resolution.y; p++, phi += step ) {
            int32_t t = 0;
            
            uint32_t a = (uint32_t)( ( p + 0 ) * resolution.x );
            uint32_t b = (uint32_t)( ( p + 1 ) * resolution.x );
            
            for ( float theta = delta; t < resolution.x; t++, theta += delta ) {
                float sinPhi = math<float>::sin( phi );
                float x = sinPhi * math<float>::cos( theta );
                float y = sinPhi * math<float>::sin( theta );
                float z = -math<float>::cos( phi );
                Vec3f position( x, y, z );
                position = (position * radius) + center;
                Vec3f normal = (position - center).normalized();
                Vec2f texCoord = ( normal.xy() + Vec2f::one() ) * 0.5f;
                
                normals.push_back( normal );
                positions.push_back( position );
                texCoords.push_back( texCoord );
                
                uint32_t n = (uint32_t)( t + 1 >= resolution.x ? 0 : t + 1 );
                indices.push_back( a + t );
                indices.push_back( b + t );
                indices.push_back( a + n );
                indices.push_back( a + n );
                indices.push_back( b + t );
                indices.push_back( b + n );
            }
        }
        
        for ( vector<uint32_t>::iterator iter = indices.begin(); iter != indices.end(); ) {
            if ( *iter < positions.size() ) {
                ++iter;
            } else {
                iter = indices.erase( iter );
            }
        }
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        layout.setStaticTexCoords2d();
        
        gl::VboMesh vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
        vboMesh.bufferPositions( positions );
        vboMesh.bufferNormals( normals );
        vboMesh.bufferIndices( indices );
        vboMesh.bufferTexCoords2d( 0, texCoords );
        
        indices.clear();
        normals.clear();
        positions.clear();
        texCoords.clear();
        
        return vboMesh;
    }
    
    //ogre3D implementation
    static gl::VboMesh getConeVboMesh( const Vec3f &pointPos, const float &coneHeight, const float &coneRadius, const int numSegments )
    {
        vector<uint32_t> indices;
        vector<Vec3f> normals;
        vector<Vec3f> positions;
        
        //Positions : cone head and base
        positions.push_back( pointPos + Vec3f(0.0f, 0.0f, 0.0f) );
        normals.push_back( Vec3f(0, 1, 0) );
        
        //Base :
        Vec3f basePoint = Vec3f( pointPos.x, coneHeight, pointPos.z );
        float fDeltaBaseAngle = (2 * M_PI) / numSegments;
        for (int i=0; i<numSegments; i++)
        {
            float angle = i * fDeltaBaseAngle;
            Vec3f vertPos = pointPos + Vec3f(coneRadius * cosf(angle), -coneHeight, coneRadius * sinf(angle));
            positions.push_back( vertPos );
            normals.push_back( (vertPos - basePoint).normalized() );
        }
        
        //Indices :
        //Cone head to vertices
        for (int i=0; i<numSegments; i++)
        {
            indices.push_back( 0 );
            indices.push_back( (i%numSegments) + 1 );
            indices.push_back( ((i+1)%numSegments) + 1 );
        }
        //Cone base
        for (int i=0; i<numSegments-2; i++)
        {
            indices.push_back( 1 );
            indices.push_back( i+3 );
            indices.push_back( i+2 );
        }
        
        gl::VboMesh::Layout layout;
        layout.setStaticPositions();
        layout.setStaticIndices();
        layout.setStaticNormals();
        
        gl::VboMesh vboMesh = gl::VboMesh( positions.size(), indices.size(), layout, GL_TRIANGLES );
        vboMesh.bufferPositions( positions );
        vboMesh.bufferNormals( normals );
        vboMesh.bufferIndices( indices );
        
        indices.clear();
        normals.clear();
        positions.clear();
        
        return vboMesh;
    }
};