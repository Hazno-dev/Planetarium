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

##


## Videos

*C2 Video*

[![CW2 Video](http://img.youtube.com/vi/7KogksYltOM/0.jpg)](http://www.youtube.com/watch?v=7KogksYltOM)

*C1 Video*

[![CW1 Video](http://img.youtube.com/vi/A1GKJZ3vU5Y/0.jpg)](http://www.youtube.com/watch?v=A1GKJZ3vU5Y)
