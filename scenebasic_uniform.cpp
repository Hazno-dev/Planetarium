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
#include "helper/particleutils.h"
#include "helper/noisetex.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;

SceneBasic_Uniform::SceneBasic_Uniform() : plane(60.0f, 60.0f, 1, 1), skybox(350.0f), particleLifetime(1.0f), nParticles(300), emitterPos(0, 0, 0), emitterDir(0, 0, -1),
    SparticleLifetime(2.5f), SnParticles(300)
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

    setupParticles();

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

    // Noise
    prog.setUniform("NoiseTex", 2);


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
        Texture::loadTexture("media/PlaneTextures/PlaneTex2.png");

    FParticleTex =
		Texture::loadTexture("media/VFX/fire.png");
    SParticleTex =
		Texture::loadTexture("media/VFX/smoke.png");
    RandomTex =
        ParticleUtils::createRandomTex1D(nParticles * 3);

    noiseTex =
        NoiseTex::generate2DTex(16.0f);
    FSnoisetex =
        NoiseTex::generatePeriodic2DTex(200.0f, 0.5f, 800, 600);

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

//SetupParticles: This will init the buffers used for the particles
//It creates pairs of buffers for handling the position, velocity and age of each particle (twice for smoke & fire particles)
//It also creates multiple vertex arrays to link the first and second buffers together
//The position and velocity values are XYZ, and the age is a single float
//Age is populated with decreasing values based on the lifetime to gradually transition particle age.
//Transform feedback objects are created to capture the output from the shader program and store it in the buffer
void SceneBasic_Uniform::setupParticles()
{
    //FIRE PARTICLES
    // 
    //setp position, velocity, age buffers
    glGenBuffers(2, posBuf);
	glGenBuffers(2, velBuf);
	glGenBuffers(2, ageBuf);
    
    
    //allocate space for buffers
	int size = nParticles * 3 * sizeof(float);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, ageBuf[0]);
	glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), NULL, GL_DYNAMIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, ageBuf[1]);
	glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(float), NULL, GL_DYNAMIC_COPY);
    
	//fill age buffer
	std::vector<GLfloat> tempData(nParticles);
	float rate = particleLifetime / nParticles;
	for (int i = 0; i < nParticles; i++) tempData[i] = rate * (i - nParticles);
    
	glBindBuffer(GL_ARRAY_BUFFER, ageBuf[0]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(float), tempData.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    
	//create and set VAO for each set of buffers
	glGenVertexArrays(2, particleArray);
    
	glBindVertexArray(particleArray[0]);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
    
	glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
    
	glBindBuffer(GL_ARRAY_BUFFER, ageBuf[0]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
    
	glBindVertexArray(particleArray[1]);
	glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
    
	glBindBuffer(GL_ARRAY_BUFFER, ageBuf[1]);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
    
	//create transform feedback objects
	glGenTransformFeedbacks(2, feedback);
    
	//set up transform feedback objects
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, ageBuf[0]);
    
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, ageBuf[1]);
    
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    //SMOKE PARTICLES
    //
    //setp position, velocity, age buffers
    glGenBuffers(2, SposBuf);
    glGenBuffers(2, SvelBuf);
    glGenBuffers(2, SageBuf);


    //allocate space for buffers
    int Ssize = SnParticles * 3 * sizeof(float);
    glBindBuffer(GL_ARRAY_BUFFER, SposBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, Ssize, NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, SposBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, Ssize, NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, SvelBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, Ssize, NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, SvelBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, Ssize, NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, SageBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, SnParticles * sizeof(float), NULL, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, SageBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, SnParticles * sizeof(float), NULL, GL_DYNAMIC_COPY);

    //fill age buffer
    std::vector<GLfloat> StempData(SnParticles);
    float Srate = SparticleLifetime / SnParticles;
    for (int i = 0; i < SnParticles; i++) StempData[i] = Srate * (i - SnParticles);

    glBindBuffer(GL_ARRAY_BUFFER, SageBuf[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, SnParticles * sizeof(float), StempData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //create and set VAO for each set of buffers
    glGenVertexArrays(2, SparticleArray);

    glBindVertexArray(SparticleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, SposBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, SvelBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, SageBuf[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(SparticleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, SposBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, SvelBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, SageBuf[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    //create transform feedback objects
    glGenTransformFeedbacks(2, Sfeedback);

    //set up transform feedback objects
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, Sfeedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, SposBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, SvelBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, SageBuf[0]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, Sfeedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, SposBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, SvelBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, SageBuf[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
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
//Basic Particle: This will perform the transform feedback particle simulation and rendering.
//Basic NightVIS: This will perform a simple night vision effect on the outputted texture from the uniform.
// Skybox: This is used simply for the skybox around the scene.
void SceneBasic_Uniform::compile()
{
	try {
        Alphaprog.compileShader("shader/basic_alpha.vert");
        Alphaprog.compileShader("shader/basic_alpha.frag");
        Alphaprog.link();
        
		ParticleProg.compileShader("shader/basic_particle.vert");
		ParticleProg.compileShader("shader/basic_particle.frag");
		GLuint progHandle = ParticleProg.getHandle();
		const char* outputNames[] = { "Position", "Velocity", "Age" };
		glTransformFeedbackVaryings(progHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);
		ParticleProg.link();
        
        Skyboxprog.compileShader("shader/skybox.vert");
        Skyboxprog.compileShader("shader/skybox.frag");
        Skyboxprog.link();
        HDRprog.compileShader("shader/basic_HDR.vert");
        HDRprog.compileShader("shader/basic_HDR.frag");
        HDRprog.link();
		NightVprog.compileShader("shader/basic_NightVIS.vert");
		NightVprog.compileShader("shader/basic_NightVIS.frag");
		NightVprog.link();
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
	meteorAngle = t * meteorRotationSpeed;
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

    //Particle pass - BasicParticle.Vert/Frag
    ParticleProg.use();
    Pass4();

    //HDR shader pass - BasicHDR.Vert/Frag
    HDRprog.use();
    computeLogAveLuminance();
    Pass5();

    //Night vis pass - BasicNightVIS.Vert/Frag
	NightVprog.use();
    Pass6();
}

// Pass1: Sets the material properties, textures, and transformations for each mesh in the scene and renders them using the basic shader program.
void SceneBasic_Uniform::Pass1()
{
    prog.setUniform("Time", elapsedTime);

    prog.setUniform("Material.Kd", 1.0f, 1.0f, 1.0f);
    prog.setUniform("Material.Ks", 1.05f, 1.05f, 1.05f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);

	//Planet1 Render
    setTextures(Planet1BCTex, Planet1NMTex);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(Planet1Angle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(Planet1Distance, -0.6f, 0.0f));
    model = glm::scale(model, vec3(0.6f, 0.6f, 0.6f));
    setMatrices();
    setLightUniforms();
    Planet1Mesh->render();

    //Planet2 Render
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);

    setTextures(Planet2BCTex, Planet2NMTex);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(Planet2Angle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(Planet2Distance, 0.0f, 0.0f));
    setMatrices();
    Planet1Mesh->render();

    //Moon Render
    setTextures(MoonBCTex, MoonNMTex);
    model = glm::rotate(model, glm::radians(moonAngle), vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, vec3(moonDistance, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));
    setMatrices();
    MoonMesh->render();

	//Meteor Render - Uses disintegration shader && moon mesh
	if (meteorLocation != meteorPreviousLocation) meteorPreviousLocation = meteorLocation;
	model = mat4(1.0f);
	model = glm::rotate(model, glm::radians(meteorAngle), vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, vec3(meteorDistance, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.6f, 0.6f, 0.6f));
	meteorLocation = vec3(model[3]);
	setMatrices();
    prog.setUniform("bDisintegrationOn", true);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTex);
	MoonMesh->render();
    prog.setUniform("bDisintegrationOn", false);

    //Crystal Render - Uses Sinwave surface animation 
    prog.setUniform("Material.Ks", 1.0f, 1.0f, 1.0f);

    prog.setUniform("bSinWaveAnim", true);
    setTextures(CrystalBCTex, CrystalNMTex);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, crystalOffset, 0.0f));
    setMatrices();
    CrystalMesh->render();
    prog.setUniform("bSinWaveAnim", false);
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

// Pass5: Reverts to the default framebuffer and renders the HDR quad using the HDR shader program.
void SceneBasic_Uniform::Pass5()
{
    if (tKey) return;
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

// Pass6: Night Vision Post Processing if T Key is pressed
void SceneBasic_Uniform::Pass6()
{
	if (!tKey) return;

	// Revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    view = mat4(1.0);
    model = mat4(1.0);
    projection = mat4(1.0);
    setNightVISMatrices();

    // Render the quad
    glBindVertexArray(quad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// Pass4: Particle System Pass - rendered after (almost) everything else
// Sets the uniforms for the particle system and renders it using the particle shader program.
void SceneBasic_Uniform::Pass4()
{
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // First Render
    // SMOKE Particles
    
    
    setTextures(SParticleTex, SParticleTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, RandomTex);

    model = mat4(1.0f);
    setParticleMatrices();
    setSmokeParticleMatrices();

    //First pass - render particles to buffer

    ParticleProg.setUniform("Pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, Sfeedback[SdrawBuf]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(SparticleArray[1 - SdrawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, SnParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    //Second pass - render from buffer to screen

    glDepthMask(GL_FALSE);
    ParticleProg.setUniform("Pass", 2);

    glBindVertexArray(SparticleArray[SdrawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, SnParticles);
    glBindVertexArray(0);

    SdrawBuf = 1 - SdrawBuf;
    glDepthMask(GL_TRUE);
    
	//Second Render - Rendered after to avoid depth issues
    //FIRE Particles!!

    setTextures(FParticleTex, FParticleTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, RandomTex);

    model = mat4(1.0f);
    setParticleMatrices();
	setFireParticleMatrices();

    //First pass - render particles to buffer

    ParticleProg.setUniform("Pass", 1);

    glEnable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
    glBeginTransformFeedback(GL_POINTS);

    glBindVertexArray(particleArray[1 - drawBuf]);
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);
    glDrawArrays(GL_POINTS, 0, nParticles);
    glBindVertexArray(0);

    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);

    //Second pass - render from buffer to screen

    glDepthMask(GL_FALSE);
    ParticleProg.setUniform("Pass", 2);

    glBindVertexArray(particleArray[drawBuf]);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
    glBindVertexArray(0);

    drawBuf = 1 - drawBuf;

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
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

// randFloat: Returns a random float 
float SceneBasic_Uniform::randFloat()
{
	return rand.nextFloat();
}

// resize: Updates the viewport and projection matrix according to the new window dimensions.
void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 500.f);
    
    //Fix for full-screen noise texture
    FSnoisetex =NoiseTex::generatePeriodic2DTex(200.0f, 0.5f, w, h);
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

void SceneBasic_Uniform::setSmokeParticleMatrices()
{
    ParticleProg.setUniform("ParticleLifetime", SparticleLifetime);
    ParticleProg.setUniform("ParticleSize", 0.4f);
    ParticleProg.setUniform("bVariableSize", true);
    ParticleProg.setUniform("MinParticleSize", 1.3f);
    ParticleProg.setUniform("MaxParticleSize", 2.3f);
}

void SceneBasic_Uniform::setFireParticleMatrices()
{
    ParticleProg.setUniform("ParticleLifetime", particleLifetime);
    ParticleProg.setUniform("ParticleSize", 0.4f);
    ParticleProg.setUniform("bVariableSize", true);
	ParticleProg.setUniform("MinParticleSize", 1.0f);
    ParticleProg.setUniform("MaxParticleSize", 1.4f);
}

void SceneBasic_Uniform::setParticleMatrices()
{
    glm::mat4 mv = view * model;
    
    emitterPos = meteorLocation;
    emitterDir = -glm::normalize(meteorLocation - meteorPreviousLocation);
    
    ParticleProg.setUniform("RandomTex", 1);
    ParticleProg.setUniform("ParticleTex", 0);
	ParticleProg.setUniform("Time", elapsedTime);
    ParticleProg.setUniform("DeltaTime", deltaTime);
    ParticleProg.setUniform("Acceleration", vec3(0.0f, 0.1f, 0.0f));
    ParticleProg.setUniform("EmitterPosition", emitterPos);
    ParticleProg.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));
    
	ParticleProg.setUniform("MV", mv);
	ParticleProg.setUniform("Projection", projection);
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

void SceneBasic_Uniform::setNightVISMatrices()
{
    glm::mat4 mv = view * model;
    NightVprog.setUniform("MVP", projection * mv);
    NightVprog.setUniform("NoiseTex", 1);
    NightVprog.setUniform("Width", width);
	NightVprog.setUniform("Height", height);
    NightVprog.setUniform("Radius", width / 3.5f);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, FSnoisetex);
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
