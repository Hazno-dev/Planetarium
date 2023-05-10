#version 460

//
// In/Outs
//

layout (triangles_adjacency) in;
layout (triangle_strip, max_vertices=15) out;

out vec4 GPosition;
out vec2 GTexCoord;
out vec3 GLightsDir[3];
out vec3 GSpotlightPos;
out vec3 GSpotlightDir;
out vec3 GViewDir;

noperspective out vec3 GEdgeDistance;
in vec3 Normal[];
in vec4 Position[];
in vec2 TexCoord[];
in vec3 LightsDir[][3];
in vec3 SpotlightPos[];
in vec3 SpotlightDir[];
in vec3 ViewDir[];

flat out int GIsEdge; //0 No edge - 1 edge

//
//Uniforms
//

uniform float EdgeWidth;
uniform float PctExtend;

//
//Consts
//

const bool bOulineOn = true;

//
//Functions
//

bool isFrontFacing(vec3 a, vec3 b, vec3 c) { return ((a.x * b.y - b.x * a.y) + (b.x * c.y - c.x * b.y) + (c.x * a.y - a.x * c.y)) > 0; }
void emitEdgeQuad(vec3 e0, vec3 e1);
bool isInsideViewport(vec4 pos);

//
//Main
//

void main(){

	if (bOulineOn){
		vec3 p0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
		vec3 p1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
		vec3 p2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
		vec3 p3 = gl_in[3].gl_Position.xyz / gl_in[3].gl_Position.w;
		vec3 p4 = gl_in[4].gl_Position.xyz / gl_in[4].gl_Position.w;
		vec3 p5 = gl_in[5].gl_Position.xyz / gl_in[5].gl_Position.w;

		if (isFrontFacing(p0,p2,p4)){
			if (!isFrontFacing(p0,p1,p2)) emitEdgeQuad(p0, p2);
			if (!isFrontFacing(p2,p3,p4)) emitEdgeQuad(p2, p4);
			if (!isFrontFacing(p4,p5,p0)) emitEdgeQuad(p4, p0);
		}
	}

	GIsEdge = 0;

	GTexCoord = TexCoord[0];
	for (int i = 0; i < 3; ++i) GLightsDir[i] = LightsDir[0][i];
	GSpotlightPos = SpotlightPos[0];
	GSpotlightDir = SpotlightDir[0];
	GViewDir = ViewDir[0];
	GPosition = Position[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	GTexCoord = TexCoord[2];
	for (int i = 0; i < 3; ++i) GLightsDir[i] = LightsDir[2][i];
	GSpotlightPos = SpotlightPos[2];
	GSpotlightDir = SpotlightDir[2];
	GViewDir = ViewDir[2];
	GPosition = Position[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();

	GTexCoord = TexCoord[4];
	for (int i = 0; i < 3; ++i) GLightsDir[i] = LightsDir[4][i];
	GSpotlightPos = SpotlightPos[4];
	GSpotlightDir = SpotlightDir[4];
	GViewDir = ViewDir[4];
	GPosition = Position[4];
	gl_Position = gl_in[4].gl_Position;
	EmitVertex();
	EndPrimitive();
}

bool isInsideViewport(vec4 pos) {
    return all(lessThanEqual(pos.xy, vec2(1.0))) && all(greaterThanEqual(pos.xy, vec2(-1.0)));
}

void emitEdgeQuad(vec3 e0, vec3 e1) {
    vec2 ext = PctExtend * (e1.xy - e0.xy);
    vec2 v = normalize(e1.xy - e0.xy);
    vec2 n = vec2(-v.y, v.x) * EdgeWidth;
    float depthBias = 0.0001; // Adjust this value as needed
    GIsEdge = 1;

    vec4 p0 = vec4(e0.xy - ext, e0.z - depthBias, 1.0);
    vec4 p1 = vec4(e0.xy - n - ext, e0.z - depthBias, 1.0);
    vec4 p2 = vec4(e1.xy + ext, e1.z - depthBias, 1.0);
    vec4 p3 = vec4(e1.xy - n + ext, e1.z - depthBias, 1.0);

    if (isInsideViewport(p0) && isInsideViewport(p1) && isInsideViewport(p2) && isInsideViewport(p3)) {
        gl_Position = p0; EmitVertex();
        gl_Position = p1; EmitVertex();
        gl_Position = p2; EmitVertex();
        gl_Position = p3; EmitVertex();
        EndPrimitive();
    }
}

