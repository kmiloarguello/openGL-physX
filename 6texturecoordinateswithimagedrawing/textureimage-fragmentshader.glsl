#version 330 core

uniform vec4 input_color;
out vec4 output_color;

uniform sampler2D imagetex;

in vec2 mytexturecoordinates;

void main (void)
{   
	output_color = texture(imagetex, mytexturecoordinates);
}