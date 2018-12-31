/**
* \class Generator
*
* \author Stefan Hermes
*
* This class provides function to create different source images for certain purposes from an equirectangular HDR image.
* It converts the equirectangular image into six images to usa as cubemap side images. It also creates a diffuse IBL Map and
* specular maps to use in pbr shaders.
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
    Generator(const std::string&);
    Generator(const std::string&, const std::string&);
    ~Generator();

    const std::string & getInFilePath() const;
    void setInFilePath(const std::string&);
    const std::string & getOutPath() const;
    void setOutPath(const std::string&);
    GLFWwindow* getWindow() const;
    void saveCubeMap() const;
    void saveIrradianceMap() const;

    void generateCubeMap();
    void generateIrradianceMap(const int sideWidth);
    void renderDisplay();

    void processWindowInput() const;

private:
    const unsigned int SRC_WIDTH = 800;
    const unsigned int SRC_HEIGHT = 600;
    std::string inFilePath;
    std::string outPath;
    GLFWwindow* window;
    Image HDRsrcImg;
    Shader displayShader, equirectangularToCubemapShader, irradianceShader;
    glm::mat4 captureProjection;

    unsigned int HDRsrcTexture;
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    unsigned int captureFBO;
    unsigned int captureColorbuffer;
    unsigned int captureRBO;
    // TODO: reuse captureFBO?
    unsigned int irradianceFBO;
    unsigned int irradianceColorbuffer;
    unsigned int irradianceRBO;

    void initShader();
    void loadSrcImg();
    void initCubeCapture(unsigned int &fbo, unsigned int &cubeTexture, unsigned int &rbo, int sideWidth);

    void captureCubeFaces(const int sideWidth, const unsigned int fbo, const unsigned int cubeTexture, Shader shader);
    void saveCubeImages(const GLuint texID) const;

    void renderQuad();
    void renderCube();

    friend std::ostream& operator<<(std::ostream& output, const Generator& gen);
};

#endif // GENERATOR_H
