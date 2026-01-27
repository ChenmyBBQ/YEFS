# YEFS 地图格式解析系统

## 概述

YEFS 地图格式解析系统是一个灵活、可扩展的地图数据解析和显示框架，支持多种地图格式和在线地图服务。

## 架构设计

### 核心组件

1. **IMapSource** - 地图数据源接口
   - `IVectorMapSource` - 矢量数据源（GeoJSON、GPX、KML）
   - `IRasterMapSource` - 栅格瓦片数据源
   - `IOnlineMapSource` - 在线地图数据源

2. **IMapParser** - 地图格式解析器接口
   - 定义了解析器的标准接口
   - 支持格式检测和批量解析

3. **MapParserFactory** - 解析器工厂
   - 管理所有注册的解析器
   - 自动选择合适的解析器

4. **MapSourceManager** - 数据源管理器
   - 管理所有已加载的地图数据源
   - 提供 QML 访问接口

5. **OnlineMapProviderManager** - 在线地图提供商管理器
   - 管理预定义的在线地图提供商
   - 支持创建自定义在线地图源

## 支持的地图格式

### 矢量格式

#### GeoJSON
- 完整支持 GeoJSON 标准
- 支持的几何类型：Point, MultiPoint, LineString, MultiLineString, Polygon, MultiPolygon, GeometryCollection, Feature, FeatureCollection

#### GPX (GPS Exchange Format)
- 支持 GPX 1.0 和 1.1 标准
- 解析内容：轨迹（Tracks）、轨迹段（Track Segments）、航点（Waypoints）、海拔、时间、速度等扩展数据

#### KML (Keyhole Markup Language)
- 支持 KML 2.2/2.3 标准
- 解析内容：地标（Placemarks）、点、线、多边形、样式和描述信息

### 在线地图服务

系统内置了多个常用在线地图提供商：

#### 免费提供商
- **OpenStreetMap** - 开源社区驱动的世界地图
- **OpenFreeMap** - 免费的 OSM 托管服务
- **ESRI World Imagery** - Esri 全球卫星影像
- **CartoDB Positron** - 简洁清爽的地图样式

#### 需要 API Key 的提供商
- **MapTiler** - 高质量全球地图服务
- **Bing Maps** - 微软地图服务

## QML 集成

### MapSourceManager 单例

在 QML 中访问地图源管理器：

```qml
import YEFSApp

// 加载文件
MapSourceManager.loadFile("/path/to/map.gpx")

// 批量加载
MapSourceManager.loadFiles([
    "/path/to/file1.geojson",
    "/path/to/file2.kml"
])

// 获取所有数据源
Repeater {
    model: MapSourceManager.sources
    delegate: Text {
        text: modelData.name
    }
}

// 移除数据源
MapSourceManager.removeSource(sourceId)
```

### OnlineMapProviderManager 单例

在 QML 中添加在线地图：

```qml
import YEFSApp

// 添加 OpenStreetMap
let provider = OnlineMapProviderManager.createProvider(0) // OpenStreetMap
MapSourceManager.addSource(provider)

// 添加需要 API Key 的提供商
let maptiler = OnlineMapProviderManager.createProvider(2, "your-api-key-here")
```

## 功能特性

### 图层管理面板

系统提供了完整的图层管理 UI (`src/qml/Components/LayerPanel.qml`)：

- 显示所有已加载的图层
- 导入新的地图文件
- 切换图层可见性
- 定位到图层范围
- 删除图层

### 在线地图面板

提供在线地图选择和添加界面 (`src/qml/Components/OnlineMapPanel.qml`)：

- 浏览可用的在线地图提供商
- 查看提供商说明和使用条款
- 输入 API Key（如需要）
- 添加在线地图到图层

## 扩展开发

### 添加新的地图格式解析器

1. 创建解析器类继承 `IMapParser`
2. 实现必要的接口方法
3. 在 `Application::initializeMapParsers()` 中注册

### 添加新的在线地图提供商

在 `OnlineMapProviderManager::initializeProviders()` 中添加新的提供商配置。

## 未来计划

- [ ] 实现 MBTiles 离线瓦片地图支持
- [ ] 实现 WMS/WMTS 在线地图协议
- [ ] 添加 Shapefile 格式支持
- [ ] 实现数据源缓存机制
- [ ] 添加更多在线地图提供商
- [ ] 实现地形数据（DEM）支持
- [ ] 优化大文件解析性能
- [ ] 添加数据源导出功能

## 参考

- [GeoJSON 规范](https://geojson.org/)
- [GPX 格式文档](https://www.topografix.com/gpx.asp)
- [KML 参考](https://developers.google.com/kml/documentation/)
- [MapLibre GL JS](https://maplibre.org/)
- [GPXSee](https://github.com/tumic0/GPXSee) - 参考实现
