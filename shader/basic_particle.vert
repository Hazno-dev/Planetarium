#version 460

//
// In/Outs
//

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexVelocity;
layout (location = 2) in float VertexAge;

out vec3 Position;
out vec3 Velocity;
out float Age;

out vec2 TexCoord;
out float Transparency;

//
//Uniforms
//

uniform int Pass;

uniform float Time;
uniform float DeltaTime;
uniform vec3 Acceleration; //Gravity Direc
uniform float ParticleLifetime;
uniform vec3 EmitterPosition = vec3(0.0);
uniform mat3 EmitterBasis;
uniform float ParticleSize = 0.5;

uniform bool bVariableSize = false;
uniform float MinParticleSize = 1.0;
uniform float MaxParticleSize = 1.5;

uniform mat4 MV;
uniform mat4 Projection;

uniform sampler1D RandomTex;

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
							 
const float PI = 3.14159265359;

//
//Functions
//

vec3 RandomInitialVelocty();
vec3 RandomInitialPosition();
void update();
void render();

//
//Main
//

void main()
{
	if (Pass == 1) update();
	else render();
}

vec3 RandomInitialVelocty() {
	float theta = mix(0, PI/8, texelFetch(RandomTex, 3 * gl_VertexID, 0).r);
	float phi = mix(0, 2 * PI, texelFetch(RandomTex, 3 * gl_VertexID + 1, 0).r);
	float velocity = mix(1.25, 1.5, texelFetch(RandomTex, 3 * gl_VertexID + 2, 0).r);
	vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
	return normalize(EmitterBasis * v) * velocity;
}

// This implementation of RandomInitialPosition worked, but created unnatural looking particle distributions.
// Left in to show that it was implemented.
//
//vec3 RandomInitialVelocty() {
//	float velocity = mix(0.1,0.5, texelFetch(RandomTex, 2 * gl_VertexID, 0).r);
//	return EmitterBasis * vec3(velocity, velocity, velocity);
//}

vec3 RandomInitialPosition() {
	float offset = mix(-0.1, 0.1, texelFetch(RandomTex, 2 * gl_VertexID + 1, 0).r);
	return EmitterPosition + vec3(offset, 0, 0);
}

void update() {
	if (VertexAge < 0 || VertexAge > ParticleLifetime) {
		Position  = RandomInitialPosition();
		Velocity = RandomInitialVelocty();
		if (VertexAge < 0) Age = VertexAge + DeltaTime;
		else Age = (VertexAge - ParticleLifetime) + DeltaTime;
	} else {
		Position = VertexPosition + VertexVelocity * DeltaTime;
		Velocity = VertexVelocity + Acceleration * DeltaTime;
		Age = VertexAge + DeltaTime;
	}
}

void render() {
	Transparency = 0;
	vec3 posCam = vec3(0);
	if (VertexAge >= 0.0) {
	    float agePct = VertexAge / ParticleLifetime;
		Transparency = clamp(0.8 - agePct, 0.0, 0.8);
		if (!bVariableSize) posCam = (MV*vec4(VertexPosition,1)).xyz + offsets[gl_VertexID] * ParticleSize;
		else posCam = (MV * vec4(VertexPosition, 1)).xyz + offsets[gl_VertexID] * mix(MinParticleSize, MaxParticleSize, agePct);
		
	}
	TexCoord = texCoords[gl_VertexID];
	gl_Position = Projection * vec4(posCam, 1.0);
}