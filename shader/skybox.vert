#version 460

//
// In/Outs
//

layout (location = 0) in vec3 VertexPosition;

out vec3 Vec;

//
//Uniforms
//

uniform mat4 MVP;

//
//Main
//

void main()
{
    Vec = VertexPosition;
 
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}


