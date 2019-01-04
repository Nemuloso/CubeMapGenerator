#if defined (_DEBUG) && defined (_WIN32)
    #include <vld.h> // memcheck
#endif //Debug
#include <iostream>
#include <string>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "src\cpp\generator.h"

const std::string help = "\n"
"This program converts and saves several cube maps from an\n"
"equirectangular .hdr soure image file.\n"
"It generates the following cube textures and saves each\n"
"as six single images as well in the .hdr format:\n"
"- a cube map to use as background\n"
"- an irradiance map\n"
"- a set of prefiltered environment maps.\n"
"\n"
"Usage: .\hdr_envmap_generator_win.exe [path to equirect image] [optional parameter]\n"
"\n"
"-out [path where to save to]     Define the output path. Default is .\out in the programs root directory.\n"
"-mips [n]                        Number of generated prefiltered maps. Default is 5.\n"
"\n";

int main(int argc, char *argv[])
{
    if (argc == 1) {
        std::cout << help;
        return 0;
    }
    Generator g("../test/test_equirect.hdr");
    g.generateCubeMap();
    g.saveCubeMap();
    g.generateIrradianceMap(64);
    g.saveIrradianceMap();
    g.generateEnvironmentMap();
    g.savePrefilteredEnvMap();

#if defined (_DEBUG) && defined (_WIN32)
    while (!glfwWindowShouldClose(g.getWindow()))
    {
        g.processWindowInput();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // then before rendering, configure the viewport to the original framebuffer's screen dimensions
        glViewport(0, 0, 800, 600);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        g.renderSkybox();

        glfwPollEvents();
    }
#endif //Debug
    return 0;
}