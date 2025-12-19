#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <glad/glad.h>

#include "Matrix.h"
#include "Structs.h"

class GLShader
{
public:
	unsigned int id = 0;

	GLShader() = default;
	GLShader(const char* vertPath, const char* fragPath)
	{
		std::string vertCode;
		std::string fragCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vShaderFile.open(vertPath);
			fShaderFile.open(fragPath);

			std::stringstream vShaderStream;
			std::stringstream fShaderStream;

			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertCode = vShaderStream.str();
			fragCode = fShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}

		const char* vShaderCode = vertCode.c_str();
		const char* fShaderCode = fragCode.c_str();

		unsigned int vertex = CreateVertexShader(vShaderCode);
		unsigned int fragment = CreateFragmentShader(fShaderCode);
		CreateShaderProgram(vertex, fragment);
	}

	void Use()
	{
		glUseProgram(id);
	}

	void SetInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	void SetBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}

	void SetVec2(const std::string& name, const Vec2f& value)
	{
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value.x);
	}

	void SetVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
	}

	void SetVec3(const std::string& name, const Vec3f& value) const
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value.x);
	}

	void SetVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}

	void SetVec4(const std::string& name, const Vec4f& value) const
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value.x);
	}

	void SetVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
	}

	// void SetMat2(const std::string& name, const Mat2& mat) const
	// {
	// 	glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	// }

	void SetMat3(const std::string& name, const Mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void SetMat4(const std::string& name, const Mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	unsigned int CreateVertexShader(const char* shaderCode)
	{
		unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &shaderCode, NULL);
		glCompileShader(vertex);

		return vertex;
	}

	unsigned int CreateFragmentShader(const char* shaderCode)
	{
		unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &shaderCode, NULL);
		glCompileShader(fragment);

		return fragment;
	}

	void CreateShaderProgram(unsigned int vert, unsigned int frag)
	{
		id = glCreateProgram();
		glAttachShader(id, vert);
		glAttachShader(id, frag);
		glLinkProgram(id);

		glDeleteShader(vert);
		glDeleteShader(frag);
	}
};
