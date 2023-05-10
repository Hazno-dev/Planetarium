#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Scene
{
protected:
	glm::mat4 model, view, projection;

    //Camera Movement
    glm::vec3 cameraPos, cameraFront;
    float cameraSpeed, mouseSensitivity;
    double lastX, lastY;
    float yaw, pitch;

public:
    int width;
    int height;

	Scene() : m_animate(true), width(800), height(600) { }
	virtual ~Scene() {}

	void setDimensions( int w, int h ) {
	    width = w;
	    height = h;
	}
	
    /**
      Load textures, initialize shaders, etc.
      */
    virtual void initScene() = 0;

    /**
      This is called prior to every frame.  Use this
      to update your animation.
      */
    virtual void update( float t ) = 0;

    /**
      Draw your scene.
      */
    virtual void render() = 0;

    /**
      Called when screen is resized
      */
    virtual void resize(int, int) = 0;

    /**
      Setup camera movement variables. 
      Call from constructor.
    */
    virtual void setupCamera() = 0;

    /**
      Update camera movement variables based on input.
      Call in scenerunner with window passed in.
    */
    virtual void updateCamera(GLFWwindow* window) = 0;

    void animate(bool value) { m_animate = value; }
    bool animating() { return m_animate; }
    
protected:
	bool m_animate;

    float deltaTime = 0.f;
    float elapsedTime = 0.f;
};
