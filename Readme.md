Install make in some way, I used chocolatey

Installed GLFW3

- git clone https://github.com/microsoft/vcpkg.git
- cd vcpkg
- bootstrap-vcpkg.bat
- vcpkg install glfw3
- Use this install path as a variable in cmake
- Update your Makefile to -D to your glfw install path

