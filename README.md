# Image Processing

Small Windows application written in C++ using [GLFW](https://www.glfw.org/), [Vulkan](https://www.lunarg.com/vulkan-sdk/), and the [Dear ImGui Library](https://github.com/ocornut/imgui/tree/docking) that can open an image file and display to the user.

The intent for this project is to eventually be an image processing application that can perform transformations, effects, and other processing to various image types.

## Development Environment

This program was developed in Visual Studio 2022 using the C++20 standard.

External Dependencies: 
- [Dear ImGui Library](https://github.com/ocornut/imgui/tree/docking)
- [GLFW](https://www.glfw.org/)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)

The Additional Include Directories and Additional Library Directories in the project are setup with project relative paths and paths using environment variables GLFW_DIR, and VULKAN_SDK. Once you have downloaded and installed GLFW and Vulkan, create new environment variables with those names mapped to the install directories of the libraries.

The Dear ImGui library is built with source code files in the project itself.

## Usage

TODO

## Architecture

TODO