#if defined (_DEBUG) && defined (_WIN32)
    #include <vld.h> // memcheck
#endif
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "src\cpp\generator.h"

int main()
{
    Generator g("../test/test_equirect.hdr");
    g.generateCubeMap();
    g.generateIrradianceMap(32);
    //g.saveCubeMap();
    g.saveIrradianceMap();
    g.renderDisplay();
    while (!glfwWindowShouldClose(g.getWindow()))
    {
        g.processWindowInput();
        glfwPollEvents();
    }

    return 0;
}