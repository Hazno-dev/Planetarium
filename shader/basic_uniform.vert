#version 460

//
// In/Outs
//

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec4 VertexTangent;

out vec4 Position;
out vec2 TexCoord;
out vec3 LightsDir[3];
out vec3 SpotlightPos;
out vec3 SpotlightDir;
out vec3 ViewDir;

//
//Uniforms
//

uniform struct LightInfo {
    vec4 Position;
    vec3 La; //Amb
    vec3 Ld; //Diff
    vec3 Ls; //Spec
} Lights[3];

uniform struct SpotLightInfo {
    vec3 Position;
    vec3 La;
    vec3 Ld;
    vec3 Ls;
    vec3 Direction;
    float Exponent;
    float Cutoff;
} Spotlight;

uniform bool bSinWaveAnim = false;
uniform float Time;
uniform float Frequency = 2.5;
uniform float Velocity = 2.5;
uniform float Amplitude = 0.15;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ProjectionMatrix;

//
//Main
//

// Main: Transforms the normals into tangent space.
// Calculates the point lights and spotlight position/direction in tangent space.
// These values are sent to the geometry shader and then to the fragment shader.
void main()
{
    // Convert vertex position to camera space
    Position = (ModelViewMatrix * vec4(VertexPosition,1.0));
    
    // SinWave animation if enabled 
    vec4 VecPos = vec4(VertexPosition, 1.0);
    vec4 VecNormal = vec4(VertexNormal, 1.0);
    
    if (bSinWaveAnim){
        float u = Frequency * VecPos.x - Velocity * Time;
        VecPos.y += Amplitude * sin(u);

        float u1 = Frequency * VecNormal.x - Velocity * Time;
        VecNormal.y += Amplitude * sin(u1);
    }
    
    // Calculate and normalize the transformed vertex normal
    vec3 Normal = normalize(NormalMatrix * VecNormal.xyz);
    vec3 tang = normalize(NormalMatrix * vec3(VertexTangent));

    // Calculate and normalize the binormal vector
    vec3 binormal = normalize(cross(Normal,tang)) * VertexTangent.w;

    // Construct the matrix for transformation to tangent space
    mat3 toObjectLocal = mat3(tang.x, binormal.x, Normal.x, tang.y, binormal.y, Normal.y, tang.z, binormal.z, Normal.z );

    vec3 pos = vec3( ModelViewMatrix * VecPos );

    // Calculate light directions in tangent space for each light
    for (int i = 0; i < 3; i++) {
        vec3 lightVec = (Lights[i].Position.xyz - (pos * Lights[i].Position.w));
        if (Lights[i].Position.w != 0.0) {
            lightVec = normalize(lightVec);
        }
        LightsDir[i] = toObjectLocal * lightVec;
    }

    SpotlightPos = toObjectLocal * (Spotlight.Position.xyz - pos);
    SpotlightDir = toObjectLocal * Spotlight.Direction;

    ViewDir = toObjectLocal * normalize(-pos);

    TexCoord = VertexTexCoord;
    gl_Position = MVP * VecPos;
}

// ------------------ NOTE ---------------
//
//Lighting calculations are conducted in the fragment shader instead of the vertex shader as to avoid interpolation across vertices (gouraud shading)
//and instead perform lighting calculations on a per-pixel basis for a more accurate result.
//However the old implementation of vertex-based phong lighting can be viewed below as mentioned in the marking rubric:
//
//----------------------------------------
//
//vec3 phong(vec3 normCamS, vec4 vertexPosCamS){
//
//    vec3 ambient = Light.La * Material.Ka;
//
//    //normalize vertex pos to light pos
//    vec3 lightDir = normalize(vec3(Light.Position - vertexPosCamS));
//    float lightDirDot = max(dot(lightDir, normCamS), 0.0);
//
//    // Implement the diffuse shading calculation
//    vec3 diffuse = Light.Ld * Material.Kd * lightDirDot;
//
//    vec3 spec = vec3(0.0);
//    if (lightDirDot > 0.0){
//        vec3 v = normalize(-vertexPosCamS.xyz);
//        vec3 r = reflect(-lightDir, normCamS);
//        spec = Light.Ls * Material.Ks * pow(max(dot(r,v), 0.0), Material.Shininess);
//    }
//
//    return ambient + diffuse + spec;
//}
//
//void getCamSpaceValues ( out vec3 norm, out vec4 position )
//{
//    norm = normalize( NormalMatrix * VertexNormal);
//    position = (ModelViewMatrix * vec4(VertexPosition,1.0));
//}