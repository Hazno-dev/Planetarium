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
	
	//Black mixture as the particle gets old
	FragColor = vec4(mix(vec3(0,0,0), texColor.rgb, Transparency), texColor.a);
	FragColor.a *= Transparency;
}