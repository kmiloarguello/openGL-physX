#version 330 core

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;

flat in vec3 mynormal;
flat in vec4 myvertex;
 
vec4 kd = vec4(0,1,0,1);
vec4 ks = vec4(1,1,1,1);

vec4 light_color = vec4(1,1,1,1);
uniform vec4 light_position;

out vec4 output_color;

void main (void)
{   
	vec3 eyepos = vec3(0,0,0);

	vec4 _mypos = myview_matrix * myvertex;
	vec3 mypos = _mypos.xyz / _mypos.w;

	vec3 normal = normalize( mynormal_matrix * mynormal );

	vec3 mypos_to_eyepos = normalize(eyepos - mypos);


	vec4 _lightpos = myview_matrix * light_position;
	vec3 lightpos = _lightpos.xyz / _lightpos.w;

	vec3 mypos_to_lightpos; 
	mypos_to_lightpos = normalize(lightpos - mypos);

	//diffuse color
	output_color = light_color * kd * max(dot(normal, mypos_to_lightpos), 0);

	//specular color
	vec3 reflectedray = reflect(-mypos_to_lightpos, normal);
	output_color += light_color * ks * pow(max(dot(reflectedray, mypos_to_eyepos), 0.0), 200);
}