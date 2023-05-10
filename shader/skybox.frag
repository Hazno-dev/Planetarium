#version 460

//
// In/Outs
//

layout (binding = 0) uniform samplerCube SkyboxTex;

layout (location = 0) out vec3 FragColor;

in vec3 Vec;

//
//Main
//

void main() {

    vec3 texColour = texture(SkyboxTex, normalize(Vec)).rgb;
    FragColor = texColour;

}