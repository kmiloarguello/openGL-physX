#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>


#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    

#include "myShader.h"

using namespace std;

myShader::myShader(string file_vertexshader, string file_fragmentshader)
{
	text_to_id.clear();
	clear();

	vertex_shader = _initShader(GL_VERTEX_SHADER, file_vertexshader);
	fragment_shader = _initShader(GL_FRAGMENT_SHADER, file_fragmentshader);

	if (!_initProgram())
		cout << "Error: shader not initialized properly.\n";
}

myShader::~myShader()
{
	clear();
}

GLuint myShader::_initShader(GLenum type, string filename)
{
	ifstream fin(filename);

	if (!fin)
	{
		cerr << "Unable to Open File " << filename << "\n";
		throw 2;
	}
	string shadertext = string((istreambuf_iterator<char>(fin)), (istreambuf_iterator<char>()));
	const GLchar* shadertext_cstr = shadertext.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shadertext_cstr, NULL);

	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		_shaderErrors(shader);
		throw 3;
	}
	return shader;
}

bool myShader::_initProgram()
{
	shaderprogram = glCreateProgram();
	glAttachShader(shaderprogram, vertex_shader);
	glAttachShader(shaderprogram, fragment_shader);
	glLinkProgram(shaderprogram);

	GLint linked;
	glGetProgramiv(shaderprogram, GL_LINK_STATUS, &linked);
	if (linked) glUseProgram(shaderprogram);
	else {
		_programErrors(shaderprogram);
		return false;
	}
	return true;
}

void myShader::_programErrors(const GLint program) {
	GLint length;
	GLchar * log;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	log = new GLchar[length + 1];
	glGetProgramInfoLog(program, length, &length, log);
	cout << "Compile Error, Log Below\n" << log << "\n";
	delete[] log;
}

void myShader::_shaderErrors(const GLint shader) {
	GLint length;
	GLchar * log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	log = new GLchar[length + 1];
	glGetShaderInfoLog(shader, length, &length, log);
	cout << "Compile Error, Log Below\n" << log << "\n";
	delete[] log;
}

void myShader::clear()
{
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(shaderprogram);
}

void myShader::start() const
{
	glUseProgram(shaderprogram);
}

void myShader::stop() const
{
	glUseProgram(0);
}

GLint myShader::getUniformLocation(string text) 
{
	if (text_to_id.find(text) == text_to_id.end())
	{
		int location = glGetUniformLocation(shaderprogram, text.c_str());
		if (location == -1)
		{
			//cerr << "Error: unable to get location of variable with name: " << text << endl;
			return -1;
		}
		else text_to_id[text] = location;
	}

	return text_to_id[text];
}

void myShader::setUniform(std::string name, glm::mat4 mat)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void myShader::setUniform(std::string name, glm::mat3 mat)
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void myShader::setUniform(std::string name, float val)
{
	glUniform1f(getUniformLocation(name), val);
}

void myShader::setUniform(std::string name, int val)
{
	glUniform1i(getUniformLocation(name), val);
}

void myShader::setUniform(std::string name, glm::vec2 vec)
{
	glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void myShader::setUniform(std::string name, glm::vec3 vec)
{
	glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void myShader::setUniform(std::string name, glm::vec4 vec)
{
	glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void myShader::setUniform(std::string name, vector<glm::vec3> input_array)
{
	for (unsigned int i = 0; i < input_array.size(); ++i) {
		glUniform3fv(getUniformLocation(name + "[" + std::to_string(i) + "]"), 1, &input_array[i][0]);
	}
}
