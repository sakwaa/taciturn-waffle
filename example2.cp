
// =================== EXAMPLE CAMERA CODE
//
// Display a color cube, a color tube, and view them with a moving camera
//

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
using namespace std;

#include <GLUT/glut.h>

#include "Angel.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint loadBMP_custom(unsigned char** data, int* w, int* h, const char* imagepath);

//---- some variables for our camera
//----------------------------------------------------------------------------

// array of rotation angles (in degrees) for each axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Zaxis;
GLfloat  Theta1[NumAxes] = { 0.0, 0.0, 0.0 };

float r = 4.0;

mat4 view_matrix, default_view_matrix;
mat4 proj_matrix;

bool spherical_cam_on = true;


//---- cube model
//----------------------------------------------------------------------------
const int NumVerticesCube = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points_cube[NumVerticesCube];
color4 colors[NumVerticesCube];
vec3   normals[NumVerticesCube];
vec2   tex_coords[NumVerticesCube];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA colors
color4 vertex_colors[8] = {
    color4( 0.1, 0.1, 0.1, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 0.9, 0.9, 0.9, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

// quad generates two triangles for each face and assigns colors to the vertices
int Index;
void
quad_cube( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to compute its face normal
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];
    
    vec3 normal = normalize( cross(u, v) );
    
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 1.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[b]; tex_coords[Index] = vec2( 0.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[a]; tex_coords[Index] = vec2( 0.0, 1.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[c]; tex_coords[Index] = vec2( 1.0, 0.0 ); Index++;
    normals[Index] = normal; colors[Index] = vertex_colors[a]; points_cube[Index] = vertices[d]; tex_coords[Index] = vec2( 1.0, 1.0 ); Index++;
}

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
    Index = 0;
    quad_cube( 1, 0, 3, 2 );
    quad_cube( 2, 3, 7, 6 );
    quad_cube( 3, 0, 4, 7 );
    quad_cube( 6, 5, 1, 2 );
    quad_cube( 4, 5, 6, 7 );
    quad_cube( 5, 4, 0, 1 );
}


//---- cylinder model
//----------------------------------------------------------------------------
const int segments = 64;
const int NumVerticesCylinder = segments*6 + segments*3*2;
point4 points_cylinder[NumVerticesCylinder];
vec3   tnormals[NumVerticesCylinder];
point4 vertices_cylinder[4];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_cylinder( int a, int b, int c, int d )
{
    points_cylinder[Index] = vertices_cylinder[a];
    tnormals[Index] = vec3(vertices_cylinder[a].x, 0.0, vertices_cylinder[a].z); Index++;
    points_cylinder[Index] = vertices_cylinder[b];
    tnormals[Index] = vec3(vertices_cylinder[b].x, 0.0, vertices_cylinder[b].z); Index++;
    points_cylinder[Index] = vertices_cylinder[c];
    tnormals[Index] = vec3(vertices_cylinder[c].x, 0.0, vertices_cylinder[c].z); Index++;
    points_cylinder[Index] = vertices_cylinder[a];
    tnormals[Index] = vec3(vertices_cylinder[a].x, 0.0, vertices_cylinder[a].z); Index++;
    points_cylinder[Index] = vertices_cylinder[c];
    tnormals[Index] = vec3(vertices_cylinder[c].x, 0.0, vertices_cylinder[c].z); Index++;
    points_cylinder[Index] = vertices_cylinder[d];
    tnormals[Index] = vec3(vertices_cylinder[d].x, 0.0, vertices_cylinder[d].z); Index++;
}


float const bottom = -0.5;
float const top    = 0.5;

void
colortube(void)
{
    float r = 0.5;
    Index = 0;
    
    for ( int n = 0; n < segments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        points_cylinder[Index].x = 0.0;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = 0.0;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t0) * r;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = sin(t0) * r;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t1) * r;
        points_cylinder[Index].y = top;
        points_cylinder[Index].z = sin(t1) * r;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, 1.0, 0.0);
        Index++;
    }
    
    
    int num;
    for ( int n = 0; n < segments; n++ )
    {
        num = 0;
        float x = 0.0, y = 0.0, r = 0.5;
        
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        //quad vertex 0
        vertices_cylinder[num].x = cos(t0) * r;
        vertices_cylinder[num].y = bottom;
        vertices_cylinder[num].z = sin(t0) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 1
        vertices_cylinder[num].x = cos(t1) * r;
        vertices_cylinder[num].y = bottom;
        vertices_cylinder[num].z = sin(t1) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 2
        vertices_cylinder[num].x = cos(t1) * r;
        vertices_cylinder[num].y = top;
        vertices_cylinder[num].z = sin(t1) * r;
        vertices_cylinder[num++].w = 1.0;
        //quad vertex 3
        vertices_cylinder[num].x = cos(t0) * r;
        vertices_cylinder[num].y = top;
        vertices_cylinder[num].z = sin(t0) * r;
        vertices_cylinder[num++].w = 1.0;
        
        quad_cylinder( 0, 1, 2, 3 );
    }
    
    for ( int n = 0; n < segments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)segments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)segments;
        
        points_cylinder[Index].x = 0.0;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = 0.0;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t1) * r;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = sin(t1) * r;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
        points_cylinder[Index].x = cos(t0) * r;
        points_cylinder[Index].y = bottom;
        points_cylinder[Index].z = sin(t0) * r;
        points_cylinder[Index].w = 1.0;
        tnormals[Index] = vec3(0.0, -1.0, 0.0);
        Index++;
    }
    
}


