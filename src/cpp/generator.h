#ifndef GENERATOR_H
#define GENERATOR_H

// Include standard libraries
#include <iostream>
#include <string>
// Include glad for OpenGL function pointers
#include "glad/glad.h"
// Include GLFW for window context. OpenGL does not work without...
#include "GLFW/glfw3.h"
// Include glm for vector and matrix operations
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
// Include shader class from https://learnopengl.com
#include "learnogl/shader.h"

/**
* A helper structure just to keep things simpler.
**/
struct Image {
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int format;
};

/**
* \class Generator
*
* \author Stefan Hermes
*
* This class provides function to create different source images for different purposes from an equirectangular HDR image.
* It converts the equirectangular image into six images to usa as cubemap side images. It also creates a diffuse IBL Map and
* specular maps to use in pbr shaders.
*
* While programming I used a lot of code originally from https://learnopengl.com. Thanks to the author :-)
*
* To initialize the OpenGL environment the glad library is used.
* For image IO the Developer Image Library is used.
* Since OpenGL does not work without any window context and for debuging purposes I use the GLFW library.
**/
class Generator {
public:
    /**
    * \brief Single parameter constructor
    *
    * This constructor passes its parameter to the two parameter
    * constructor with an additional default output path ./out
    *
    * \param std::string& The input images path
    **/
    Generator(const std::string&);
    /**
    * \brief Two parameter constructor
    *
    * This constructor initializes all variables and the OpenGL context.
    * It also calls the intern image load function.
    *
    * \param std::string& The input images path
    * \param std::string& The path where to store the output
    **/
    Generator(const std::string&, const std::string&);
    /**
    * \brief Destructor
    *
    * Frees all ressources
    **/
    ~Generator();
    /**
    * Sets and creates the output directories.
    **/
    void setOutPath(const std::string&);
    /**
    *
    **/
    void setMaxMipLevels(const int mips);

    /**
    * Converts a eqirectangular environment texture to a cube texture.
    **/
    void generateCubeMap();
    /**
    * Before use generateCubeMap has to be called to generate the cube texture.
    *
    * Generates a convoluted irradiance map from the source texture.
    **/
    void generateIrradianceMap(const int sideWidth);
    /**
    * Before use generateCubeMap has to be called to generate the cube texture.
    *
    * Generates a set of prefiltered images using the Hammersly algorithm.
    **/
    void generateEnvironmentMap();
    /// Save the background texture
    void saveCubeMap() const;
    /// Save the irradiance texture
    void saveIrradianceMap() const;
    /// Save the environment texture
    void savePrefilteredEnvMap() const;

    /// Debug function
    GLFWwindow* getWindow() const;
    /// Debug function
    void renderDisplay();
    /// Debug function
    void renderSkybox();

    /// Glfw callback function
    void processWindowInput() const;

private:
    /// Windows width
    const unsigned int SRC_WIDTH = 800;
    /// Windows height
    const unsigned int SRC_HEIGHT = 600;
    /// The equirectangular .hdr source file.
    std::string inFilePath;
    /// Also the source path. The image library needs it that way.
    WCHAR *w_inFilePath = 0;
    /// Function that manages type conversion.
    //WCHAR getWideCharInPath();
    /// The path where to save to.
    std::string outPath;
    /// Also the out path but as wide char array.
    WCHAR *w_outPath = 0;
    /// Function that manages type conversion.
    //WCHAR getWideCharOutPath();
    /// The filename to save as
    std::string outFileName;
    /// For the window context.
    GLFWwindow* window;
    /// The eqirectangulars source images ID.
    Image HDRsrcImg;
    /// The different shader objects.
    Shader displayShader, equirectangularToCubemapShader, irradianceShader, prefilterEnvironmentShader, skyboxShader;
    /// The projection matrix used to render the cube faces, when rendering the cube textures.
    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);

    /// The eqirectangular texture object created from the src image.
    unsigned int HDRsrcTexture;
    /// The cubes vertex array object.
    unsigned int cubeVAO = 0;
    /// The cubes vertex buffer object bound to cubeVAO.
    unsigned int cubeVBO = 0;
    /// The planes vertex array object. This was used to render a simple plane to the screen for debugging.
    unsigned int quadVAO = 0;
    /// The quads vertex buffer object bound to quadVAO.
    unsigned int quadVBO;
    /// The custom framebuffer to render the different textures.
    unsigned int captureFBO;
    /// The textures ID where the unchanged cube faces are saved in.
    unsigned int captureColorbuffer;
    /// The render buffer object boud to captureFBO.
    unsigned int captureRBO;
    // TODO: reuse captureFBO?
    unsigned int irradianceFBO;
    /// The textures ID where the irradiance map is writen to.
    unsigned int irradianceColorbuffer;
    unsigned int irradianceRBO;

    /// That is the textures ID for the prefiltered environment maps.
    unsigned int environmentColorbuffer;
    /// This determines how many mipmaps are created and which roughness values are used for wvery one.
    unsigned int maxMipLevels = 6;

    /// Initializes all Shader objects.
    void initShader();
    /// Creates the src image object.
    void loadSrcImg();
    /**
    * Initializes a framebuffer object and a texture object without mipmaps to write the render results to.
    *
    * \param unsigned int &fbo Stores the newly crated framebuffers ID.
    * \param unsigned int &cubeTexture Stores the newly crated textures ID.
    * \param unsigned int &rbo Stores the newly crated rbo ID. TODO!
    * \param int sideWidth The cubes side with and height. This will become the textures dimensions.
    **/
    void initCubeCapture(unsigned int &fbo, unsigned int &cubeTexture, unsigned int &rbo, int sideWidth);

    /**
    * Renders the cube faces and stores them into the given texture.
    *
    * \param const int sideWidth The images dimensions
    * \param const unsigned int fbo The framebuffers ID where to render the images to.
    * \param const unsigned int cubeTextures The cube texture ID where to render the images to.
    * \param Shader shader The shader object to use for the cubes faces while rendering.
    **/
    void captureCubeFaces(const int sideWidth, const unsigned int fbo, const unsigned int cubeTexture, Shader shader);
    /**
    * Save the renderings to disk.
    *
    * \param const unsigned int texID The cubeTexture ID where the images data is.
    **/
    void saveCubeImages(const GLuint texID, const std::string name, const std::string subDir = "") const;

    /// Helper to display a 2d texture.
    void renderQuad();
    /// Called every time a the cube has to be rendered to capture one of its faces.
    void renderCube();

    /// From the beginning when I checked if everything worked fine.
    friend std::ostream& operator<<(std::ostream& output, const Generator& gen);
};

#endif // GENERATOR_H
