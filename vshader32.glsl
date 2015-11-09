#version 150

//---- ins
in vec4	vPosition;          // object-space vertex position.
in vec4	vColor;             // per-vertex color
in vec3 vNormal;
in vec2 vTexCoord;

//---- outs
out vec4 color;
out vec3 fN;
out vec3 fE;
out vec3 fL;
out vec2 tex_coord;

//---- uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 LightPosition;

uniform vec4 obj_color;
uniform bool obj_color_on = false;

//===========================================================================
// vshader main
//===========================================================================
void
main()
{
    //---- transform the vertex
    gl_Position = projection * view * model * vPosition;
    
    if (obj_color_on)
        color = obj_color;
    else
        color = vColor;
    
    
    fN = (view * model*vec4(vNormal, 0.0)).xyz;
    fE = -(view * model * vPosition).xyz;
    fL = (view * LightPosition).xyz - (view * model * vPosition).xyz;
    
    tex_coord    = vTexCoord;
}