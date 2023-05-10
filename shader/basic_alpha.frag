#version 460

//
// In/Outs
//

layout (binding = 0) uniform sampler2D baseTex;
layout (binding = 1) uniform sampler2D alphaTex;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

//
//Main
//

void main() {

    vec3 baseColour = texture(baseTex, TexCoord).rgb;
    vec4 alphaMap = texture(alphaTex, TexCoord);

    if (alphaMap.a < 0.15) discard;
    else FragColor = vec4(baseColour, 1.0);
}