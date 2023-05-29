#version 460

//
// In/Outs
//

layout (binding = 0) uniform sampler2D HdrTex;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

//
//Uniforms
//

uniform int Width;
uniform int Height;
uniform float Radius;
uniform sampler2D NoiseTex;

//
//Functions
//

float luminance (vec3 colour);

//
//Main
//

void main() {
     // Retrieve high-res color from texture
     vec4 colour = texture( HdrTex, TexCoord );
     vec4 noise = texture( NoiseTex, TexCoord );
     float green = luminance (colour.rgb);
     
     // Screen borders 
     float dist1 = length(gl_FragCoord.xy - vec2(Width / 4, Height / 2));
     float dist2 = length(gl_FragCoord.xy - vec2(3 * Width / 4, Height / 2));
     if (dist1 > Radius && dist2 > Radius ) green = 0.0;
     
     vec4 result = vec4(0.0, green * clamp (noise.a, 0.0, 1.0), 0.0, 1.0);
     
     FragColor =  vec4( pow( result.rgb, vec3(1.0/1.4) ), 1.0 );
}

float luminance (vec3 colour) {
    return dot (colour.rgb, vec3(0.2126, 0.7152, 0.0722));
}