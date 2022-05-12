import QtQuick
import modelReviewTool

Item {
    width: 1024
    height: 640

    QQuickBgfxItem {
        objectName: "bgfxItem1"
        id: bgfxItem1

        anchors.fill: parent
        viewId: 0
        backgroundColor: "#3369ff"
    }
}
