#include "scenebasic_uniform.h"
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include <string>
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
#include "helper/glutils.h"
#include "helper/texture.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() : plane(60.0f, 60.0f, 1, 1), skybox(350.0f)
{
    //Load meshes
    Planet1Mesh = ObjMesh::loadWithAdjacency("media/Meshes/Planet.obj", true, true);
    CrystalMesh = ObjMesh::loadWithAdjacency("media/Meshes/Crystalline.obj", true, true);
    MoonMesh = ObjMesh::loadWithAdjacency("media/Meshes/MoonRock.obj", true, true);
    
    //Set Lightpos's - Can be expanded to be moved very easily with the update() function
    Light1Pos = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    Light2Pos = vec4(1.0f, 1.0f, 0.0f, 0.0f);
    Light3Pos = vec4(-1.0f, 0.15f, 0.0f, 0.0f);
    SpotLightPos = glm::vec3(0.0f, -14.0f, 0.0f);
    SpotLightDir = vec3(-glm::vec4(0.0f, -10.0f, 0.0f, 1.0f));

    setupCamera();
}

// Initscene: Initializes the scene by compiling the shaders, setting up the MVP matrices, 
// setting up the FBO for HDR, quad, light uniforms and loads textures.
void SceneBasic_Uniform::initScene()
{
    compile();

    std::cout << std::endl;

    prog.printActiveUniforms();

    glEnable(GL_DEPTH_TEST);

    model = mat4(1.0f);
    view = glm::lookAt(vec3(0.f, 0.f, 6.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    projection = mat4(1.f);

    setupFBO(); //we call the setup for our fbo

    setupQuad(); //we call the setup on the Quad


    // Center yellow point light
    prog.setUniform("Lights[0].Position", Light1Pos);
    prog.setUniform("Lights[0].Ld", vec3(2.0f, 1.0f, 0.3f));
    prog.setUniform("Lights[0].La", vec3(0.2f, 0.15f, 0.05f));
    prog.setUniform("Lights[0].Ls", vec3(1.0f, 0.5f, 0.2f));


    // Top light-blue directional light
    prog.setUniform("Lights[1].Position", Light2Pos);
    prog.setUniform("Lights[1].Ld", vec3(0.3f, 0.6f, 0.8f));
    prog.setUniform("Lights[1].La", vec3(0.0f, 0.1f, 0.0f));
    prog.setUniform("Lights[1].Ls", vec3(0.3f, 0.6f, 0.8f));

    // Side pink directional light
    prog.setUniform("Lights[2].Position", Light3Pos);
    prog.setUniform("Lights[2].Ld", vec3(0.4f, 0.1f, 0.1f));
    prog.setUniform("Lights[2].La", vec3(0.1f, 0.0f, 0.1f));
    prog.setUniform("Lights[2].Ls", vec3(0.4f, 0.1f, 0.1f));

    // Spotlight
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    prog.setUniform("Spotlight.Position", vec3(view * glm::vec4(SpotLightPos, 1.0f)));
    prog.setUniform("Spotlight.Direction", normalMatrix * SpotLightDir);
    prog.setUniform("Spotlight.Ld", vec3(2.0f, 1.6f, 0.6f));
    prog.setUniform("Spotlight.Ls", vec3(0.2f, 0.15f, 0.05f));
    prog.setUniform("Spotlight.La", vec3(1.0f, 0.8f, 0.3f));
    prog.setUniform("Spotlight.Exponent", 50.f);
    prog.setUniform("Spotlight.Cutoff", glm::radians(15.f));

    // Fog properties
    prog.setUniform("Fog.MaxDist", 30.0f);
    prog.setUniform("Fog.MinDist", 10.0f);
    prog.setUniform("Fog.Colour", vec3(0.0f, 0.0f, 0.0f));

    // Outline properties
    prog.setUniform("LineColour", vec3(0.0f, 0.0f, 0.0f));
    prog.setUniform("EdgeWidth", 0.005f);
    prog.setUniform("PctExtend", 0.20f);

    //Load Textures
    CrystalBCTex =
        Texture::loadTexture("media/CrystalTextures/CrystalBase_initialShadingGroup_BaseColor.1001.png");
    CrystalNMTex =
        Texture::loadTexture("media/CrystalTextures/CrystalBase_initialShadingGroup_Normal.1001.png");

    Planet1BCTex =
        Texture::loadTexture("media/PlanetTextures/PlanetLower_1001_BaseColor.png");
    Planet1NMTex =
        Texture::loadTexture("media/PlanetTextures/PlanetLower_1001_Normal.png");

    Planet2BCTex = 
        Texture::loadTexture("media/PlanetTextures/PlanetLower_1002_BaseColor.png");
    Planet2NMTex = 
        Texture::loadTexture("media/PlanetTextures/PlanetLower_1002_Normal.png");

    MoonBCTex = 
        Texture::loadTexture("media/MoonTextures/MoonLow_1001_BaseColor.png");
    MoonNMTex =
        Texture::loadTexture("media/MoonTextures/MoonLow_1001_Normal.png");

    PlaneTex =
        Texture::loadTexture("media/PlaneTextures/PlaneTex.png");

    GLuint SkyboxTex = Texture::loadHdrCubeMap("media/Skybox/space");
}

//Setupcamera: This will init the variables used for the movement system (WASD, Right click movement and rotation)
void SceneBasic_Uniform::setupCamera()
{
    cameraPos = vec3(0.0f, 0.0f, 6.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraSpeed = 5.0f;
    mouseSensitivity = 0.1f;
    lastX = width / 2;
    lastY = height / 2;
    yaw = -90.0f;
    pitch = 0.0f;
}

//SetupFBO: Setup the FBO for HDR rendering. It will create/bind the FBO, create a depth buffer and a HDR buffer.
//Has an additional check to ensure the FBO has been setup correctly.
void SceneBasic_Uniform::setupFBO()
{
    GLuint depthBuf;
    // Create and bind the FBO
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // The depth buffer
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // The HDR color buffer
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &hdrTex);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Attach the images to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        hdrTex, 0);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };

    glDrawBuffers(1, drawBuffers);

    //Check if the FBO is setup correctly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer is not complete!" << std::endl;
        exit(EXIT_FAILURE);
    }

    //Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//SetupQuad: Setup an FSQ to render the HDR FBO to, could also probably be used for UI and other elements.
