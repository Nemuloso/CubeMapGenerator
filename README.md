# Environment Map Generator

## Description
This tool is designed to provide a simple way converting equirectangular HDR maps to a set of IBL cubemaps.
The created cubemaps are saved as sets of six single images each. That way they are ready to use in the [three.js](https://github.com/mrdoob/three.js/) framework.

## Overview

* Installation
  * Using pre-built binaries(TODO)
  * [Building from source](https://github.com/Nemuloso/CubeMapGenerator#building-it-from-source)
  * [Usage](https://github.com/Nemuloso/CubeMapGenerator#usage)
  * [Troubleshooting](https://github.com/Nemuloso/CubeMapGenerator#troubleshooting)

### Building from source

Prerequisites:
* OpenGL (Is most likely already installed.)
* Visual Studio(VS) for its Compiler.

Download the project and open it with VS through doubleclicking the *.sln file in the projects root directory.
Change the Project configuration in the top bar to "Release" and "x86".
It should be possible to just hit "Build"->"Build Solution" and a Folder with the name Release should appear in the projects directory.

### Usage

In the Release folder should be an executeable file. Navigate with the console to this folder and call the file as following:

.\hdr_envmap_generator_win.exe \[path to equirect image\] \[optional parameter\]

The program just supports *.hdr files. It outputs to a defined folder or generates an "./out" folder in the Release Folder. The square size of the
resulting cubemap sides is 1/4th of the width of the original image. Optional parameters are shown below.

| Argument | Description |
| ------ | ------ |
| -out \[path where to save to\] | Define the output path. Default is .\out in the programs root directory. |
| -mips \[n\]                    | Number of generated prefiltered maps. Default is 6. |
| -irr_res \[n\]                 | Resolution of the irradiance maps squares. Default is 64. |

### Troubleshooting

* Update your graphics drivers

* Before building the project, check the following settings:
  * Navigate in the topmenu to "Project"->"hdr_envmap_generator-Properties".
  * Choose on the left side "Configuration Properties"->"General" and change the encoding to "Not Set".
  * Choose "Linker"->"Input" and add the following libraries to the additional dependencies:
opengl32.lib
glfw3.lib
DevIL.lib
ILU.lib
ILUT.lib