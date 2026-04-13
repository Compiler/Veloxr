# Purpose
- Will render any resolution image by tiling to your graphics drivers max resolution size.
- Tradeoff of space for instant access. I do 0 mipmapping. We want to minimize VRAM here for ML processing.
- Stress tested on 168,000 x 128,000 images, beats photoshop and lightroom for load times. 4ms draw time. 3090 and 4070 and dual 4070 hardware tests. 


# Instuctions at a high level, ping me.

Install make in some way, I used chocolatey
- Or don't and manually run the lines in the makefile 

Install GLFW3 + OpenCV

- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
- bootstrap-vcpkg.bat
- vcpkg install glfw3
- vcpkg install opencv
- Use this install path as a variable in cmake
- Update your Makefile to -D to your glfw install path

## MAC
must set VK_ICD_FILENAMES to where MoltenVK_icd.json is at!

you can set VELOXR_SHADER_PATH to point to the spirv
