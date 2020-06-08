#version 330 core

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

in vec4 myvertex;
in vec3 mynormal;
 
out vec4 color;

uniform samplerCube cubetex;
 
void main (void)
{   
	vec4 _mypos = myview_matrix * myvertex;
	vec3 mypos = (_mypos.xyz) / _mypos.w;

	vec3 eyepos = vec3(0,0,0);

    vec3 normal = normalize(mynormal_matrix * mynormal);

	vec3 mypos_to_eyepos = normalize(eyepos - mypos);
	
	vec3 reflection = reflect(-mypos_to_eyepos, normal);
	color = texture(cubetex, inverse(mynormal_matrix) * reflection);

	color.a = 1.0f;
}
