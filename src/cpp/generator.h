/**
* \class Generator
*
* \author Stefan Hermes
*
* This class provides function to render and save convoluted, scaled
* images. Theese can be used as mipmaps to determine the roughness in
* PBR shaders.
**/
#ifndef GENERATOR_H
#define GENERATOR_H

// Include standard libraries
#include <iostream>
#include <string>
// Include glad for OpenGL function pointers
#include "glad/glad.h"
// Include GLFW for window context. OpenGL does not work without...
#include "GLFW/glfw3.h"
// Include shader class from https://learnopengl.com
#include "learnogl/shader.h"

struct Image {
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int format;
};

class Generator {
public:
    Generator();
    Generator(const std::string&);
    Generator(const std::string&, const std::string&);
    ~Generator();

    const std::string & getInFilePath() const;
    void setInFilePath(const std::string&);
    const std::string & getOutPath() const;
    void setOutPath(const std::string&);
    GLFWwindow* getWindow() const;
    Image getHDRsrcImg() const;

    void captureCubeFaces();
    void loadSrcImg();
    void renderDisplay();

    void processWindowInput() const;

private:
    const unsigned int SRC_WIDTH = 800;
    const unsigned int SRC_HEIGHT = 600;
    std::string inFilePath;
    std::string outPath;
    GLFWwindow* window;
    Image HDRsrcImg;
    Shader displayShader, equirectangularToCubemapShader;

    unsigned int HDRsrcTexture;
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    unsigned int captureFBO;
    unsigned int captureColorbuffer;
    unsigned int captureRBO;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    void initCubeCapture();
    void initShader();

    void renderQuad();
    void renderCube();

    void saveCubeImages(const GLuint texID) const;

    friend std::ostream& operator<<(std::ostream& output, const Generator& gen);
};

#endif // GENERATOR_H
