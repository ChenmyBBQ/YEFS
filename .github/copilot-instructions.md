# YEFS - AI Agent Instructions

## Project Overview

YEFS (无人机地面站系统) is a Qt6/QML-based GIS platform for drone ground station systems. It integrates MapLibre Native Qt for map rendering and HuskarUI for the component library.

## Architecture

### Core Framework (`src/core/`)
- **Application** - App lifecycle, initializes services and QML engine
- **MessageBus** - Pub/sub system for cross-module communication (C++/QML). Use predefined topics in `YEFS::Topics::*`
- **PluginManager** - Plugin discovery/loading. Implements `IPlugin` interface for extensions
- **MapLibreEngine** - MapLibre wrapper exposing unified map API to QML
- **SettingsManager** - JSON-based settings persistence. Access via `SettingsManager.getValue(category, key, default)`

### QML Structure (`src/qml/`)
```
qml/
├── Main.qml          # App entry, HusWindow + HusMenu navigation
├── YefsGlobal.qml    # Menu structure definition with routes
├── Home/             # Main pages (MapPage, SettingsPage, etc.)
└── Settings/         # Settings category components
```

### Key Dependencies
- **HuskarUI** (`3rdparty/HuskarUI/`) - Ant Design-style QML components. Import as `HuskarUI.Basic`
- **MapLibre** (`3rdparty/maplibre-native-qt/`) - Map rendering. Import as `MapLibre 4.0`

## Build System

```bash
# Configure (from project root)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build

# Output location: out/<BuildType>/bin/
```

### CMake Options
| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_HUSKARUI_STATIC_LIBRARY` | OFF | Static link HuskarUI |
| `MLN_QT_WITH_LOCATION` | ON | Enable Qt Location integration |

## Code Patterns

### QML Singletons (C++ → QML)
All core services use `QML_SINGLETON` macro and are auto-registered:
```qml
import YEFSApp
// Access directly: MessageBus.send("topic", data)
// Access directly: SettingsManager.getValue("map", "styleUrl", "")
```

### Adding Menu Items
Edit [YefsGlobal.qml](src/qml/YefsGlobal.qml) `menus` array:
```qml
{ key: 'NewPage', label: qsTr('Label'), iconSource: HusIcon.IconName, source: './Home/NewPage.qml' }
```

### Settings Integration
```qml
// Read setting
let value = SettingsManager.getValue("category", "key", defaultValue);

// Write setting
SettingsManager.setValue("category", "key", newValue);

// React to changes
Connections {
    target: SettingsManager
    function onSettingsChanged(category, key) { /* handle */ }
}
```

### Plugin Development
Implement `IPlugin` interface in [IPlugin.h](src/core/IPlugin.h):
- Plugin types: `ToolPlugin`, `DataProvider`, `UIExtension`, `MapOverlay`
- Required: `id()`, `name()`, `version()`, `pluginType()`, `initialize()`, `shutdown()`
- Optional: `qmlEntry()`, `settingsPage()` for QML integration

## Conventions

- **Namespace**: All C++ core code in `YEFS::` namespace
- **Chinese comments**: Source uses Chinese (中文) for documentation
- **HuskarUI prefix**: UI components use `Hus*` prefix (HusButton, HusText, HusMenu, etc.)
- **Theme access**: Use `HusTheme.Primary.*` for colors, `HusTheme.isDark` for dark mode
- **Resource paths**: `qrc:/YEFSApp/resources/...` for images/shaders

## File Locations

| Purpose | Location |
|---------|----------|
| New QML pages | `src/qml/Home/` |
| Settings panels | `src/qml/Settings/` |
| Core C++ services | `src/core/` |
| Images/icons | `src/resources/images/` |
| Shaders | `src/resources/shaders/` |
