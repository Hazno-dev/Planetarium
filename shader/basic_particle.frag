#version 460

//
// In/Outs
//

in float Transparency;
in vec2 TexCoord;
uniform sampler2D ParticleTex;

layout (location = 0) out vec4 FragColor;

//
//Main
//

void main() {
	vec4 texColor = texture(ParticleTex, TexCoord);
	
	FragColor = texColor;
	FragColor.a *= Transparency;
}