#include <fstream>
#include <string>
#include <vector>
#include <iostream>

#include <GL/glew.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    

#include "myShader.h"

using namespace std;

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

// GUI variables
bool quit = false;
int mouse_position[2];
bool button_pressed = false;
int window_height = 863;
int window_width = 1646;

// Projection variables
float fovy = 45.0f;
float znear = 1.0f;
float zfar = 2000.0f;

// Camera variables
glm::vec3 camera_eye = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camera_forward = glm::vec3(0.0f, 0.0f, -1.0f);


void rotate(glm::vec3 & inputvec, glm::vec3 rotation_axis, float theta, bool tonormalize = false)
{
	const float cos_theta = cos(theta);
	const float dot = glm::dot(inputvec, rotation_axis);
	glm::vec3 cross = glm::cross(inputvec, rotation_axis);

	inputvec *= cos_theta;
	inputvec += rotation_axis * dot * (float)(1.0 - cos_theta);
	inputvec -= cross * sin(theta);

	if (tonormalize) inputvec = glm::normalize(inputvec);
}


// Process the event.  
void processEvents(SDL_Event current_event)
{
	switch (current_event.type)
	{
	case SDL_QUIT:
	{
		quit = true;
		break;
	}
	case SDL_KEYDOWN:
	{
		if (current_event.key.keysym.sym == SDLK_ESCAPE)
			quit = true;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		mouse_position[0] = current_event.button.x;
		mouse_position[1] = window_height - current_event.button.y;
		button_pressed = true;
		break;
	}
	case SDL_MOUSEBUTTONUP:
	{
		button_pressed = false;
		break;
	}
	case SDL_MOUSEMOTION:
	{
		if (button_pressed == false) break;

		int x = current_event.motion.x;
		int y = window_height - current_event.motion.y;

		int dx = x - mouse_position[0];
		int dy = y - mouse_position[1];

		if (dx == 0 && dy == 0) break;

		mouse_position[0] = x;
		mouse_position[1] = y;

		float vx = (float)dx / (float)window_width;
		float vy = (float)dy / (float)window_height;
		float theta = 4.0f * (fabs(vx) + fabs(vy));

		glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));

		glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

		glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

		rotate(camera_forward, rotation_axis, theta, true);
		rotate(camera_up, rotation_axis, theta, true);
		rotate(camera_eye, rotation_axis, theta, false);

		break;
	}
	case SDL_MOUSEWHEEL:
	{
		if (current_event.wheel.y < 0)
			camera_eye -= 0.1f * camera_forward;
		else if (current_event.wheel.y > 0)
			camera_eye += 0.1f * camera_forward;
	}
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	// Initialize video subsystem
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	// Using OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// Create window
	window = SDL_CreateWindow("Normals-Drawing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);

	// Initialize glew
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	myShader *shader = new myShader("basic-vertexshader.glsl", "basic-fragmentshader.glsl");
	shader->start();

	vector<glm::vec3> vertices;
	vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	vertices.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

	vector<glm::ivec3> faces;
	faces.push_back(glm::ivec3(1, 2, 3));
	faces.push_back(glm::ivec3(0, 1, 2));
	faces.push_back(glm::ivec3(2, 0, 3));
	faces.push_back(glm::ivec3(1, 0, 3));

	/**************************************************************************/
	vector<glm::vec3> normals;
	normals.push_back(glm::vec3(-1, -1, -1));
	normals.push_back(glm::vec3(1, 0, 0));
	normals.push_back(glm::vec3(0,1,0));
	normals.push_back(glm::vec3(0, 0, 1));
	/**************************************************************************/

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint buffers[3];
	glGenBuffers(3, buffers);

	unsigned int location;

	location = 0;
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(location);

	/**************************************************************************/
	location = 1;
	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(location);
	glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
	/**************************************************************************/

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(glm::ivec3), &faces[0], GL_STATIC_DRAW);

	glBindVertexArray(0);


	// display loop
	while (!quit)
	{
		glViewport(0, 0, window_width, window_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection_matrix = glm::perspective(glm::radians(fovy), static_cast<float>(window_width) / static_cast<float>(window_height), znear, zfar);
		glUniformMatrix4fv(glGetUniformLocation(shader->shaderprogram, "myprojection_matrix"), 1, GL_FALSE, glm::value_ptr(projection_matrix));

		glm::mat4 view_matrix = glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
		glUniformMatrix4fv(glGetUniformLocation(shader->shaderprogram, "myview_matrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
		
		glUniform4fv(glGetUniformLocation(shader->shaderprogram, "input_color"), 1, glm::value_ptr(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)));
		
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(faces.size() * 3), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		
		glUniform4fv(glGetUniformLocation(shader->shaderprogram, "input_color"), 1, glm::value_ptr(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));

		/**************************************************************************/
		glBegin(GL_LINES);
		for (unsigned int i = 0; i < vertices.size(); ++i)
		{
				glm::vec3 v = vertices[i] + glm::normalize(normals[i]);
				glVertex3fv(&vertices[i][0]);
				glVertex3fv(&v[0]);
		}
		glEnd();
		/**************************************************************************/

		SDL_GL_SwapWindow(window);

		SDL_Event current_event;
		while (SDL_PollEvent(&current_event) != 0)
			processEvents(current_event);
	}

	// Destroy window
	if (glContext) SDL_GL_DeleteContext(glContext);
	if (window) SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}