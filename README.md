# imgui opengl cmake examples

- cloned from the tamaskenez `imgui-cmake` repo and the conan-io `examples2` repo
https://github.com/conan-io/examples2.git

- build on Rocky Linux 9.4 (Blue Onyx)


- original instructions from imgui-cmake
This project shows how to integrate ImGui for a CMake C++ project. The result can be seen like this screenshot:

![Screenshot](./images/screenshot.png)

# Getting Started

## Prerequisites

On Ubuntu 21.04:

```
sudo apt install -y build-essential cmake libglew-dev libglfw3-dev xorg-dev
```

## Build

```
mkdir build && cd build
cmake ..
make
```

# References

1. https://github.com/ocornut/imgui.git
2. https://github.com/aniongithub/imgui-cmake