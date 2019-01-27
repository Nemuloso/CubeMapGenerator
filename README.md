# Environment Map Generator
## Description
This tool is designed to provide a simple way converting equirectangular HDR maps to a set of IBL cubemaps.
The created cubemaps are saved as sets of six single images each. That way they are ready to use in the [three.js](https://github.com/mrdoob/three.js/) framework.


## Overview

* Installation
  * Using pre-built binaries(TODO)
  * [Building from source](https://github.com/Nemuloso/CubeMapGenerator#building-it-from-source)
  * [Troubleshooting](https://github.com/Nemuloso/CubeMapGenerator#troubleshooting)

### Building from source

Prerequisites:
* OpenGL( Is most likely already installed. )
* Visual Studio(VS) for its Compiler.

1. Download the project and open it with VS through doubleclicking the *.sln file in the projects root directory.
2. Change the Project configuration in the top bar to "Release" and "x86".
3. It should be possible to just hit "Build"->"Build Solution" and a Folder with the name Release should appear in the projects directory.

### Troubleshooting

1. Before building the project, check the following settings.
2. Navigate in the topmenu to "Project"->"hdr_envmap_generator-Properties".
3. Choose on the left side "Configuration Properties"->"General" and change the encoding to "Not Set".
4. Choose "Linker"->"Input" and add the following libraries to the additional dependencies:
opengl32.lib
glfw3.lib
DevIL.lib
ILU.lib
ILUT.lib