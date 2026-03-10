import QtQuick
import QtQuick.Layouts
import HuskarUI.Basic
import YEFSApp

HusCard {
    id: root
    width: 260
    
    // 只在绘制中且至少有1个点时显示
    visible: DrawCtrl.drawingState === 1 && DrawCtrl.pointCount > 0

    property var shapeInfo: DrawCtrl.currentShapeInfo || {}
    property int shapeType: shapeInfo.type !== undefined ? shapeInfo.type : -1
    property int pointCount: shapeInfo.pointCount !== undefined ? shapeInfo.pointCount : 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        HusText {
            text: "图形信息"
            font.bold: true
            font.pixelSize: 14
            color: HusTheme.primaryColor
            Layout.bottomMargin: 4
        }

        HusText {
            text: getShapeName(shapeType) + " (" + pointCount + " 个点)"
            color: HusTheme.textColor
            font.pixelSize: 13
        }

        HusDivider { Layout.fillWidth: true }

        // Rectangle / Square
        GridLayout {
            visible: (shapeType === 0 || shapeType === 1) && shapeInfo.diagonalMeters !== undefined
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            HusText { text: "中心纬度:"; color: HusTheme.textColorLight }
            HusText { text: Number(shapeInfo.centerLat || 0).toFixed(6) + "°" }

            HusText { text: "中心经度:"; color: HusTheme.textColorLight }
            HusText { text: Number(shapeInfo.centerLng || 0).toFixed(6) + "°" }

            HusText { text: "对角距离:"; color: HusTheme.textColorLight }
            HusText { text: UnitManager.formatDistance(shapeInfo.diagonalMeters || 0) }
        }

        // Circle
        GridLayout {
            visible: shapeType === 2 && shapeInfo.radiusMeters !== undefined
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            HusText { text: "圆心纬度:"; color: HusTheme.textColorLight }
            HusText { text: Number(shapeInfo.centerLat || 0).toFixed(6) + "°" }

            HusText { text: "圆心经度:"; color: HusTheme.textColorLight }
            HusText { text: Number(shapeInfo.centerLng || 0).toFixed(6) + "°" }

            HusText { text: "半径距离:"; color: HusTheme.textColorLight }
            HusText { text: UnitManager.formatDistance(shapeInfo.radiusMeters || 0) }
        }

        // Polygon / Boundary
        GridLayout {
            visible: shapeType === 3 || shapeType === 4
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            HusText { text: "当前节点:"; color: HusTheme.textColorLight }
            HusText { text: pointCount + " 个点" }
        }

        // Ring
        GridLayout {
            visible: shapeType === 5 && shapeInfo.outerRadius !== undefined
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            HusText { text: "外半径:"; color: HusTheme.textColorLight }
            HusText { text: UnitManager.formatDistance(shapeInfo.outerRadius || 0) }
            
            HusText { text: "内半径:"; color: HusTheme.textColorLight; visible: shapeInfo.innerRadius !== undefined }
            HusText { text: UnitManager.formatDistance(shapeInfo.innerRadius || 0); visible: shapeInfo.innerRadius !== undefined }
        }
        
        // Sector / Arc
        GridLayout {
            visible: (shapeType === 6 || shapeType === 7 || shapeType === 8) && shapeInfo.radius !== undefined
            columns: 2
            rowSpacing: 6
            columnSpacing: 8

            HusText { text: "外半径:"; color: HusTheme.textColorLight; visible: shapeInfo.outerRadius !== undefined }
            HusText { text: UnitManager.formatDistance(shapeInfo.outerRadius || 0); visible: shapeInfo.outerRadius !== undefined }

            HusText { text: "内半径:"; color: HusTheme.textColorLight; visible: shapeInfo.innerRadius !== undefined }
            HusText { text: UnitManager.formatDistance(shapeInfo.innerRadius || 0); visible: shapeInfo.innerRadius !== undefined }
            
            HusText { text: "半径距离:"; color: HusTheme.textColorLight; visible: shapeInfo.radius !== undefined }
            HusText { text: UnitManager.formatDistance(shapeInfo.radius || 0); visible: shapeInfo.radius !== undefined }
            
            HusText { text: "起始角:"; color: HusTheme.textColorLight; visible: shapeInfo.startAngle !== undefined }
            HusText { text: Number(shapeInfo.startAngle || 0).toFixed(1) + "°"; visible: shapeInfo.startAngle !== undefined }
            
            HusText { text: "结束角:"; color: HusTheme.textColorLight; visible: shapeInfo.endAngle !== undefined }
            HusText { text: Number(shapeInfo.endAngle || 0).toFixed(1) + "°"; visible: shapeInfo.endAngle !== undefined }
        }
    }

    function getShapeName(type) {
        switch (type) {
            case 0: return "矩形"
            case 1: return "正方形"
            case 2: return "圆形"
            case 3: return "多边形"
            case 4: return "边界线"
            case 5: return "圆环"
            case 6: return "圆弧"
            case 7: return "扇形"
            case 8: return "扇环"
            default: return "未知图形"
        }
    }
}
