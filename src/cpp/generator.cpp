// Include own header
#include "./generator.h"
#include "./constants.h"
// Include glad for OpenGL function pointers
#include "glad/glad.h"
// Include GLFW for window context. OpenGL does not work without...
#include "GLFW/glfw3.h"
// Include glm for vector and matrix operations
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
// Include Developers Image Library (DevIL)
#ifndef ILUT_USE_OPENGL
    // This MUST be defined before calling the DevIL headers or we don't get OpenGL functionality
    #define ILUT_USE_OPENGL
#endif
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

// Forward declaration... GLFW does not like the callback inside the class structure.
void window_size_callback(GLFWwindow* window, int width, int height);

Generator::Generator(const std::string& in) : Generator(in, "./out") {}

Generator::Generator(const std::string& in, const std::string& out) {
    this->captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    
    this->setInFilePath(in);
    this->setOutPath(out);

    // glfw: initialize and configure
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW" << std::endl;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    this->window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, window_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
        iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
        ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION) {
        printf("DevIL version is different...exiting!\n");
    }

    ilInit();

    this->initShader();
    this->loadSrcImg();
}

Generator::~Generator() {
    glfwTerminate();
}

const std::string & Generator::getInFilePath() const {
    return this->inFilePath;
}

void Generator::setInFilePath(const std::string& in) {
    this->inFilePath = in;
    // TODO: This causes a crash
    //this->loadSrcImg();
}

const std::string & Generator::getOutPath() const {
    return this->outPath;
}

void Generator::setOutPath(const std::string& out) {
    this->outPath = out;
}

GLFWwindow* Generator::getWindow() const {
    return this->window;
}

void Generator::saveCubeMap() const {
    saveCubeImages(captureColorbuffer);
}

void Generator::saveIrradianceMap() const {
    saveCubeImages(irradianceColorbuffer);
}

std::ostream& operator<<(std::ostream& output, const Generator& gen) {
    output << gen.getInFilePath() << std::endl;
    output << gen.getOutPath() << std::endl;
    return output;
}

void Generator::loadSrcImg() {
    ilGenImages(1, &HDRsrcImg.id);
    ilBindImage(HDRsrcImg.id);

    ilLoad(IL_HDR, (const char*)"../test/test_equirect.hdr");

    HDRsrcImg.width = ilGetInteger(IL_IMAGE_WIDTH);
    HDRsrcImg.height = ilGetInteger(IL_IMAGE_HEIGHT);
    HDRsrcImg.format = ilGetInteger(IL_IMAGE_FORMAT);

    // If the image is flipped (i.e. upside-down and mirrored, flip it the right way up!)
    ILinfo ImageInfo;
    iluGetImageInfo(&ImageInfo);
    if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
    {
        iluFlipImage();
    }

    ilutRenderer(ILUT_OPENGL);
    ilutEnable(ILUT_OPENGL_CONV);

    glGenTextures(1, &HDRsrcTexture);
    glBindTexture(GL_TEXTURE_2D, HDRsrcTexture);
    // Specify the texture specification
    glTexImage2D(GL_TEXTURE_2D, // Type of texture
        0,// Pyramid level (for mip-mapping) - 0 is the top level
        GL_RGB16F,// Internal pixel format to use. We need a floating point buffer for HDR
        HDRsrcImg.width,// Image width
        HDRsrcImg.height,// Image height
        0,// Border width in pixels (can either be 1 or 0)
        HDRsrcImg.format,// Format of image pixel data
        GL_FLOAT,// Image data type
        ilGetData());// The actual image data itself

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (ilGetError() != IL_NO_ERROR) {
        std::cout << "Image load error!" << std::endl;
    }
    else {
        this->initCubeCapture(this->captureFBO, this->captureColorbuffer, this->captureRBO, this->HDRsrcImg.width / 4);
    }
}

