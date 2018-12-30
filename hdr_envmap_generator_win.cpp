#include <vld.h> // memcheck
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "src\cpp\generator.h"

int srcImgWidth, srcImgHeight, srcImgFormat;

int main()
{
    Generator g("../test/test_equirect.hdr");
    g.loadSrcImg();
    g.captureCubeFaces();
    while (!glfwWindowShouldClose(g.getWindow()))
    {
        // input
        g.processWindowInput();
        glfwPollEvents();
    }

    return 0;
}