void SceneBasic_Uniform::setupQuad()
{
    // Array for full-screen quad
    GLfloat verts[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };

    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
    // Set up the vertex array object

    glGenVertexArrays(1, &quad);
    glBindVertexArray(quad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates
    glBindVertexArray(0);
}

//compile; This will compile and link the GLSL shaders for all the created shaders used.
//Basic Uniform: This is the main shader and it uses a Vertex, Geometry and Fragment shader to perform lighting calculations on a per-pixel basis.
//Basic Alpha: A simple shader that uses a Vertex and Fragment shader to sample from a base and alpha texture to discard pixels.
//Basic HDR: This will perform tonemapping and gamma correction on the outputted texture from the uniform to calculate exposure.
//Skybox: This is used simply for the skybox around the scene.
void SceneBasic_Uniform::compile()
{
	try {
        Alphaprog.compileShader("shader/basic_alpha.vert");
        Alphaprog.compileShader("shader/basic_alpha.frag");
        Alphaprog.link();
        Skyboxprog.compileShader("shader/skybox.vert");
        Skyboxprog.compileShader("shader/skybox.frag");
        Skyboxprog.link();
        HDRprog.compileShader("shader/basic_HDR.vert");
        HDRprog.compileShader("shader/basic_HDR.frag");
        HDRprog.link();
		prog.compileShader("shader/basic_uniform.vert");
        prog.compileShader("shader/basic_uniform.gs");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

// update: Updates the angles and animation for the scene's meshes, it also calculates the deltaTime between a given frame for consistent movement.
// Only runs the animations if m_animate is true (space controleld in scenerunner)
void SceneBasic_Uniform::update( float t )
{
    deltaTime = t - elapsedTime;
    elapsedTime = t;

    if (!m_animate) return;

    Planet1Angle = t * Planet1RotationSpeed;
    Planet2Angle = t * Planet2RotationSpeed;
    moonAngle = t * moonRotationSpeed;
    crystalOffset = sin(t * crystalLevSpeed) * crystalLevAmplitude;

}

// render: Renders the scene by setting up framebuffers, clearing buffers, enabling depth testing, and running multiple shader passes.
void SceneBasic_Uniform::render()
{
    projection = glm::perspective(glm::radians(70.0f), (float)width / height, 0.3f, 500.f);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //Scene geometry pass - BasicUniform.Vert/GS/Frag
    prog.use();
    Pass1();

    //Alpha shader pass - BasicAlpha.Vert/Frag
    Alphaprog.use();
    Pass2();

    //Skybox shader pass - Skybox.Vert/Frag
    Skyboxprog.use();
    Pass3();

    //HDR shader pass - BasicHDR.Vert/Frag
    HDRprog.use();
    computeLogAveLuminance();
    Pass4();
}

// Pass1: Sets the material properties, textures, and transformations for each mesh in the scene and renders them using the basic shader program.
void SceneBasic_Uniform::Pass1()
{
    prog.setUniform("Material.Kd", 1.0f, 1.0f, 1.0f);
    prog.setUniform("Material.Ks", 1.05f, 1.05f, 1.05f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

    setTextures(Planet1BCTex, Planet1NMTex);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(Planet1Angle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(Planet1Distance, -0.6f, 0.0f));
    model = glm::scale(model, vec3(0.6f, 0.6f, 0.6f));
    setMatrices();
    setLightUniforms();
    Planet1Mesh->render();

    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);

    setTextures(Planet2BCTex, Planet2NMTex);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(Planet2Angle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(Planet2Distance, 0.0f, 0.0f));
    setMatrices();
    Planet1Mesh->render();

    setTextures(MoonBCTex, MoonNMTex);
    model = glm::rotate(model, glm::radians(moonAngle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(moonDistance, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));
    setMatrices();
    MoonMesh->render();

    prog.setUniform("Material.Ks", 1.0f, 1.0f, 1.0f);

    setTextures(CrystalBCTex, CrystalNMTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, crystalOffset, 0.0f));
    setMatrices();
    CrystalMesh->render();
}

// Pass2: Sets the textures and transformations for the alpha blended plane and renders it using the alpha shader program.
void SceneBasic_Uniform::Pass2()
{
    setTextures(PlaneTex, PlaneTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -1.5f, 0.0f));
    setAlphaMatrices();
    plane.render();
}

// Pass3: Sets the transformations for the skybox and renders it using the skybox shader program.
void SceneBasic_Uniform::Pass3()
{
    model = mat4(1.0f);
    setSkyboxMatrices();
    skybox.render();
}

// Pass4: Reverts to the default framebuffer and renders the HDR quad using the HDR shader program.
void SceneBasic_Uniform::Pass4()
{
    // Revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    view = mat4(1.0);
    model = mat4(1.0);
    projection = mat4(1.0);
    setHDRMatrices();

    // Render the quad
    glBindVertexArray(quad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// computeLogAveLuminance: Computes the average logarithmic luminance of the rendered scene for tone mapping, using a downsampled set of pixels.
// This will only run for every 64 pixels as running it on every pixel can be very costly (I'm on an rtx 3080 and it tanked FPS massively)
void SceneBasic_Uniform::computeLogAveLuminance()
{
 
    int size = width * height;

    std::vector<GLfloat> texData(size * 3);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTex);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texData.data());
    float sum = 0.0f;
    for (int i = 0; i < size; i+= 64) {
        float lum = glm::dot(vec3(texData[i * 3 + 0], texData[i * 3 + 1],
            texData[i * 3 + 2]),
            vec3(0.2126f, 0.7152f, 0.0722f));
        sum += logf(lum + 0.0000001f) ;
    }
    float Totalsum = expf(sum / (size / 64));
    
    HDRprog.setUniform("AveLum", Totalsum * 8);

}

// resize: Updates the viewport and projection matrix according to the new window dimensions.
void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 500.f);
}

