#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/plane.h"
#include "helper/objmesh.h"
#include "helper/skybox.h"

class SceneBasic_Uniform : public Scene
{
public:
    SceneBasic_Uniform();

    // Public functions and overrides
    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void setupCamera();
    void updateCamera(GLFWwindow* window);

private:
    // GLSL programs
    GLSLProgram prog;
    GLSLProgram Alphaprog;
    GLSLProgram Skyboxprog;
    GLSLProgram HDRprog;

    // HDR Variables
    GLuint quad;
    GLuint hdrFBO;
    GLuint hdrTex, avgTex;

    // Scene Mesh Information
    // ----------------------
    // Meshes and objects
    Plane plane;
    std::unique_ptr<ObjMesh> CrystalMesh;
    std::unique_ptr<ObjMesh> Planet1Mesh;
    std::unique_ptr<ObjMesh> Planet2Mesh;
    std::unique_ptr<ObjMesh> MoonMesh;
    std::unique_ptr<ObjMesh> RingMesh;

    // Object and Light Locations
    glm::vec3 Planet1Location;
    glm::vec3 Planet2Location;
    glm::vec3 moonLocation;
    glm::vec4 Light1Pos;
    glm::vec4 Light2Pos;
    glm::vec4 Light3Pos;
    glm::vec3 SpotLightPos;
    glm::vec3 SpotLightDir;

    // Rotation and movement speeds
    float Planet1RotationSpeed = 20.f;
    float Planet2RotationSpeed = 10.f;
    float moonRotationSpeed = 25.0f;
    float crystalLevSpeed = 2.0f;

    // Distances and amplitudes
    float Planet1Distance = 12.0f;
    float Planet2Distance = 26.5f;
    float moonDistance = 3.0f;
    float crystalLevAmplitude = 0.2f;

    // Rotation angles
    float Planet1Angle;
    float Planet2Angle;
    float moonAngle;
    float crystalOffset;

    // Textures
    GLuint Planet1BCTex, Planet1NMTex,
        Planet2BCTex, Planet2NMTex,
        MoonBCTex, MoonNMTex,
        PlaneTex, PlaneAlpha,
        CrystalBCTex, CrystalNMTex,
        SkyboxTex;

    // Skybox object
    SkyBox skybox;

    // Framebuffer object handle
    GLuint fboHandle;

    // Private function declarations
    void setMatrices();
    void setAlphaMatrices();
    void setSkyboxMatrices();
    void setHDRMatrices();
    void setLightUniforms();
    void setTextures(GLuint Tex1, GLuint Tex2);
    void compile();

    void setupFBO();
    void setupQuad();
    void computeLogAveLuminance();

    void Pass1();
    void Pass2();
    void Pass3();
    void Pass4();

};

#endif // SCENEBASIC_UNIFORM_H
