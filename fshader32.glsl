#version 150

//---- ins
in  vec4 color;
in  vec3 fN;
in  vec3 fL;
in  vec3 fE;
in  vec2 tex_coord;

//---- outs
out vec4 FragColor;

//---- uniforms
uniform vec4 AmbientLight, DiffuseLight, SpecularLight;
uniform vec4 ka, kd, ks;
uniform float Shininess;
uniform bool light_out;
uniform bool texture_on;

uniform sampler2D myTextureSampler;


//===========================================================================
// fshader main
//===========================================================================
void
main()
{
    //---- Normalize the input lighting vectors
    vec3 N = normalize(fN);     if (light_out) N = -N;
    vec3 E = normalize(fE);
    vec3 L = normalize(fL);
    
    vec3 H = normalize( L + E );
    
    //---- get texture value
    vec4 texel = vec4(1.0, 1.0, 1.0, 1.0);
    
    if (texture_on)
        texel = texture( myTextureSampler, tex_coord );
    
    //---- Compute terms in the illumination equation
    
    vec4 ambient = AmbientLight * ka * texel;
    
    vec4 diffuse = DiffuseLight * kd * texel * max( dot(L, N), 0.0 )/(pow(length(fL),2.0));
    
    vec3 R = 2*dot(N,L)*N - L;
    vec4 specular = SpecularLight * ks * texel * pow( max(dot(R, E), 0.0), Shininess );
    // vec4 specular = SpecularLight * ks * texel * pow( max(dot(reflect(-L, N), E), 0.0), Shininess );
    // vec4 specular = SpecularLight * ks * texel * pow( max(dot(N, H), 0.0), Shininess );
    
    // if ( dot(L, N) < 0.0 ) specular = vec4(0.0, 0.0, 0.0, 1.0);
    
    
    FragColor = ambient + diffuse + specular;
    
    //FragColor = kd * max( dot(L, N), 0.0 );//ambient + diffuse + specular;
    //FragColor = vec4(fN,1.0);//ambient + diffuse + specular;
    //FragColor.a = 1.0;
    
    //---- compute the output color for this fragment
    //FragColor = color; //vec4(1.0, 0.2, 0.4, 1.0);
}