//---- sphere model
//----------------------------------------------------------------------------
const int ssegments = 64;
const int NumVerticesSphere = ssegments*6*(ssegments-2) + ssegments*3*2;
point4 points_sphere[NumVerticesSphere];
vec3   bnormals[NumVerticesSphere];
point4 vertices_sphere[4];
vec2   stex_coords[NumVerticesSphere];

// quad generates two triangles for each face and assigns colors to the vertices
void
quad_sphere( int a, int b, int c, int d, float t0, float t1, float p0, float p1 )
{
    points_sphere[Index] = vertices_sphere[a];
    bnormals[Index] = vec3(vertices_sphere[a].x, vertices_sphere[a].y, vertices_sphere[a].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p1-M_PI/2.0)/M_PI);
    Index++;
    
    points_sphere[Index] = vertices_sphere[b];
    bnormals[Index] = vec3(vertices_sphere[b].x, vertices_sphere[b].y, vertices_sphere[b].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p1-M_PI/2.0)/M_PI);
    Index++;
    
    points_sphere[Index] = vertices_sphere[c];
    bnormals[Index] = vec3(vertices_sphere[c].x, vertices_sphere[c].y, vertices_sphere[c].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p0-M_PI/2.0)/M_PI);
    Index++;
    
    points_sphere[Index] = vertices_sphere[a];
    bnormals[Index] = vec3(vertices_sphere[a].x, vertices_sphere[a].y, vertices_sphere[a].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p1-M_PI/2.0)/M_PI);
    Index++;
    
    points_sphere[Index] = vertices_sphere[c];
    bnormals[Index] = vec3(vertices_sphere[c].x, vertices_sphere[c].y, vertices_sphere[c].z);
    stex_coords[Index] = vec2( t1/(2*M_PI), -(p1-M_PI/2.0)/M_PI);
    Index++;
    
    points_sphere[Index] = vertices_sphere[d];
    bnormals[Index] = vec3(vertices_sphere[d].x, vertices_sphere[d].y, vertices_sphere[d].z);
    stex_coords[Index] = vec2( t0/(2*M_PI), -(p0-M_PI/2.0)/M_PI);
    Index++;
}


