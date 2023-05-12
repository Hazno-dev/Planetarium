#version 460

//
// In/Outs
//

layout (location = 0) in vec3 VertexInitVelocity; 
layout (location = 1) in float VertexBirthTime;

out float Transparency;
out vec2 TexCoord;

//
//Uniforms
//

uniform float Time;
uniform vec3 Gravity = vec3(0.0, -0.05, 0.0);
uniform float ParticleLifetime;
uniform float ParticleSize = 1.0;
uniform vec3 EmitterPos;

uniform mat4 MV;
uniform mat4 Projection;

//
//Consts
//

const vec3 offsets[] = vec3[](vec3(-0.5, -0.5, 0.0),
							  vec3( 0.5, -0.5, 0.0),
							  vec3( 0.5,  0.5, 0.0),
							  vec3(-0.5,  -0.5, 0.0), 
							  vec3( 0.5,  0.5, 0.0),
							  vec3(-0.5,  0.5, 0.0));
							  
const vec2 texCoords[] = vec2[](vec2(0.0, 0.0),
								vec2(1.0, 0.0),
								vec2(1.0, 1.0),
								vec2(0.0, 0.0),
								vec2(1.0, 1.0),
								vec2(0.0, 1.0));
							  

//
//Main
//

void main()
{
	vec3 CameraPos;
	float t = Time - VertexBirthTime;
	if ( t >= 0 && t < ParticleLifetime) {
		vec3 pos = EmitterPos + VertexInitVelocity * t + Gravity * t * t;
		
		CameraPos = (MV * vec4(pos, 1.0)).xyz + (offsets[gl_VertexID] * ParticleSize);
		Transparency = mix(1, 0, t / ParticleLifetime);
	} else {
		//Dead particle 
		Transparency = 0;
		CameraPos = vec3(0.0, 0.0, 0.0);
	}
	
	TexCoord = texCoords[gl_VertexID];
	gl_Position = Projection * vec4(CameraPos, 1.0);
}
