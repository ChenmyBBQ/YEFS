# AirspaceManagerPlugin 模块说明

## 1. 模块职责

`plugins/AirspaceManagerPlugin` 是当前唯一的业务插件模块，负责把“空域绘制、属性编辑、列表管理、数据库持久化、地图渲染”串成一条完整业务链。模块包含四个核心对象：

- `AirspaceManagerPlugin`：插件入口，负责初始化、宿主服务接入、QML 注册和地图回填。
- `DrawingController`：绘制状态机，处理取点、悬停预览、完成、取消、保存。
- `ShapeGenerator`：几何计算层，把各种空域参数离散为 GeoJSON。
- `AirspaceModel` / `AirspaceDatabase`：列表模型与数据库适配层。

## 2. 与主框架的交互方式

### 2.1 初始化流程

插件加载时执行以下步骤：

1. 宿主 `PluginManager` 通过 `QPluginLoader` 创建插件实例。
2. 调用 `initialize(context)`。
3. 插件创建 `AirspaceDatabase`，检查 DBCompt 是否已打开。
4. 创建 `AirspaceModel` 并从数据库加载已有空域。
5. 创建 `ShapeGenerator`。
6. 创建 `DrawingController`。
7. 连接宿主 `MessageBus` 与绘制控制器。
8. 把数据库、模型、控制器注册为 QML 单例。
9. 把历史空域重新加载到地图图层。

这条链路要求宿主已经先完成 DBCompt 初始化和 MapLibreEngine 注册，否则插件无法正常工作。

### 2.2 消息主题约定

插件主要依赖以下消息：

- 宿主 -> 插件：
  - `airspace-manager/new`
  - `airspace-manager/show`
  - `airspace-manager/hide`
  - `airspace-manager/draw`
  - `airspace-manager/finish`
  - `airspace-manager/cancel`
  - `map/clicked`
  - `map/hovered`
  - `map/hovered/clear`
- 插件 -> 宿主：
  - `airspace-manager/drawing-state`
  - `map/preview/annotation/set`
  - `map/preview/annotation/clear`

消息总线是这个插件的主通信手段，QML 页面和插件控制器并不直接共享复杂状态对象。

## 3. 用户交互主链路

### 3.1 从菜单进入空域功能

1. 用户在主窗口点击“新建空域”或“空域管理”。
2. `Main.qml` 发送对应消息。
3. `MapPageStateController` 接收后切换地图页状态：
   - 控制新建工具栏是否显示。
   - 控制空域列表面板是否显示。
   - 控制是否清空当前形状选择。

### 3.2 新建空域绘制流程

1. 用户在地图页左上工具栏选择形状。
2. `MapPageStateController` 记录 `selectedShapeType`。
3. 用户首次点击地图时，控制器不直接下点，而是先发送 `airspace-manager/draw` 启动插件绘制状态。
4. 插件进入 `Drawing` 状态后，再接收后续地图点击点位。
5. 每次点击调用 `DrawingController::addPoint()` 累积控制点。
6. 鼠标移动时，插件用 hover 点生成预览几何。
7. 达到固定点数的形状会自动完成；多边形/边界线等可变长形状需要用户手动点击“完成”。
8. 绘制完成后弹出属性编辑框。
9. 用户保存后，模型写入数据库，并把最终图层加入地图。

### 3.3 列表管理流程

空域列表页面支持：

- 搜索名称。
- 切换图层可见性。
- 打开编辑弹窗。
- 删除空域并同步移除地图图层。

列表模型 `AirspaceModel` 是 QML 的唯一数据源，界面不会直接操作数据库结果集。

## 4. 状态流与对象职责

### 4.1 DrawingController 状态机

`DrawingController` 只有三个状态：

- `Idle`：未绘制。
- `Drawing`：正在采点和预览。
- `Editing`：图形已完成，等待属性编辑和保存。

核心状态流：

