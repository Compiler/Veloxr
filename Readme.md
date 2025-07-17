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