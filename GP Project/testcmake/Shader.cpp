//
//  Shader.cpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Shader.hpp"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>              // Include GLM core
#include <glm/gtc/matrix_transform.hpp> // Include GLM transformations if needed

namespace gps {
    std::string Shader::readShaderFile(std::string fileName) {

        std::ifstream shaderFile;
        std::string shaderString;
        
        //open shader file
        shaderFile.open(fileName);
        
        std::stringstream shaderStringStream;
        
        //read shader content into stream
        shaderStringStream << shaderFile.rdbuf();
        
        //close shader file
        shaderFile.close();
        
        //convert stream into GLchar array
        shaderString = shaderStringStream.str();
        return shaderString;
    }
    
    void Shader::shaderCompileLog(GLuint shaderId) {

        GLint success;
        GLchar infoLog[512];
        
        //check compilation info
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

        if(!success) {

            glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
            std::cout << "Shader compilation error\n" << infoLog << std::endl;
        }
    }
    
    void Shader::shaderLinkLog(GLuint shaderProgramId) {

        GLint success;
        GLchar infoLog[512];
        
        //check linking info
        glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "Shader linking error\n" << infoLog << std::endl;
        }
    }
    void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
        GLint location = glGetUniformLocation(this->shaderProgram, name.c_str());
        if (location == -1) {
            std::cerr << "WARNING: Uniform '" << name << "' doesn't exist or is not used in the shader." << std::endl;
        }
        glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }

    void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
        GLint location = glGetUniformLocation(this->shaderProgram, name.c_str());
        if (location == -1) {
            std::cerr << "WARNING: Uniform '" << name << "' doesn't exist or is not used in the shader." << std::endl;
        }
        glUniform3fv(location, 1, &value[0]);
    }
    
    void Shader::loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName) {

        //read, parse and compile the vertex shader
        std::string v = readShaderFile(vertexShaderFileName);
        const GLchar* vertexShaderString = v.c_str();
        GLuint vertexShader;
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
        glCompileShader(vertexShader);
        //check compilation status
        shaderCompileLog(vertexShader);
        
        //read, parse and compile the vertex shader
        std::string f = readShaderFile(fragmentShaderFileName);
        const GLchar* fragmentShaderString = f.c_str();
        GLuint fragmentShader;
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
        glCompileShader(fragmentShader);
        //check compilation status
        shaderCompileLog(fragmentShader);
        
        //attach and link the shader programs
        this->shaderProgram = glCreateProgram();
        glAttachShader(this->shaderProgram, vertexShader);
        glAttachShader(this->shaderProgram, fragmentShader);
        glLinkProgram(this->shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        //check linking info
        shaderLinkLog(this->shaderProgram);
    }
    
    void Shader::useShaderProgram() {

        glUseProgram(this->shaderProgram);
    }

}