1. `startDrawing(shapeType)`：重置旧状态并进入 `Drawing`。
2. `addPoint()`：采点、更新提示语、刷新预览。
3. `finishDrawing()`：校验最少点数，进入 `Editing`，发出 `drawingCompleted`。
4. `saveAirspace()`：写库成功后回到 `Idle`。
5. `cancel()`：清空临时状态并通知宿主移除预览。

### 4.2 AirspaceModel 与数据库同步策略

- 插件启动时全量 `loadFromDatabase()`。
- 新增空域时先写库，再把完整记录插到模型头部。
- 更新空域时按 id 替换模型项并触发 `dataChanged`。
- 删除空域时先删库，再从模型中移除。
- 可见性切换只更新目标项，不触发整表刷新。

这个策略的目标是让 QML 列表刷新粒度尽量小，避免每次编辑都重载全部空域。

## 5. 关键算法

### 5.1 距离与角度计算

`DrawingController` 使用两个轻量地理算法辅助交互：

- `haversineDistance()`：根据两点经纬度计算半径、对角线等米制距离。
- `bearingTo()`：根据中心点和控制点计算起止角，用于圆弧、扇形、扇环。

这些计算主要用于“从控制点推导几何参数”，而不是用于高精度测地分析。

### 5.2 形状离散化

`ShapeGenerator` 的核心原则是：把所有曲线形几何都离散为 MapLibre 可接受的 GeoJSON 折线或多边形。

各形状的生成方式如下：

- 矩形/正方形：根据中心、宽高和旋转角，计算四个角点后闭合。
- 圆：沿 0 到 360 度按固定分段采样生成多边形。
- 多边形：按用户采点顺序闭合外环。
- 边界线：直接输出 `LineString`。
- 圆环：外环顺时针采样，内环反向采样形成孔洞。
- 圆弧：按起止角采样输出 `LineString`。
- 扇形：中心点 + 外弧 + 回中心组成多边形。
- 扇环形：外弧与内弧反向拼接成闭合环带。

底层坐标换算通过 `destinationPoint()` 完成，即“已知中心点、方位角和距离，反算目标经纬度”。

### 5.3 高频预览优化

插件为了减少拖动绘制时的闪烁和 JSON 开销，采用了两级预览策略：

1. 用户悬停期间：
   - `DrawingController` 只发 `previewAnnotationSet(points)`。
   - 宿主地图使用 annotation 进行高频预览。
   - 不反复创建/删除普通 GeoJSON 图层。
2. 用户完成或取消时：
   - 清理 annotation 预览。
   - 再把最终 GeoJSON 通过 `previewUpdated` 或正式图层更新到地图。

另外，插件内部会缓存 `airspace-preview` 图层是否已创建；已存在时只调用 `updateLayerData()`，避免逐帧 remove/add 造成闪烁。

## 6. 持久化链路

1. `saveAirspace()` 把当前预览 GeoJSON 序列化为紧凑 JSON 字符串。
2. 调用 `AirspaceModel::addAirspace()`。
3. 模型转调 `AirspaceDatabase::addAirspace()`。
4. 数据库适配器生成 UUID 和时间戳，通过 DBCompt 插入 `airspaces` 表。
5. 成功后模型把新记录插入列表，并通知界面刷新。
6. QML 在 `onSaved` 回调中把新空域图层加入地图。

插件初始化时还会调用 `loadAirspacesToMap()`，把数据库已有记录重新回灌到地图，保证重启应用后能恢复显示状态。

## 7. 设计约束与已知边界

- 插件默认 DBCompt 已由宿主打开，不自行管理数据库生命周期。
- 预览图层 id 固定为 `airspace-preview`，正式图层 id 固定为 `airspace-<uuid>`，其他模块不要复用这些命名。
- 当前“定位到空域中心”按钮仍是 TODO，尚未实现 bbox 中心计算。
- 形状数据主要以 GeoJSON 文本保存，后续若增加更复杂属性，优先扩展 `properties_json`，避免额外拆表造成插件复杂度快速上升。
- 若后续新增更多绘图插件，建议继续沿用“宿主状态控制器 + 插件状态机 + MessageBus”这套协作方式。