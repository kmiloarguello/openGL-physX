#version 330 core

layout(location = 0) in vec4 vertex_modelspace;
layout(location = 1) in vec3 normal_modelspace;

uniform mat4 myview_matrix;
uniform mat3 mynormal_matrix;
uniform mat4 myprojection_matrix;

vec4 kd = vec4(0,1,0,1);
vec4 ks = vec4(1,1,1,1);

vec4 light_color = vec4(1,1,1,1);
uniform vec4 light_position;

out vec4 mycolor;

void main (void)
{   
    gl_Position = myprojection_matrix * myview_matrix * vertex_modelspace;
	

	vec3 eyepos = vec3(0,0,0);

	vec4 _mypos = myview_matrix * vertex_modelspace;
	vec3 mypos = _mypos.xyz / _mypos.w;

	vec3 normal = normalize( mynormal_matrix * normal_modelspace );

	vec3 mypos_to_eyepos = normalize(eyepos - mypos);
	
	vec4 _lightpos = myview_matrix * light_position;
	vec3 lightpos = _lightpos.xyz / _lightpos.w;

	vec3 mypos_to_lightpos; 
	mypos_to_lightpos = normalize(lightpos - mypos);

	//diffuse color
	mycolor = light_color * kd * max(dot(normal, mypos_to_lightpos), 0);

	//specular color
	vec3 reflectedray = reflect(-mypos_to_lightpos, normal);
	mycolor += light_color * ks * pow(max(dot(reflectedray, mypos_to_eyepos), 0.0), 200);
}