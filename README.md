# software rasterizer 📐

A compact C++20 learning project inspired by ssloy’s **tinyrenderer**. It implements a **CPU rasterization** pipeline with a simple TGA image backend and utilities for drawing, loading models, and writing output images. Ideal for experimenting with fundamentals (lines, projections, faces) before scaling to a fuller renderer.

<img src="assets/demo.gif" height=500px alt="Demo rasterization of demon from Diablo 3">

## Run

**Requirements**

- CMake **3.24+**
- A C++20 compiler

Build and run with these commands:

```bash
cmake -S . -B build -G "Ninja"
cmake --build build

# Choose an .obj file to render
./build/rasterizer obj/{file}.obj        # Linux/macOS
./build/rasterizer.exe obj/{file}.obj    # Windows
```

> Output images are written to `assets/output.tga` by default.

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](./LICENSE) file for details.
