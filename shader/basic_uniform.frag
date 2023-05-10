#version 460

//
// In/Outs
//

layout (binding = 0) uniform sampler2D BaseColourTex;
layout (binding = 1) uniform sampler2D NormalMapTex;
layout (binding = 2) uniform sampler2D SecondaryColourTex;

layout (location = 0) out vec3 HdrColor;


in vec4 GPosition;
in vec2 GTexCoord;
in vec3 GLightsDir[3];
in vec3 GSpotlightPos;
in vec3 GSpotlightDir;
in vec3 GViewDir;

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

uniform struct FogInfo {
float MaxDist; //max distance
float MinDist; //min distance
vec3 Colour; //colour of the fog
} Fog;

uniform struct MaterialInfo{
    vec3 Ka; //Amb
    vec3 Kd; //Diff
    vec3 Ks; //Spec
    float Shininess; //Shininess
} Material;

uniform vec3 LineColour;
flat in int GIsEdge;

//
//Consts
//

const bool bMultiTexOn = false;

const bool bFogOn = false;

const bool bToonShadingOn = true;
const bool bSpecToonShadingOn = true;
const int level = 5;
const float scaleFactor = 1.0/level;

//
//Functions
//

vec3 Phong(int light, vec3 NormalCam);
vec3 BlinnPhong(int light, vec3 norm, vec3 texColour);
vec3 SpotBlinnPhong(vec3 norm, vec3 texColour);
float CalcFogFactor(vec4 PositionCam);

//
//Main
//

void main() {

    if (GIsEdge == 1){
        HdrColor = LineColour;
        return;
    }

    //TEXTURE BLENDING
    //Multiplying texture inputs is bMultiTex is true - Uses alpha channel of secondary texture to mix.
    vec3 baseColour = vec3(0.0);
    if (bMultiTexOn) {
        vec4 BC = texture(BaseColourTex, GTexCoord);
        vec4 SC = texture(SecondaryColourTex, GTexCoord);
        baseColour = mix(BC.rgb, SC.rgb, SC.a);
    } else baseColour = texture(BaseColourTex, GTexCoord).rgb;

    vec3 norm = texture(NormalMapTex, GTexCoord).xyz;
    norm.xy = 2.0 * norm.xy - 1.0;

    //LIGHTING CALCULATIONS
    //Calculate lighting nased on 3 point-lights (or directional depending on W component)
    //+ 1 spotlight using the Blinn-Phong model
    vec3 shadeColour = vec3(0.0);
    for (int i = 0; i <3; i++) shadeColour += BlinnPhong(i, normalize(norm), baseColour);
    shadeColour += SpotBlinnPhong(normalize(norm), baseColour);

    vec3 Colour = vec3(0.0);

    //FOG CALCULATIONS
    //Calculate fog factor if fog is enabled and assign a colour based on the fogFactor using mix 
    if (bFogOn) Colour = mix(Fog.Colour, shadeColour, CalcFogFactor(GPosition));
    else Colour = shadeColour;


    HdrColor = Colour;
}

// Blinn-Phong - MORE PERFORMANT
//
// Blinn-Phong shading improves performance by replacing the reflection vector calculation with the halfway vector
// This results in slightly different visual appearance but with lower computational cost
// For a directional light simply set the w component of a given light to 0.0 
//
vec3 BlinnPhong(int light, vec3 norm, vec3 texColour){

    // Calculate ambient lighting component
    vec3 ambient = Lights[light].La * Material.Ka * texColour;

    // Calculate and normalize light direction vector
    vec3 lightDir = GLightsDir[light];
    float lightDirDot = max(dot(lightDir, norm), 0.0);


    // Implement the diffuse shading calculation based on ToonShading being true/false
    vec3 diffuse = vec3(0.0);

    if (bToonShadingOn) diffuse = Lights[light].Ld * Material.Kd * (floor(lightDirDot*level)*scaleFactor) * texColour;
    else diffuse = Lights[light].Ld * Material.Kd * lightDirDot * texColour;

    // Initialize specular component
    vec3 spec = vec3(0.0);
    if (lightDirDot > 0.0){
        vec3 v = normalize(GViewDir);
        vec3 h = normalize(lightDir + v); // Calculate the halfway vector
        float specIntensity = pow(max(dot(h, norm), 0.0), Material.Shininess); // Calculate the specular intensity using halfway vector

        // Apply ToonShading to the specular component
        if (bSpecToonShadingOn) specIntensity = floor(specIntensity * level) * scaleFactor;

        spec = Lights[light].Ls * Material.Ks * specIntensity; // Calculate the specular component using the specular intensity
    }

    // Return the final shading color (ambient + diffuse + specular)
    return ambient + diffuse + spec;
}

