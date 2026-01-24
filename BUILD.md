# YEFS Build Instructions

YEFS (无人机地面站系统) is a GIS platform for drone ground station systems built with Qt6, QML, and MapLibre.

## Prerequisites

### Required Software

- **CMake** >= 3.16
- **Qt6** >= 6.5
  - Qt6 Quick
  - Qt6 Location
  - Qt6 Positioning
- **C++ Compiler** with C++17 support
  - GCC 7+ (Linux)
  - Clang 5+ (macOS)
  - MSVC 2019+ (Windows)
- **Git** (for submodules)

### Platform-Specific Requirements

#### Linux
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev qt6-declarative-dev qt6-positioning-dev qt6-location-dev cmake build-essential git

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtpositioning-devel qt6-qtlocation-devel cmake gcc-c++ git
```

#### macOS
```bash
# Using Homebrew
brew install qt@6 cmake
```

#### Windows
1. Install Qt6 from [Qt Online Installer](https://www.qt.io/download-qt-installer)
2. Install CMake from [cmake.org](https://cmake.org/download/)
3. Install Visual Studio 2019 or later with C++ development tools

## Building YEFS

### 1. Clone the Repository

```bash
git clone https://github.com/ChenmyBBQ/YEFS.git
cd YEFS
```

### 2. Initialize Submodules

The project uses HuskarUI and MapLibre as third-party dependencies:

```bash
git submodule update --init --recursive
```

This will download:
- HuskarUI - UI component library
- MapLibre Native Qt - Map rendering engine

### 3. Configure the Build

```bash
mkdir build
cd build
cmake ..
```

For Release build:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### 4. Build the Project

```bash
cmake --build .
```

Or with specific build system:
```bash
# Using make (Linux/macOS)
make -j$(nproc)

# Using Visual Studio (Windows)
cmake --build . --config Release
```

### 5. Run the Application

```bash
# Linux/macOS
./out/Debug/bin/YEFS

# Windows
.\out\Debug\bin\YEFS.exe
```

## Build Configuration

### CMake Options

- `CMAKE_BUILD_TYPE` - Build type (Debug, Release, RelWithDebInfo, MinSizeRel)
- `BUILD_HUSKARUI_STATIC_LIBRARY` - Build HuskarUI as static library (default: OFF)
- `MLN_QT_WITH_LOCATION` - Build MapLibre with Qt Location support (default: ON)
- `MLN_QT_WITH_QUICK_PLUGIN` - Build MapLibre QML plugin (default: ON)

Example:
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_HUSKARUI_STATIC_LIBRARY=ON ..
```

## Project Structure

```
YEFS/
├── 3rdparty/              # Third-party dependencies
│   ├── HuskarUI/          # UI component library
│   └── maplibre-native-qt/  # Map rendering engine
├── src/
│   ├── core/              # Core framework
│   │   ├── Application.*  # Application lifecycle
│   │   ├── MessageBus.*   # Inter-module communication
│   │   ├── PluginManager.* # Plugin management
│   │   ├── SettingsManager.* # Settings persistence
│   │   └── MapLibreEngine.* # Map engine wrapper
│   ├── cpp/               # Legacy C++ code
│   ├── qml/               # QML UI files
│   │   ├── Main.qml       # Main window
│   │   ├── Home/          # Main pages
│   │   └── Settings/      # Settings components
│   └── resources/         # Images, shaders, etc.
└── CMakeLists.txt
```

## Troubleshooting

### Qt6 Not Found
Ensure Qt6 is in your PATH or set CMAKE_PREFIX_PATH:
```bash
cmake -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64 ..
```

### Submodules Not Initialized
```bash
git submodule update --init --recursive
```

### Build Errors with HuskarUI
The HuskarUI include path should be automatically configured. If you encounter issues:
```bash
# Clean build directory
rm -rf build
mkdir build && cd build
cmake ..
```

### MapLibre Build Issues
MapLibre has many dependencies. Ensure all system libraries are installed:
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

## Features

- **Modular Plugin Architecture** - Extensible plugin system for adding new functionality
- **Settings Management** - Persistent settings with JSON storage
- **Map Integration** - MapLibre-based map rendering with online and offline support
- **Modern UI** - Built with HuskarUI component library
- **Cross-Platform** - Supports Windows, Linux, and macOS

## Development

### Adding a New Plugin

1. Implement the `IPlugin` interface
2. Register plugin metadata in `Q_PLUGIN_METADATA`
3. Place plugin library in the plugins directory
4. The plugin will be automatically discovered and loaded

### Settings API

```cpp
// C++
SettingsManager::instance()->setValue("category", "key", value);
QVariant value = SettingsManager::instance()->getValue("category", "key", defaultValue);
```

```qml
// QML
SettingsManager.setValue("category", "key", value)
let value = SettingsManager.getValue("category", "key", defaultValue)
```

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please ensure:
1. Code follows existing style conventions
2. All tests pass
3. New features include appropriate documentation