void
colorbube(void)
{
    float r = 0.5;
    Index = 0;
    float ph_top = (float)((ssegments/2)-1)/(float)(segments/2) * M_PI/2.0;
    float ph_bottom = -ph_top;
    
    for ( int n = 0; n < ssegments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
        
        points_sphere[Index].x = 0.0;
        points_sphere[Index].y = top;
        points_sphere[Index].z = 0.0;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2(0.0, 0.0);
        Index++;
        points_sphere[Index].x = cos(ph_top) * cos(t0) * r;
        points_sphere[Index].y = sin(ph_top) * r;
        points_sphere[Index].z = cos(ph_top) * sin(t0) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2(t0/(2*M_PI), -(ph_top-M_PI/2.0)/M_PI);
        Index++;
        points_sphere[Index].x = cos(ph_top) * cos(t1) * r;
        points_sphere[Index].y = sin(ph_top) * r;
        points_sphere[Index].z = cos(ph_top) * sin(t1) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        stex_coords[Index] = vec2(t1/(2*M_PI), -(ph_top-M_PI/2.0)/M_PI);
        Index++;
    }
    
    for( int m = 1; m < ssegments-1; m++ )
    {
        float p0 = M_PI/2.0 - m * M_PI/(float)ssegments;
        float p1 = M_PI/2.0 - (m+1) * M_PI/(float)ssegments;
        
        int num;
        for ( int n = 0; n < ssegments; n++ )
        {
            num = 0;
            float x = 0.0,
                y = 0.0,
                r = 0.5;
            
            GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
            GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
            
            //quad vertex 0
            vertices_sphere[num].x = cos(p1)*cos(t0) * r;
            vertices_sphere[num].y = sin(p1) * r;
            vertices_sphere[num].z = cos(p1)*sin(t0) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 1
            vertices_sphere[num].x = cos(p1)*cos(t1) * r;
            vertices_sphere[num].y = sin(p1) * r;
            vertices_sphere[num].z = cos(p1)*sin(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 2
            vertices_sphere[num].x = cos(p0)*cos(t1) * r;
            vertices_sphere[num].y = sin(p0) * r;
            vertices_sphere[num].z = cos(p0)*sin(t1) * r;
            vertices_sphere[num++].w = 1.0;
            //quad vertex 3
            vertices_sphere[num].x = cos(p0)*cos(t0) * r;
            vertices_sphere[num].y = sin(p0) * r;
            vertices_sphere[num].z = cos(p0)*sin(t0) * r;
            vertices_sphere[num++].w = 1.0;
            
            quad_sphere( 0, 1, 2, 3, t0, t1, p0, p1 );
        }
    }
    
    for ( int n = 0; n < ssegments; n++ )
    {
        GLfloat const t0 = 2 * M_PI * (float)n / (float)ssegments;
        GLfloat const t1 = 2 * M_PI * (float)(n+1) / (float)ssegments;
        
        points_sphere[Index].x = 0.0;
        points_sphere[Index].y = bottom;
        points_sphere[Index].z = 0.0;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        Index++;
        points_sphere[Index].x = cos(ph_bottom) * cos(t0) * r;
        points_sphere[Index].y = sin(ph_bottom) * r;
        points_sphere[Index].z = cos(ph_bottom) * sin(t0) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        Index++;
        points_sphere[Index].x = cos(ph_bottom) * cos(t1) * r;
        points_sphere[Index].y = sin(ph_bottom) * r;
        points_sphere[Index].z = cos(ph_bottom) * sin(t1) * r;
        points_sphere[Index].w = 1.0;
        bnormals[Index] = vec3(points_sphere[Index].x, points_sphere[Index].y, points_sphere[Index].z);
        Index++;
    }
    
}


//---- OpenGL initialization

GLuint program;
GLuint vPosition;
GLuint vColor;
GLuint vNormal;
GLuint vTexCoord;
GLuint vSphereCoord;

GLuint textures[3];


size_t CUBE_OFFSET;
size_t CUBE_N_OFFSET;

size_t CYLINDER_OFFSET;
size_t CYLINDER_N_OFFSET;

size_t SPHERE_OFFSET;
size_t SPHERE_N_OFFSET;

size_t CUBE_TEX_OFFSET;

size_t SPHERE_TEX_OFFSET;


void
init()
{
    // Load shaders and use the resulting shader program
    //program = InitShader( "vshader21.glsl", "fshader21.glsl" );
    program = InitShader( "vshader32.glsl", "fshader32.glsl" );
    glUseProgram( program );
    
    //---------------------------------------------------------------------
    colorcube();
    colortube();
    colorbube();
    
    //----- generate a checkerboard pattern for texture mapping
    const int  TextureSize  = 1024;
    GLubyte checker_tex[TextureSize][TextureSize][3];
    
    for ( int i = 0; i < TextureSize; i++ )
    {
        for ( int j = 0; j < TextureSize; j++ )
        {
            // GLubyte c = ((1) ^ ((i  & (j | 0x8))  == 0)) * 255;
            // GLubyte c = (j & i) ^ (i | j);
            GLubyte c = (((int) pow((double) i, 3.0)) ^ j);
            checker_tex[i][j][0]  = c;
            checker_tex[i][j][1]  = c;
            checker_tex[i][j][2]  = c;
        }
    }
    
    //---- Initialize texture objects
    glGenTextures( 2, textures );
    
    glActiveTexture( GL_TEXTURE0 );
    
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, TextureSize, TextureSize, 0, GL_BGR, GL_UNSIGNED_BYTE, checker_tex );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    //---- bmp texture
    unsigned char* data;
    int w,h;
    loadBMP_custom(&data, &w, &h, "bmp-texture.bmp");
    
    // glGenTextures( 1, textures );
    
    glActiveTexture( GL_TEXTURE0 );
    
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, data );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    //---- bmp texture
    unsigned char* data2;
    int w2,h2;
    loadBMP_custom(&data2, &w2, &h2, "logo.bmp");
    
    // glGenTextures( 1, textures );
    
    glActiveTexture( GL_TEXTURE0 );
    
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w2, h2, 0, GL_BGR, GL_UNSIGNED_BYTE, data2 );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    
    //---------------------------------------------------------------------
    
    //----set offset variables
    
    CUBE_OFFSET = 0;
    CUBE_N_OFFSET = sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere);
    
    CYLINDER_OFFSET = sizeof(points_cube) + sizeof(colors);
    CYLINDER_N_OFFSET = sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere) + sizeof(normals);
    
    SPHERE_OFFSET = sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder);
    SPHERE_N_OFFSET = sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere) + sizeof(normals) + sizeof(tnormals);
    
    CUBE_TEX_OFFSET = SPHERE_N_OFFSET + sizeof(bnormals);
    SPHERE_TEX_OFFSET = CUBE_TEX_OFFSET + sizeof(tex_coords);
    
    
    //---- Create a vertex array object
    
    GLuint vao;
    //glGenVertexArraysAPPLE( 1, &vao );
    //glBindVertexArrayAPPLE( vao );
    glGenVertexArrays( 1, &vao );  // removed 'APPLE' suffix for 3.2
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points_cube) + sizeof(colors) +
                 sizeof(points_cylinder) + sizeof(points_sphere)+
                 sizeof(normals) + sizeof(tnormals) + sizeof(bnormals) + sizeof(tex_coords) + sizeof(stex_coords),
                 NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points_cube), points_cube );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points_cube), sizeof(colors), colors );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_OFFSET, sizeof(points_cylinder), points_cylinder );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_OFFSET, sizeof(points_sphere), points_sphere );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_N_OFFSET, sizeof(normals), normals );
    glBufferSubData( GL_ARRAY_BUFFER, CYLINDER_N_OFFSET, sizeof(tnormals), tnormals );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_N_OFFSET, sizeof(bnormals), bnormals );
    glBufferSubData( GL_ARRAY_BUFFER, CUBE_TEX_OFFSET, sizeof(tex_coords), tex_coords );
    glBufferSubData( GL_ARRAY_BUFFER, SPHERE_TEX_OFFSET, sizeof(stex_coords), stex_coords );
    
    //---------------------------------------------------------------------
    
    // set up vertex arrays
    vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    
    vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube)) );
    
    vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder) + sizeof(points_sphere)) );
    
    vTexCoord = glGetAttribLocation( program, "vTexCoord" );
    glEnableVertexAttribArray( vTexCoord );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_TEX_OFFSET) );
    
    // Set the value of the fragment shader texture sampler variable (myTextureSampler) to GL_TEXTURE0
    glUniform1i( glGetUniformLocation(program, "myTextureSampler"), 0 );
    
    //---------------------------------------------------------------------
    
    //---- setup initial view
    float tr_y = Theta1[Yaxis]* M_PI/180.0;
    float tr_z = Theta1[Zaxis]* M_PI/180.0;
    float eye_x = r * (cos(tr_z)) * cos(tr_y);
    float eye_z = r * (cos(tr_z)) * sin(tr_y);
    float eye_y = r * sin(tr_z);
    
    view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, cos(tr_z), 0.0, 0.0));
    default_view_matrix = view_matrix;
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.8, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
SetMaterial( vec4 ka, vec4 kd, vec4 ks, float s )
{
    glUniform4fv( glGetUniformLocation(program, "ka"), 1, ka );
    glUniform4fv( glGetUniformLocation(program, "kd"), 1, kd );
    glUniform4fv( glGetUniformLocation(program, "ks"), 1, ks );
    glUniform1f( glGetUniformLocation(program, "Shininess"), s );
}