// Adapted Blinn-Phong shading for Spotlight
//
vec3 SpotBlinnPhong(vec3 norm, vec3 texColour){

    // Calculate ambient lighting component and initialize diffuse and specular components
    vec3 ambient = Spotlight.La * Material.Ka * texColour;
    vec3 diffuse = vec3(0);
    vec3 spec = vec3(0.0);

    // Calculate and normalize light direction vector
    vec3 lightDir = normalize(GSpotlightPos);
    float cosAng = dot(-lightDir, normalize(GSpotlightDir));
    float angle = acos(cosAng);
    float spotScale = 0.0;

    // Check if the point is within the spotlight's cutoff angle
    if (angle < Spotlight.Cutoff){
        // Calculate the spotlight attenuation using the exponent term
        spotScale = pow(cosAng, Spotlight.Exponent);

        // Calculate the diffuse shading component + implement toon shading if enabled
        float lightDirDot = max(dot(lightDir, norm), 0.0);

        if (bToonShadingOn) diffuse = Spotlight.Ld * Material.Kd * (floor((lightDirDot * spotScale) * level) * scaleFactor) * texColour;
        else diffuse = Spotlight.Ld * Material.Kd * lightDirDot * spotScale * texColour;


        // Calculate the specular shading component if the point is lit
        if (lightDirDot > 0.0){
            vec3 v = normalize(GViewDir);
            vec3 h = normalize(lightDir + v); // Calculate the halfway vector
            float specIntensity = pow(max(dot(h, norm), 0.0), Material.Shininess); // Calculate the specular intensity

            // Apply ToonShading to the specular component
            if (bSpecToonShadingOn) specIntensity = floor(specIntensity * level) * scaleFactor;

            spec = Spotlight.Ls * Material.Ks * specIntensity * spotScale; // Calculate the specular component using the halfway vector and spotlight attenuation
        }
    }

    // Return the final shading color (ambient + diffuse + specular)
    return ambient + diffuse + spec;
}

// CalcFogFactor - Calculate fog based on distance to the camera
// Returns fog-factor, a value between 0 and 1 (0 being no fog, 1 being 100%)
//
float CalcFogFactor(vec4 PositionCam){

    // Calculate the abs (positive) distance between the camera and the fragment 
    float dist = abs(PositionCam.z);

    // Fog factor is calculated as max distance / distance 
    // divided by max distance / min distance
    float fogFactor = (Fog.MaxDist - dist) / (Fog.MaxDist - Fog.MinDist);

    return clamp(fogFactor, 0.0, 1.0);
}


// ---------- DEPRECATED -------------

// PHONG 
//
// Phong lighting in fragment shader is conducted on a per-pixel basis > per fragment
// Phong shading uses the reflection vector to calculate specular highlights, which can be more computationally expensive
// Only supports point lights 
//
vec3 Phong(int light, vec3 NormalCam, vec4 PositionCam){

    // Calculate ambient lighting component
    vec3 ambient = Lights[light].La * Material.Ka;

    // Calculate and normalize light direction vector
    vec3 lightDir = normalize(vec3(Lights[light].Position - (PositionCam * Lights[light].Position.w)));
    float lightDirDot = max(dot(lightDir, NormalCam), 0.0);

    // Implement the diffuse shading calculation
    vec3 diffuse = vec3(0.0);
    if (bToonShadingOn) diffuse = Lights[light].Ld * Material.Kd * (floor(lightDirDot*level)*scaleFactor);
    else diffuse = Lights[light].Ld * Material.Kd * lightDirDot;

    // Initialize specular component
    vec3 spec = vec3(0.0);
    if (lightDirDot > 0.0){
         vec3 v = normalize(-PositionCam.xyz);
        vec3 r = reflect(-lightDir, NormalCam); // Calculate the reflection vector
        float specIntensity = pow(max(dot(r,v), 0.0), Material.Shininess); // Calculate the specular intensity

        // Apply ToonShading to the specular component
        if (bSpecToonShadingOn) specIntensity = floor(specIntensity * level) * scaleFactor;

        spec = Lights[light].Ls * Material.Ks * specIntensity; // Calculate the specular component using the reflection vector
    }

    // Return the final shading color (ambient + diffuse + specular)
    return ambient + diffuse + spec;

}