// setMatrices: Sets the model-view, normal, and model-view-projection matrices for the basic shader program.
void SceneBasic_Uniform::setMatrices()
{
    glm::mat4 mv = view * model;
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
    prog.setUniform("ProjectionMatrix", projection);


}

// setAlphaMatrices: Sets the model-view, normal, and model-view-projection matrices for the alpha shader program.
void SceneBasic_Uniform::setAlphaMatrices()
{
    glm::mat4 mv = view * model;
    Alphaprog.setUniform("ModelViewMatrix", mv);
    Alphaprog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    Alphaprog.setUniform("MVP", projection * mv);
    Alphaprog.setUniform("ProjectionMatrix", projection);
}

// setSkyboxMatrices: Sets the model-view-projection matrix for the skybox shader program.
void SceneBasic_Uniform::setSkyboxMatrices()
{
    glm::mat4 mv = view * model;
    Skyboxprog.setUniform("MVP", projection * mv);
}

// setHDRMatrices: Sets the model-view-projection matrix for the HDR shader program.
void SceneBasic_Uniform::setHDRMatrices()
{
    glm::mat4 mv = view * model;
    HDRprog.setUniform("MVP", projection * mv);
}

// setLightUniforms: Sets the light position, direction, and other properties for the basic shader program.
void SceneBasic_Uniform::setLightUniforms()
{

    prog.setUniform("Lights[0].Position", view * Light1Pos);
    prog.setUniform("Lights[1].Position", view * Light2Pos);
    prog.setUniform("Lights[2].Position", view * Light3Pos);
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    prog.setUniform("Spotlight.Position", vec3(view * glm::vec4(SpotLightPos, 1.0f)));
    prog.setUniform("Spotlight.Direction", normalMatrix * SpotLightDir);
}

