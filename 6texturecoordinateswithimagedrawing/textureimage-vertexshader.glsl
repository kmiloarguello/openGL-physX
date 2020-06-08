#version 330 core

layout(location = 0) in vec4 vertex_modelspace;
layout(location = 2) in vec2 texturecoordinate_modelspace;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;

out vec2 mytexturecoordinates;

void main() {
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace; 
	mytexturecoordinates = texturecoordinate_modelspace;
}
