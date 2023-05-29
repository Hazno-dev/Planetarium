# The Planetarium

A small planet simulation scene built in OpenGL 4.6 and C++. Tested on Visual Studio 2022 and Windows 11.

![Image Of Solar System](https://github.com/Hazno-dev/Planetarium/assets/62346467/6544d39a-9726-4a71-9f43-29198dff4997)

## Usage & Installation

Existing build of the project is located inside "Build" folder.
Run "Project_Template.exe" to run.

## Controls

| Control      | Usage          |
| ------------- |:-------------:| 
| WASD      | Forward/Backwards/Left/Right Movement | 
| Q/E      | Up/Down Movement      | 
| Right Click | Enable Movement / Camera Panning     |    
| Mouse | Camera Panning      |
| Space | Pause Animations      |
| T | Toggle Night Vision      |

## Project Info

The Planetarium is a small OpenGL renderer that contains 2 planets, a moon, and a meteor orbiting a morphing crystal. This project split different features into 6 different shader programs rather than utilizing one large shader to reduce unnecessary overhead and code size. The scene uses a BlinnPhong lighting model, with several lighting, texturing,and model deformation techniques to produce a (hopefully) aesthetically pleasing scene. The C++ side of the project splits the render loop into 6 passes, each one utilizing a different shader program and rendering different components of the scene.

#### Supported features:
- BlinnPhong Shading Model
- Multi-Lights
- Spotlights
- ToonShading
- Fog
- Normals
- **Surface Animations**
- **Particle System with Transform Feedback**
- **Fire and Smoke Particles**
- Multi-Texture Inputs
- Outlines
- Alpha Map Discarding
- Skybox
- **Disintegration/Paint Splatter**
- HDR Tonemapping
- **Night Vision**
- Gamma Correction
- User Input

**Features highlighted in bold are implemented in C2**

## Code Overview

### Basic_Uniform – BlinnPhong, Multi-Lights, Spotlight, ToonShading, Fog, Normals, Multitexture, Outlines, Surface Animations, Disintegration/Paint Splatter

This shader handles the majority of the scenes geometry in pass1() and it utilizes a vertex, geometry and fragment shader. In the vertex shader, it takes the vertex attributes position, normal, texcoords and tangent and it transform the normals of the vertices into tangent space for the lighting calculations by constructing a matrix for transformation (so normal maps can be correctly sampled regardless of view). Additionally, the vertex contains an old implementation of vertex-based phong lighting as per the marking rubric requirements however it is not utilized as fragment shader lighting yields better results. In the scene, the crystal in the centre also uses surface animations. The surface animation is handled in the vertex shader where the y axis of the vertex and the normal are modified against a sin wave function.

The geometry shader handles the outlines of the objects. It will take in vertices with adjacency information and output a triangle strip, if the triangles are front facing, it will set the GIsEdge output to true for usage in the fragment shader. I tried to adapt this shader for perspective cameras as the lab implementation used an orthographic camera. In the scene currently, this outline shader is disabled, however it can be easily re-enabled.
The fragment shader handles most of the features of this shader. It will firstly check if the fragment is an edge (defined in the geo shader), if true, it will simply set the output colour to the uniform that defines the colours of edges. Otherwise, it will check if multi-texture blending is enabled and mix textures together based on the alpha channel. Normal map calculations are done to adjust the normal vectors and it conducts blinn-phong lighting for 3 point lights and 1 spotlight with optional toon shading to clamp the outputted colour values. The fragment shader also has the disintegration() function, which will use a noise texture uniform input to randomly sample fragments and reference them against a high and low threshold. If a fragments alpha is too high or low it will be discarded, or painted a given colour (this can be modified to discard very easily).

### Basic_Alpha – Alpha Map Discard 

This shader is relatively simple, it just takes in the same vertex attributes as before and passes the texture coordinates to the fragment shader. Additionally, it sets the gl_position like before using the model-view-projection matrix. The fragment shader for this will simply check against inputted alpha maps values to discard pixels with an alpha below a given threshold. This is utilized in the program with the Plane class. 

### Basic_Particle – Particle Fountain / Transformative Feedback / Fire & Smoke

In pass 4 of the render loop, the basic_particle shader is used. This shader is designed to handle both smoke and fire particles in the scene. The 3 features mentioned above are all built upon each other (particle fountain leads to transformative feedback, which then leads to fire & smoke). This shader is used in conjunction with a series of buffers to feed data in/out of the shader and keep track of particles over their lifetime (this is further explored in the C2 video).

The vertex shader for the particle system operates in two phases; the update phase and the render phase. It takes in attributes like the position, velocity and age from the buffer. During the update phase, it will determine the particles initial position and velocity if the particles age is less than zero or greater than its lifespan. For living particles, it’ll calculate the new position and velocity using the inputted uniforms and attributes and increase the particles age.  For the random initial position and velocity, a random texel fetch is used to sample from an inputted noise texture. This helps create diverse behaviour in the particles, ensuring that the particles don’t all behave identically and giving the scene a more realistic look.

During the rendering phase in the vertex shader, the transparency of the particle is calculated based on the age of the particle. It’ll also modify the particles size depending on if variable size is enabled. Lastly, it’ll modify the gl_position into clip_space using the model view matrix and a projection matrix.

### Basic_HDR – HDR Tonemapping, Gamma Correction 

The vertex shader for the HDR shader program is simple. It uses the vertex position and texcoords and passes them to the fragment shader. The fragment shader isn’t quite as simple. It uses a technique known as tone-mapping to convert HDR colours to LDR colours while preserving the appearance as best as possible. It uses a uniform value passed in from the “computeLogAveLuminance()” function and inputted texture to convert the inputted RGB texture to XYZ colour space and separates the luminance from the chromaticity. It then uses the reinhard tone mapping operation and converts the luminance back to XYZ and RGB colour space values. Lastly, it will gamma correct with a value of 1.4 instead of 2.2 (it fits better with the given scene). This is utilized by a simple full-screen quad to render the final texture outputted by this. This is program is used in pass 5 and is toggled between when the user presses the T key, to switch between HDR tonemapping and night vision. 

### Basic_NightVIS – Night Vision 

The night vision shader kicks in at pass 6 and is enabled/disabled when the user presses the T key. When this is enabled, the HDR pass is skipped, and vice-versa.  The vertex shader of this remains identical to basic_hdr, which just passes the texcoords to the fragment shader and modifies the gl_position using a model-view-projection matrix.

The fragment shader takes in the windows width and height, a radius and a noise texture to  produce a noisy, green image with the look of night vision goggles. The main function begins by retrieving the fully rendered image so far, sampling the colour at the previously received texcoord. This colour is used in the luminance() function to receive the dot product of the colour. The shader will then calculate two distance values, each representing the distance of the current fragment from the left and right centre of the screen. If either of these distance is larger by the inputted radius, the colour is set to black (making the goggle effect). Lastly in this shader, the final colour is outputted with the green channel mixed against the noise texture and then it is gamma corrected. 

### Skybox – Skybox 

This shader simply takes in a vertex position attribute to pass to the fragment shader and sets the position again like the last two. In the fragment shader it will simply sample a cube map (a collection of 6 2D textures) using the normalized vertex positions as lookup vectors to set the pixels colour. This is utilized in the program with the Skybox class. 

### Update and updateCamera – Control view, rotation animation 

The movement of the planets and crystal is handled in the “update()” function where it will simply multiply the planets angle by the current time and use a sin wave on the crystals current height offset. The camera movement is handled by the “updateCamera()” function. This will get the current cursors pos on the screen, disable it (to hide it) and calculate offsets to increase or decrease the cameras positions and front vectors to construct a new view matrix

## Videos

*C2 Video*

[![CW2 Video](http://img.youtube.com/vi/7KogksYltOM/0.jpg)](http://www.youtube.com/watch?v=7KogksYltOM)

*C1 Video*

[![CW1 Video](http://img.youtube.com/vi/A1GKJZ3vU5Y/0.jpg)](http://www.youtube.com/watch?v=A1GKJZ3vU5Y)

*Skybox Source*
https://www.youtube.com/watch?v=sksfsOSlGkQ
