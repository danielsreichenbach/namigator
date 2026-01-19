# MapViewerSDL - Cross-Platform Navigation Mesh Viewer

Cross-platform visualization tool for namigator navigation meshes using SDL3 and OpenGL 3.3.

## Features

- **Cross-platform**: Works on Linux, Windows, and macOS
- **Full 3D visualization**: Terrain, water, buildings (WMO), objects (doodads), and navigation meshes
- **Interactive pathfinding**: Click to set start/end points and visualize paths
- **Camera controls**: Free-look camera with WASD movement and mouse look
- **Debug information**: Height queries, zone/area lookup, terrain information
- **Rendering options**: Toggle different geometry layers, wireframe mode
- **Modern UI**: Dear ImGui-based interface

## Prerequisites

### Linux (Debian/Ubuntu)
```bash
sudo apt-get install libsdl3-dev libgl1-mesa-dev libglu1-mesa-dev libglm-dev
```

### Arch Linux
```bash
sudo pacman -S sdl3 mesa glm
```

### macOS
```bash
brew install sdl3 glm
```

### Windows
SDL3, GLM, and Dear ImGui will be automatically fetched by CMake via FetchContent.

## Building

The viewer is automatically built if SDL3 is found:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If SDL3 is not found, you'll see a message and the viewer will be skipped. Install SDL3 and reconfigure to enable it.

### Manual SDL3 Installation

If CMake can't find SDL3, you can build it from source:

```bash
git clone https://github.com/libsdl-org/SDL.git -b SDL3
cd SDL
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build
```

## Usage

```bash
MapViewerSDL <nav_data_path> <wow_data_path>
```

**Example:**
```bash
./MapViewerSDL /path/to/generated/nav/data /path/to/wow/Data
```

### Arguments

- `nav_data_path`: Directory containing generated `.nav` and `.bvh` files (output from MapBuilder)
- `wow_data_path`: World of Warcraft `Data` directory containing MPQ archives

## Controls

### Camera Movement
- **W/S**: Move forward/backward
- **A/D**: Move left/right
- **Q/E**: Move up/down (world Z-axis)
- **Space/X**: Move vertically along camera up vector
- **Right-click + drag**: Look around (pan camera)
- **Mouse wheel**: Zoom in/out

### Interaction
- **Left-click**: Set pathfinding start point (first click) or end point (second click)
- **Shift + Left-click**: Show terrain information (coordinates, zone/area, heights)

### UI Controls
- **Map dropdown**: Select which WoW map to load
- **X/Y inputs**: ADT coordinates or world coordinates
- **Load ADT**: Load the specified ADT tile
- **Z Search**: Find all possible Z heights at the specified X,Y position
- **Checkboxes**: Toggle rendering of different geometry types
- **Wireframe**: Toggle wireframe rendering mode

## Features Comparison with DirectX MapViewer

### Implemented Features
✅ All core visualization (terrain, liquid, WMO, doodads, navigation mesh)
✅ Full camera controls (WASD, mouse look, zoom)
✅ Pathfinding visualization
✅ Height queries and Z-search
✅ Zone/area lookup
✅ Wireframe mode
✅ Layer toggles
✅ Modern ImGui interface
✅ Cross-platform support

### Platform Support
- **Linux**: ✅ Full support
- **Windows**: ✅ Full support
- **macOS**: ✅ Full support (OpenGL 3.3 or newer required)

### Architecture Differences from DirectX Viewer

| Feature | DirectX Viewer | SDL3 Viewer |
|---------|----------------|-------------|
| **Graphics API** | DirectX 11 | OpenGL 3.3 |
| **Platform** | Windows only | Linux/Windows/macOS |
| **UI Framework** | Windows Common Controls | Dear ImGui |
| **Windowing** | Win32 API | SDL3 |
| **Shaders** | HLSL | GLSL 3.30 |
| **Build System** | CMake + fxc.exe | CMake + FetchContent |

## Technical Details

### Rendering Pipeline

1. **Geometry buffering**: All geometry (terrain, WMO, doodads) is buffered in OpenGL VAOs
2. **Shader-based rendering**: Simple vertex/fragment shaders with basic directional lighting
3. **Transparency handling**: Opaque geometry rendered first, then transparent (liquids, nav mesh)
4. **Ray-casting**: Mouse picking uses Möller-Trumbore ray-triangle intersection

### Coordinate System

Uses World of Warcraft's coordinate system:
- X-axis: East-West
- Y-axis: North-South
- Z-axis: Vertical (up)

ADT coordinates: 64×64 grid, each ADT is 533.33 yards square.

### Performance

- **Lazy loading**: Geometry uploaded to GPU on first render
- **Culling**: Back-face culling enabled
- **Depth testing**: Z-buffer for proper occlusion
- **Alpha blending**: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

## Troubleshooting

### "SDL3 not found" during CMake configuration

Install SDL3 system-wide or set `SDL3_DIR` to point to your SDL3 installation:
```bash
cmake -B build -DSDL3_DIR=/path/to/SDL3/install/lib/cmake/SDL3
```

### Black screen or no geometry visible

- Ensure navigation data was generated with MapBuilder
- Check console output for errors loading ADTs
- Try a different map (some maps may not have data for all coordinates)
- Verify WoW data path contains valid MPQ archives

### Camera controls not working

- Make sure the main window (not ImGui UI) has focus
- Check that mouse is not hovering over ImGui windows when trying to pan

### Low frame rate

- Reduce the number of loaded ADTs
- Disable wireframe mode
- Disable liquid rendering
- Close/minimize the ImGui window

## Development Notes

### Adding New Features

The codebase is organized as follows:

- `main.cpp`: Application loop, input handling, ImGui UI
- `Renderer.hpp/cpp`: OpenGL rendering, geometry management
- `Camera.hpp/cpp`: Camera transformations, mouse picking
- `DetourDebugDraw.hpp/cpp`: Adapter for Detour's debug visualization

### Extending the Renderer

To add new geometry types:

1. Add entry to `Geometry` enum in `Renderer.hpp`
2. Add corresponding flag to `GeometryFlags`
3. Create `Add*()` method similar to `AddTerrain()`, `AddWmo()`, etc.
4. Handle rendering in `Render()` method

### Shader Modifications

Shaders are embedded as C strings in `Renderer.cpp`:
- `vertexShaderSource`: GLSL vertex shader (transforms vertices, passes normals)
- `fragmentShaderSource`: GLSL fragment shader (lighting calculation)

Modify these strings to change rendering appearance.

## License

Same license as the parent namigator project. See top-level LICENSE file.

## Credits

- **SDL3**: Cross-platform windowing and input
- **Dear ImGui**: Immediate-mode GUI
- **GLM**: OpenGL mathematics library
- **Recast/Detour**: Navigation mesh library (from parent project)
- **Original MapViewer**: DirectX implementation by namreeb