void Generator::initShader() {
    this->displayShader = Shader("./glsl/texture.vert.glsl", "./glsl/texture.frag.glsl", nullptr);
    this->equirectangularToCubemapShader = Shader("./glsl/gen.vert.glsl", "./glsl/gen.frag.glsl", nullptr);
    this->irradianceShader = Shader("./glsl/gen.vert.glsl", "./glsl/diffuseIBL.frag.glsl", nullptr);
}

void Generator::initCubeCapture(unsigned int &fbo, unsigned int &cubeTexture, unsigned int &rbo, int sideWidth) {
    // create a new Framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // create a color attachment texture
    glGenTextures(1, &cubeTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, sideWidth, sideWidth, 0, GL_RGB, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // create a renderbuffer object (we won't be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, sideWidth, sideWidth);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rbo);
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    // return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Generator::renderDisplay(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->displayShader.use();
    glViewport(0, 0, SRC_WIDTH, SRC_HEIGHT);
    glBindTexture(GL_TEXTURE_2D, this->HDRsrcTexture);
    renderQuad();
    glfwSwapBuffers(this->window);
}

void Generator::generateCubeMap() {
    this->equirectangularToCubemapShader.use();
    this->equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    this->equirectangularToCubemapShader.setMat4("projection", captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, HDRsrcTexture);

    const int sideWidth = this->HDRsrcImg.width / 4;

    captureCubeFaces(sideWidth, this->captureFBO, this->captureColorbuffer, this->equirectangularToCubemapShader);
}

void Generator::generateIrradianceMap(const int sideWidth) {
    initCubeCapture(this->irradianceFBO, this->irradianceColorbuffer, this->irradianceRBO, sideWidth);

    this->irradianceShader.use();
    this->irradianceShader.setInt("environmentMap", 0);
    this->irradianceShader.setMat4("projection", this->captureProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->captureColorbuffer);

    captureCubeFaces(sideWidth, this->irradianceFBO, this->irradianceColorbuffer, this->irradianceShader);
}

void Generator::captureCubeFaces(const int sideWidth, const unsigned int fbo, const unsigned int cubeTexture, Shader shader) {
    //Before drawing
    glViewport(0, 0, sideWidth, sideWidth);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // render
    for (int i = 0; i < 6; ++i)
    {
        shader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeTexture, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
}

// renderCube() renders a 1x1 3D cube in NDC.
void Generator::renderCube() {
    // initialize (if necessary)
    if (this->cubeVAO == 0)
    {
        glGenVertexArrays(1, &this->cubeVAO);
        glGenBuffers(1, &this->cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, this->cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(this->cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(this->cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
void Generator::renderQuad() {
    if (quadVAO == 0)
    {
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        // Define positions location
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        // Define uv location
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Generator::saveCubeImages(const GLuint texId) const {

    for (int i = 0; i < 6; ++i) {
        GLint width, height, internalFormat;
        std::string fileName = this->outPath + "/myHDRside_" + std::to_string(i) + ".hdr";

        glBindTexture(GL_TEXTURE_CUBE_MAP, texId);
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat); // get internal format type of GL texture
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_WIDTH, &width); // get width of GL texture
        glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_TEXTURE_HEIGHT, &height); // get height of GL texture

        if (internalFormat == GL_RGB16F)
        {
            auto size = width * height * 3;
            auto buffer = new float[size];
            glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, GL_FLOAT, buffer);

            // Save to an Image
            ILuint SRCeenshot = 0;
            ilGenImages(1, &SRCeenshot);
            ilBindImage(SRCeenshot);
            ilTexImage(width, height, 0, 3, IL_RGB, IL_FLOAT, buffer);
            iluFlipImage();
            ilSave(IL_HDR, fileName.c_str());

            delete[] buffer;
        }
        else {
            std::cout << "ERROR: No HDR Image. Format: " << std::to_string(internalFormat) << std::endl;
        }
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void Generator::processWindowInput() const {
    if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(this->window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void window_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
