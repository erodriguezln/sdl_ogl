#pragma once
#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm.hpp>

#ifndef SHADER_H
#define SHADER_H

// TODO separar en cpp/h y generar log and check errors
class Shader {
public:
    unsigned int id;
    unsigned int vertex;
    unsigned int fragment;

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

        compileVertexShader(vertexCode);
        compileFragmentShader(fragmentCode);
        createShaderProgram();
    }

    void compileVertexShader(const char *vertexCode) {
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexCode, nullptr);
        glCompileShader(vertex);
    }

    void compileFragmentShader(const char *fragmentCode) {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentCode, nullptr);
        glCompileShader(fragment);
    }

    void createShaderProgram() {
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

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        // por qué &mat (&?) si ya se esta pasando como referencia?
        glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id, name.c_str()), value);
    }
};


#endif //SHADER_H