import QtQuick
import HuskarUI.Basic

QtObject {
    id: root

    // 菜单选项
    property var options: []

    // 菜单结构
    property var menus: [
        {
            key: 'MapPage',
            label: qsTr('地图'),
            iconSource: HusIcon.EnvironmentOutlined,
            source: './Home/MapPage.qml'
        },
        {
            type: 'divider'
        },
        {
            key: 'FlightPlan',
            label: qsTr('航线规划'),
            iconSource: HusIcon.SendOutlined,
            menuChildren: [
                {
                    key: 'RoutePlanning',
                    label: qsTr('航线设计'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('设计和编辑无人机飞行航线')
                },
                {
                    key: 'WaypointManager',
                    label: qsTr('航点管理'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('管理航点和任务点')
                },
                {
                    key: 'MissionImport',
                    label: qsTr('任务导入'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('导入已有的飞行任务')
                }
            ]
        },
        {
            key: 'AirspaceManagement',
            label: qsTr('空域管理'),
            iconSource: HusIcon.CloudOutlined,
            menuChildren: [
                {
                    key: 'AirspaceQuery',
                    label: qsTr('空域查询'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('查询飞行空域信息')
                },
                {
                    key: 'RestrictedZone',
                    label: qsTr('禁飞区'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('查看和管理禁飞区域')
                },
                {
                    key: 'WeatherInfo',
                    label: qsTr('气象信息'),
                    source: './Home/MapPage.qml',
                    state: 'New',
                    desc: qsTr('查看实时气象数据')
                }
            ]
        },
        {
            key: 'DataManagement',
            label: qsTr('数据管理'),
            iconSource: HusIcon.DatabaseOutlined,
            menuChildren: [
                {
                    key: 'FlightRecords',
                    label: qsTr('飞行记录'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('查看历史飞行记录')
                },
                {
                    key: 'MapData',
                    label: qsTr('地图数据'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('管理离线地图数据')
                },
                {
                    key: 'ExportData',
                    label: qsTr('数据导出'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('导出飞行数据和报告')
                }
            ]
        },
        {
            key: 'MapTools',
            label: qsTr('地图工具'),
            iconSource: HusIcon.ToolOutlined,
            menuChildren: [
                {
                    key: 'Measure',
                    label: qsTr('测量工具'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('测量距离和面积')
                },
                {
                    key: 'Annotation',
                    label: qsTr('标注工具'),
                    source: './Home/MapPage.qml',
                    desc: qsTr('在地图上添加标注')
                },
                {
                    key: 'CoordinateConvert',
                    label: qsTr('坐标转换'),
                    source: './Home/MapPage.qml',
                    state: 'New',
                    desc: qsTr('坐标系统转换工具')
                }
            ]
        }
    ]

    Component.onCompleted: {
        // 构建搜索选项
        buildOptions();
    }

    function buildOptions() {
        let opts = [];
        for (let i = 0; i < menus.length; i++) {
            const menu = menus[i];
            if (menu.type === 'divider') continue;
            
            if (menu.menuChildren) {
                for (let j = 0; j < menu.menuChildren.length; j++) {
                    const child = menu.menuChildren[j];
                    opts.push({
                        key: child.key,
                        label: child.label,
                        value: child.key,
                        state: child.state || ''
                    });
                }
            } else {
                opts.push({
                    key: menu.key,
                    label: menu.label,
                    value: menu.key,
                    state: menu.state || ''
                });
            }
        }
        options = opts;
    }
}
