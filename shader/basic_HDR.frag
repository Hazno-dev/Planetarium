#version 460

//
// In/Outs
//

layout (binding = 0) uniform sampler2D HdrTex;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

//
//Uniforms
//

uniform float AveLum;

// XYZ/RGB conversion matrices from:
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

uniform mat3 rgb2xyz = mat3(
     0.4124564, 0.2126729, 0.0193339,
     0.3575761, 0.7151522, 0.1191920,
     0.1804375, 0.0721750, 0.9503041 );
uniform mat3 xyz2rgb = mat3(
     3.2404542, -0.9692660, 0.0556434,
     -1.5371385, 1.8760108, -0.2040259,
     -0.4985314, 0.0415560, 1.0572252 );

uniform float Exposure = 0.12;
uniform float White = 1000.928;
uniform bool DoToneMap = true;

//
//Main
//

void main() {
     // Retrieve high-res color from texture
     vec4 colour = texture( HdrTex, TexCoord );

     // Convert to XYZ
     vec3 xyzCol = rgb2xyz * vec3(colour);

     // Convert to xyY
     float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
     vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

     // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
     float L = (Exposure * xyYCol.z) / AveLum;
     L = (L * ( 1 + L / (White * White) )) / ( 1 + L );

     // Using the new luminance, convert back to XYZ
     xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
     xyzCol.y = L;
     xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;

     // Convert back to RGB and send to output buffer + gamma correction (1.4 not 2.2)
     if(DoToneMap) FragColor = vec4( pow( xyz2rgb * xyzCol, vec3(1.0/1.4) ), 1.0 );
     else FragColor =  vec4( pow( colour.rgb, vec3(1.0/1.4) ), 1.0 );
}
