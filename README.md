# fractal-explorer

A simple program for visualizing and coloring Mandelbrot fractals.

![Screenshot of the program](README_img.png)

## Controls

- **Pan**: WASD or arrow keys
- **Zoom in**: +
- **Zoom out**: -
- **Toggle UI**: Tab
- **Take screenshot**: Space

## Building

Run the following commands to build the project:

`cmake -B build -DCMAKE_BUILD_TYPE=Release`

`cmake --build build --config Release`

### Dependencies

- SFML 2.6.x
- Lua 5.4.x
- ImGui 1.90.6
