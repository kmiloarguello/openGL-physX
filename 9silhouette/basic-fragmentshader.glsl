#version 330 core

uniform vec4 input_color;
out vec4 output_color;

in vec4 myvertex;
in vec3 mynormal;

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

void main (void)
{   
	vec3 eyepos = vec3(0,0,0);

    vec4 _mypos = myview_matrix * myvertex;
    vec3 mypos = (_mypos.xyz) / _mypos.w;

    vec3 normal = normalize(mynormal_matrix * mynormal);

    vec3 mypos_to_eyepos = normalize(eyepos - mypos);

    if (  abs(dot(normal, mypos_to_eyepos)) < 0.1 ) 
		output_color = vec4(1,0,0,0);
	else output_color = vec4(0,0,0,0);
}