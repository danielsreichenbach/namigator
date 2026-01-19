#!/bin/bash
# Download pre-generated GLAD for OpenGL 3.3 Core
cd "$(dirname "$0")"
curl -o glad.zip "https://glad.dav1d.de/generated/tmpt4kjzgpsglad/glad.zip"
unzip -o glad.zip
mv glad/include/glad/gl.h .
mv glad/src/gl.c glad.c
rm -rf glad glad.zip
echo "GLAD files downloaded: gl.h and glad.c"