// setTextures: Binds the provided texture objects to the active texture units
void SceneBasic_Uniform::setTextures(GLuint Tex1, GLuint Tex2)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Tex1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Tex2);
}

//
// updateCamera: is called from scenerunner and will handle WASD + mouse movement to modify the view.
// I was going to use a callback instead inside scenerunner.h, however that requires a static function that cant take additional parameters.
// There may be a minor jolt with quick right clicks 
//
void SceneBasic_Uniform::updateCamera(GLFWwindow* window)
{
    // Get the current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Calculate the offset between the current and previous cursor positions
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    // Update the last cursor position
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {

        float AdjustedcameraSpeed = deltaTime * cameraSpeed;
        // Hide and lock the cursor
        if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) 
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // Update the yaw and pitch based on the offsets
        yaw += xoffset;
        pitch += yoffset;

        // Clamp the pitch between -89 and 89 degrees to stop camera doing 360s
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // Calculate the new camera front vector based on yaw and pitch
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);

        // Move the camera based on keyboard inputs
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += AdjustedcameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= AdjustedcameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.f, 1.f, 0.f))) * AdjustedcameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, glm::vec3(0.f, 1.f, 0.f))) * AdjustedcameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPos += AdjustedcameraSpeed * glm::vec3(0.f, 1.f, 0.f);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            cameraPos -= AdjustedcameraSpeed * glm::vec3(0.f, 1.f, 0.f);
    }

    // If the right mouse button is not pressed, make the cursor visible and unlocked
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    

    // Update the view matrix for usage in setMatrices();
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, glm::vec3(0.f, 1.f, 0.f));
}
