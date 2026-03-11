# DBCompt 组件说明

## 1. 模块职责

`src/dbcompt` 是 YEFS 的数据库基础组件，定位是“宿主和插件共享的数据库访问层”，而不是某个业务模块自己的 DAO 集合。它主要负责：

- 读取 `db_schema.json` 并解析数据库结构定义。
- 根据 schema 创建驱动并打开连接。
- 在启动时执行建表、增列、改名、删列与索引补齐。
- 对上提供统一的 `insert/select/update/remove/rawQuery/rawExecute` API。
- 屏蔽 SQLite / PostgreSQL 驱动差异。

当前主工程通过 `Application::initialize()` 在插件加载前打开 DBCompt，插件和 QML 只通过其公开单例访问数据库。

## 2. 对外接口与主框架交互

### 2.1 宿主初始化流程

1. `src/core/Application.cpp` 计算 schema 路径 `AppConfig/db_schema.json`。
2. 调用 `DBCompt::instance()->open(schemaPath)`。
3. `DBCompt` 把相对路径转换为应用目录下的绝对路径。
4. 内部委托 `dbcompt::DBCore::open()` 完成 schema 加载、驱动创建和迁移。
5. 打开成功后，插件才开始加载。

这意味着 DBCompt 是插件初始化的前置依赖，尤其是 `AirspaceManagerPlugin` 这样的业务插件默认认为数据库已经可用。

### 2.2 插件使用方式

当前空域插件通过 `AirspaceDatabase` 适配器调用 DBCompt：

- `addAirspace()` -> `DBCompt::insert()`
- `getAllAirspaces()` -> `DBCompt::select()`
- `updateAirspace()` -> `DBCompt::update()`
- `count()` -> `DBCompt::rawQuery()`

这个分层的意义是：业务模块保留自己的领域模型和字段映射，但连接管理、SQL 生成和迁移逻辑不在插件内重复实现。

## 3. 打开数据库主链路

`DBCore::open()` 的处理顺序如下：

1. 读取 schema 文件。
2. 解析数据库配置，如果是 SQLite 且路径为相对路径，则改写为相对于应用目录的绝对路径。
3. 根据 `database.type` 创建驱动实例。
4. 为当前 `DBCore` 生成唯一连接名并打开连接。
5. 执行迁移。
6. 标记模块已打开。

当前 `AppConfig/db_schema.json` 使用 SQLite，数据库文件落在 `AppData/yefs.db`。

## 4. Schema 驱动迁移机制

### 4.1 当前 schema 内容

当前默认 schema 只有 `airspaces` 一张业务表，包含：

- 主键 `id`
- 名称 `name`
- 几何类型 `shape_type`
- GeoJSON / style / properties 三个 JSON 文本字段
- 可见性 `visible`
- 创建和更新时间

另外定义了按名称和形状类型的索引。

### 4.2 迁移流程

`MigrationManager::migrate()` 的算法是“先全局判断是否需要备份，再逐表增量迁移”：

1. 先检查 schema 中是否存在列改名或删列。
2. 如果存在破坏性迁移且当前是 SQLite 文件库，则先尝试备份数据库文件。
3. 逐表处理：
   - 表不存在时直接创建。
   - 表存在时执行增量迁移。

单表增量迁移顺序固定为：

1. 按 `migration.rename` 改名旧列。
2. 按 `migration.drop` 删除列。
3. 为 schema 中新增但数据库里不存在的字段追加列。
4. 补齐索引。

这种顺序可以降低“列名冲突后无法新增”以及“先删后迁移丢上下文”的风险。

## 5. SQL 构建策略

### 5.1 QueryBuilder 角色

`query/QueryBuilder.*` 不直接执行 SQL，只负责把结构化输入转成：

- SQL 字符串
- 绑定参数列表

它处理的范围包括：

- DDL：建表、建索引、增列、改名列、删列
- DML：插入、查询、更新、删除

### 5.2 关键实现策略

- 表名和列名统一加双引号，降低关键字冲突风险。
- DML 使用 `?` 占位符与绑定参数列表，避免业务层手工拼接值。
- 字段类型先映射到通用 DDL 类型，再按后端差异做细节适配。
- `select/update/delete` 都支持条件字典，宿主和插件不需要自己写 where 拼接逻辑。

当前查询构建是“最小可用封装”，适合常见 CRUD 和轻量配置表；复杂统计或多表联查仍建议用 `rawQuery()` 明确书写。

## 6. 驱动抽象

`DBCore::createDriver()` 当前支持：

- `sqlite`
- `postgres` / `postgresql` / `postgis`

宿主当前实际使用 SQLite，但 schema 和驱动接口已经为后续切换到 PostgreSQL 预留了扩展点。新增后端时，优先扩展 `IDriver` 实现和驱动工厂，不要把数据库分支逻辑散落到业务模块。

## 7. 错误与状态流

- `DBCompt` 保存最近一次错误字符串，并在失败时发出 `errorOccurred`。
- `open()` 成功后发 `opened()`，`close()` 后发 `closed()`。
- 业务插件通常通过返回值和 `lastError()` 判断失败原因，而不是直接依赖内部驱动实现。

## 8. 设计约束与维护建议

- DBCompt 的职责是基础设施，不要把空域、航线等领域规则直接写进本模块。
- schema 的改动应优先通过 `db_schema.json` 驱动迁移，而不是在业务代码里硬编码 `ALTER TABLE`。
- 如果迁移涉及删列、改名或数据搬运，应同步评估备份策略和回滚成本。
- 若未来增加新业务表或新插件，建议继续通过领域适配器包装 DBCompt，而不是让 QML 直接操作裸表结构。