//----------------------------------------------------------------------------

void
SetLight( vec4 lpos, vec4 la, vec4 ld, vec4 ls )
{
    glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1, lpos);
    glUniform4fv( glGetUniformLocation(program, "AmbientLight"), 1, la);
    glUniform4fv( glGetUniformLocation(program, "DiffuseLight"), 1, ld);
    glUniform4fv( glGetUniformLocation(program, "SpecularLight"), 1, ls);
}


//----------------------------------------------------------------------------

float rotx = 0.0;

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //---- camera
    
    float left = -1.0;
    float right = 1.0;
    float bottom = -1.0;
    float top = 1.0;
    float zNear = 1.0;
    float zFar = 10.0;
    
    proj_matrix = Frustum( left, right, bottom, top, zNear, zFar );
    GLuint proj = glGetUniformLocation( program, "projection" );
    glUniformMatrix4fv( proj, 1, GL_TRUE, proj_matrix );
    
    if (spherical_cam_on)
    {
        float tr_y = Theta1[Yaxis]* M_PI/180.0;
        float tr_z = Theta1[Zaxis]* M_PI/180.0;
        //float eye_x = r * cos(tr_y);
        //float eye_z = r * sin(tr_y);
        float eye_x = r * (cos(tr_z)) * cos(tr_y);
        float eye_z = r * (cos(tr_z)) * sin(tr_y);
        float eye_y = r * sin(tr_z);
        
        vec4 up = vec4(0.0, cos(tr_z), 0.0, 0.0);
        cout << up << ' ' << normalize(up) << endl;
        
        view_matrix = LookAt( vec4(eye_x, eye_y, eye_z, 1.0), vec4(0.0, 0.0, 0.0, 1.0), vec4(0.0, cos(tr_z), 0.0, 0.0));
    }
    
    cout << view_matrix << endl;
    
    GLuint view = glGetUniformLocation( program, "view" );
    glUniformMatrix4fv( view, 1, GL_TRUE, view_matrix );
    
    //---- action
    
    //---- lamp post 1
    
    //---- metal stand
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5);
    
    mat4 transform_tube = Translate( 1.0, 0.5, 2.0 ) * Scale(0.02, 1.0, 0.02);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_tube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    transform_tube = Translate( 1.0, 0.05, 2.0 ) * Scale(0.5, 0.1, 0.5);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_tube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    
    //---- spherical light
    SetLight( vec4( 1.0, 1.17, 2.0, 1.0 ), vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    
    SetMaterial( vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), true );
    
    mat4 transform_bube = Translate( 1.0, 1.0, 2.0 ) * Scale(0.3, 0.3, 0.3);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_bube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    //----- floor 1
    glBindTexture( GL_TEXTURE_2D, textures[1] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    SetLight( vec4( 1.0, 1.17, 2.0, 1.0 ), vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    SetMaterial( vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), vec4(0.9, 0.9, 0.9, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    glUniform1i( glGetUniformLocation(program, "noise_on"), false );
    
    mat4 model_matrix = Translate( 0.0, 0.0, 5.0 ) * Scale(10.0, 0.01, 10.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model_matrix );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_N_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_TEX_OFFSET) );

    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
    
    //---- rotating cube
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    SetMaterial( vec4(0.9, 0.0, 0.0, 1.0), vec4(0.9, 0.0, 0.0, 1.0), vec4(1.0, 0.4, 0.4, 1.0), 5.0);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    glUniform1i( glGetUniformLocation(program, "noise_on"), false );
    
    model_matrix = Translate( 0.0, 0.25, 2.0 ) * RotateY(rotx) * Scale(0.5, 0.5, 0.5);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model_matrix );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );
    
    
    
    
    
    //---- lamp post 2
    
    //---- metal stand
    SetMaterial( vec4(0.1, 0.1, 0.1, 1.0), vec4(0.1, 0.1, 0.1, 1.0), vec4(0.3, 0.3, 0.3, 1.0), 0.5);
    
    transform_tube = Translate( 1.0, 0.5, -6.0 ) * Scale(0.02, 1.0, 0.02);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_tube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    transform_tube = Translate( 1.0, 0.05, -6.0 ) * Scale(0.5, 0.1, 0.5);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_tube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CYLINDER_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCylinder );
    
    
    //---- spherical light
    SetLight( vec4( 1.0, 1.17, -6.0, 1.0 ), vec4(0.4, 0.4, 0.4, 1.0), vec4(0.7, 0.7, 0.7, 1.0), vec4(0.7, 0.7, 0.7, 1.0) );
    
    SetMaterial( vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), true );
    
    transform_bube = Translate( 1.0, 1.0, -6.0 ) * Scale(0.3, 0.3, 0.3);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_bube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    
    //----- floor 2
    
    SetMaterial( vec4(0.0, 0.6, 0.3, 1.0), vec4(0.0, 0.6, 0.3, 1.0), vec4(0.0, 0.6, 0.3, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    model_matrix = Translate( 0.0, 0.0, -9.0 ) * Scale(10.0, 0.01, 10.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model_matrix );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );
    
    
    
    //---- gravel path
    
    SetMaterial( vec4(0.99, 0.93, 0.6, 1.0), vec4(0.99, 0.93, 0.6, 1.0), vec4(0.0, 0.0, 0.0, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    
    model_matrix = Translate( 0.0, 0.0, -2.0 ) * Scale(10.0, 0.01, 4.0);
    
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, model_matrix );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(CUBE_N_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesCube );

    
    //---- Sphere
    glBindTexture( GL_TEXTURE_2D, textures[2] );
    SetMaterial( vec4(1.0, 1.0, 1.0, 1.0), vec4(0.9, 0.9, 0.7, 1.0), vec4(0.9, 0.9, 0.7, 1.0), 0.5);
    glUniform1i( glGetUniformLocation(program, "light_out"), false );
    glUniform1i( glGetUniformLocation(program, "noise_on"), false );
    glUniform1i( glGetUniformLocation(program, "texture_on"), true );
    
    transform_bube = Translate( 0.0, 1.0, 0.0 ) * Scale(10.0, 10.0, 10.0);
    glUniformMatrix4fv( glGetUniformLocation( program, "model" ), 1, GL_TRUE, transform_bube );
    
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_OFFSET) );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_N_OFFSET) );
    glVertexAttribPointer( vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(SPHERE_TEX_OFFSET) );
    glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    glUniform1i( glGetUniformLocation(program, "texture_on"), false );

    
