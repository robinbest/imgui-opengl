# Build c++ applications using imgui, cmake and opengl

This repository provides examples to build imgui applications with cmake and opengl. Code is relatively up to date with respect to all libraries and tools used. Note that not all imgui features are built and used.

September 2024

## References
- Yongkie's `imgui cmake starter` [repo](https://gitlab.com/ywiyogo/imgui-cmake-starter.git)

- conan-io's `examples2` [repo](https://github.com/conan-io/examples2.git) and [blog](https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html)

- Three submodules: [imgui (docking branch)](https://github.com/ocornut/imgui.git), [gl3w](https://github.com/skaslev/gl3w.git) and [glfw](https://github.com/glfw/glfw.git). glfw is built on windows only

- [glew-2.1.0 api for windows 64bit](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip), doc and 32 bit library files were removed to reduce size

## Build on Rocky Linux 9.4 (Blue Onyx)

### Prerequisites

Install build tools, such as cmake, gnu c/c++, python. Install additional packages, e.g.

```
    yum install libXrandr-devel libXinerama-devel libXcursor-devel
    yum install glfw glfw-devel glew glew-devel
```

### Build and run
Using gcc v11.4, cmake v3.26

1. Prepare the repository
```
    git clone https://github.com/robinbest/imgui-opengl.git
    cd imgui-opengl
    git submodule init
    git submodule update
```

2. Build
```
    mkdir build
    cd build
    cmake ..
    make -j4
```

3. Run
Stay in the build folder, run e.g. `./bin/imgui-opengl`. If things go well, you'll see the main window with the original title!
![OpenGL Example](./images/opengl0.png)

## Build on Ubuntu 22.04 (Jammy Jellyfinsh)

```
    sudo apt-get install freeglut3-dev libglfw3 libglfw3-dev libglew-dev
```
Follow the same steps as Rocky Linux.

## Build on Windows

### Visual Studio 17 2022

- Same steps to clone the code as Linux
- Run `VS2022 x64 Native Tools Command Prompt`
- Enter the build directory and run `cmake ..` 
- Run `VS2022 IDE` to open the solution file `imgui-opengl.sln` and build. Or you can run `cmake --build . --target ALL_BUILD` to build.

The executables are in `bin/Debug` or `bin/Release` folder. By default, the Debug configuration is built.

### License

Owner of this repository does not claim any copyrights to code from two references. This repo is released under the MIT license.


## Original readme from `imgui cmake starter`

This project shows how to integrate ImGui for a CMake C++ project. The result can be seen like this screenshot:

![CMake Example](./images/screenshot.png)

### Prerequisites

On Ubuntu 21.04:

```
sudo apt install -y build-essential cmake libglew-dev libglfw3-dev xorg-dev
```

### Build

```
mkdir build && cd build
cmake ..
make
```

### References

1. https://github.com/ocornut/imgui.git
2. https://github.com/aniongithub/imgui-cmake