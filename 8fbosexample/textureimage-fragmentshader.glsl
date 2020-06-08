#version 330 core

uniform vec4 input_color;
out vec4 output_color;

uniform sampler2D imagetex;

uniform int to_texture;

in vec2 mytexturecoordinates;

void main (void)
{   
	if (to_texture == 1)
		output_color = texture(imagetex, mytexturecoordinates);
	else output_color = vec4(0.0, 1.0, 0.0, 1.0);
}