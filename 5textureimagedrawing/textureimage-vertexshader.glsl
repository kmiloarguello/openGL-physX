#version 330 core

layout(location = 0) in vec4 vertex_modelspace;

uniform mat4 myprojection_matrix;
uniform mat4 myview_matrix;

void main() {
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace; 
}