#if 0
    //---- spheres
    
    for ( int i = 0; i < 100; i++ )
    {
        mat4 transform_bube = Translate( rand()%10-5, rand()%10-5, rand()%10-5 ) * Scale(0.3, 0.7, 0.3);
        
        mat4 model = glGetUniformLocation( program, "model" );
        glUniformMatrix4fv( model, 1, GL_TRUE, transform_bube );
        
        glUniform1i( glGetUniformLocation(program, "obj_color_on"), true );
        glUniform4fv( glGetUniformLocation(program, "obj_color"), 1, vec4(0.8, 0.0, 0.0, 1.0) );
        
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points_cube) + sizeof(colors) + sizeof(points_cylinder)) );
        glDrawArrays( GL_TRIANGLES, 0, NumVerticesSphere );
    }
    
#endif
    
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033:    //---- escape key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
            
        case 'u':
            view_matrix = default_view_matrix;
            Theta1[Xaxis] = 0.0;
            Theta1[Yaxis] = 0.0;
            Theta1[Zaxis] = 0.0;
            glutPostRedisplay();
            break;
            
            //---- cases for spherical viewer
        case 's':    //---- spherical cam on
            Theta1[Xaxis] = 0.0;
            Theta1[Yaxis] = 0.0;
            Theta1[Zaxis] = 0.0;
            spherical_cam_on = true;
            glutPostRedisplay();
            break;
        case 'S':    //---- spherical cam off
            spherical_cam_on = false;
            glutPostRedisplay();
            break;
            
            
        case 'y':    //---- theta
            Axis = Yaxis;
            Theta1[Axis] += 5.0;
            Theta1[Axis] = fmod(Theta1[Axis], 360.0);
            glutPostRedisplay();
            break;
        case 'z':    //---- phi
            Axis = Zaxis;
            Theta1[Axis] += 5.0;
            Theta1[Axis] = fmod(Theta1[Axis], 360.0);
            glutPostRedisplay();
            break;
        case 'r':    //---- increase radius
            r += 0.5;
            glutPostRedisplay();
            break;
        case 'R':    //---- decrease radius
            r -= 0.5;
            glutPostRedisplay();
            break;
            
            //---- cases for flying viewer
            
        case 'f':    //---- forward
            view_matrix = Translate(0.0, 0.0, 0.2) * view_matrix;
            glutPostRedisplay();
            break;
        case 'b':    //---- back
            view_matrix = Translate(0.0, 0.0, -0.2) * view_matrix;
            glutPostRedisplay();
            break;
        case 'j':    //---- pan left
            view_matrix = RotateY(-1.0) * view_matrix;
            glutPostRedisplay();
            break;
        case 'k':    //---- pan right
            view_matrix = RotateY(1.0) * view_matrix;
            glutPostRedisplay();
            break;
            
    }
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( state == GLUT_DOWN ) {
        switch( button ) {
            case GLUT_LEFT_BUTTON:    Axis = Xaxis;  break;
            case GLUT_MIDDLE_BUTTON:  Axis = Yaxis;  break;
            case GLUT_RIGHT_BUTTON:   Axis = Zaxis;  break;
        }
    }
}

//----------------------------------------------------------------------------

void
idle( void )
{
    rotx = fmod(rotx + 1.0, 360.0);
    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
reshape( int w, int h )
{
    //glViewport(0,0,w,h);
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    //glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 800, 800 );
    glutCreateWindow( "Color Cube" );
    
    init();
    
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );
    glutIdleFunc( idle );
    
    glutMainLoop();
    return 0;
}
