# OpenGL Shaders & Animation

Real-time 3D renderer in **C++ / OpenGL 3 / GLSL** with switchable shading models, an animated scene, a configurable camera and both **perspective and orthographic** projections. Linear algebra (vectors, matrices, transforms) is implemented from scratch in `algebra.cpp`.

## Features

- **4 shader programs**, switchable at runtime:
  1. Basic (colour from normals)
  2. Interlaced (procedural stripe pattern)
  3. **Phong** lighting (ambient + diffuse + specular, per fragment)
  4. **Cartoon / toon shading** (quantised lighting)
- Animated model (continuous rotation, pausable)
- Free camera: translation, rotation and field-of-view control
- Perspective ↔ orthographic projection toggle
- Built-in meshes: bunny, cow, frog, knot, sphere, teapot, triceratops, cube

## Controls

| Key | Action |
|---|---|
| `1` `2` `3` `4` | Basic / interlaced / Phong / cartoon shader |
| `x` `X`, `y` `Y`, `z` `Z` | Move the camera along X / Y / Z |
| `i` `I`, `j` `J`, `k` `K` | Rotate the camera around X / Y / Z |
| `f` `F` | Decrease / increase field of view |
| `P` | Toggle perspective / orthographic projection |
| `p` | Pause / resume the animation |
| `q` | Quit |

## Building

Windows + Visual Studio 2022: open `DVA338_Lab2.slnx`. The **freeglut** and **GLEW** NuGet packages listed in `packages.config` are restored automatically on the first build. Shaders are loaded at runtime from the `DVA338_Lab2/` folder, so run with that folder as the working directory.

## Project structure

```
DVA338_Lab2/
├── main.cpp            # window, input, render loop, shader management
├── algebra.cpp/.h      # vectors, matrices, transforms, projections
├── camera.cpp/.h       # camera model
├── mesh.cpp/.h         # mesh data and normals
├── models/             # built-in meshes
└── shaders/            # GLSL vertex / fragment shaders
```

## Tech

C++ · OpenGL 3 · GLSL · freeglut · GLEW · Visual Studio

*Lab from Fundamentals of Computer Graphics (DVA338) at Mälardalen University, Sweden.*
