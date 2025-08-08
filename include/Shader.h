#pragma once
#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm.hpp>

#ifndef SHADER_H
#define SHADER_H


class Shader {
public:
    unsigned int id;

    Shader(const char *vertexPath, const char *fragmentPath) {
        std::string vertexSource;
        std::string fragmentSource;
        std::ifstream vertexFile;
        std::ifstream fragmentFile;

        try {
            vertexFile.open(vertexPath);
            fragmentFile.open(fragmentPath);

            std::stringstream vertexStream, fragmentStream;

            vertexStream << vertexFile.rdbuf();
            fragmentStream << fragmentFile.rdbuf();

            vertexFile.close();
            fragmentFile.close();

            // por que usar str si luego se usara c_str?
            vertexSource = vertexStream.str();
            fragmentSource = fragmentStream.str();
        } catch (std::ifstream::failure &e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }

        // por que usar c_str?
        const char *vertexCode = vertexSource.c_str();
        const char *fragmentCode = fragmentSource.c_str();

        unsigned int vertex, fragment;

        vertex = glCreateShader(GL_VERTEX_SHADER);
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertex, 1, &vertexCode, nullptr);
        glCompileShader(vertex);

        glShaderSource(fragment, 1, &fragmentCode, nullptr);
        glCompileShader(fragment);


        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        glLinkProgram(id);

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() const {
        glUseProgram(id);
    }

    // por qué por referencia?, imagion que para no pasar copia? ya que es const
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        // por qué &mat (&?) si ya se esta pasando como referencia?
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
};


#endif //SHADER_H