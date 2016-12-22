[[FX]]

// Samplers
sampler2D albedoMap = sampler_state
{
    Texture = "textures/common/white.tga";
};

// Uniforms
float4 matDiffuseCol <
string desc_abc = "abc: diffuse color";
string desc_d   = "d: alpha for opacity";
> = {1.0, 1.0, 1.0, 1.0};

float4 matSpecParams <
string desc_abc = "abc: specular color";
string desc_d   = "d: gloss";
> = {0.04, 0.04, 0.04, 0.5};

// Contexts
context BACKGROUND
{
    VertexShader = compile GLSL VS_BACKGROUND;
    PixelShader = compile GLSL FS_BACKGROUND;
    BlendMode = Blend;
    AlphaToCoverage = true;
}

[[VS_BACKGROUND]]

uniform mat4 viewProjMat;
uniform mat4 worldMat;
uniform mat4 projMat;
attribute vec3 vertPos;
attribute vec2 texCoords0;
varying vec4 pos;
varying vec2 texCoords;

void main( void )
{
    // Calculate world space position
    pos = worldMat * vec4( vertPos, 1.0 );
    // Calculate texture coordinates and clip space position
    texCoords = texCoords0;
    gl_Position = projMat * vec4( vertPos, 1.0 );
}


[[FS_BACKGROUND]]
precision mediump float;

uniform mediump vec3 viewerPos;
uniform mediump vec4 matSpecParams;
uniform sampler2D albedoMap;
varying mediump vec4 pos;
varying mediump vec2 texCoords;

void main( void )
{
    mediump vec3 newCoords = vec3( texCoords, 0 );
    // Flip texture vertically to match the GL coordinate system
    newCoords.t = -newCoords.t;
    mediump vec4 albedo = texture2D( albedoMap, newCoords.st );
    mediump vec3 newPos = pos.xyz;
    mediump vec4 nalbedo = albedo;
    gl_FragColor = nalbedo;
}
