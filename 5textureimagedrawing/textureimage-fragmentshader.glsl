#version 330 core

uniform vec4 input_color;
out vec4 output_color;

uniform sampler2D imagetex;

void main (void)
{   
	output_color = texture(imagetex, vec2(0.6,0.9));
}