import QtQuick
import QtQuick.Window

Window {
    id: root

    width: 420
    height: 280
    visible: true
    flags: Qt.SplashScreen | Qt.FramelessWindowHint
    color: "transparent"
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: appController.darkMode ? "#0b0f14" : "#f8fafc"
        border.width: 1
        border.color: appController.darkMode ? "#80334155" : "#b3cbd5e1"

        Column {
            anchors.centerIn: parent
            spacing: 7

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "qrc:/icons/hubsight-512.png"
                sourceSize: Qt.size(82, 82)
                width: 82
                height: 82
            }
            Text {
                width: root.width - 72
                text: "HubSight"
                color: appController.darkMode ? "#f8fafc" : "#0f172a"
                font.family: "Roboto"
                font.pixelSize: 22
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                width: root.width - 72
                text: "Secure desktop workspace"
                color: appController.darkMode ? "#94a3b8" : "#64748b"
                font.family: "Roboto"
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                width: root.width - 72
                topPadding: 8
                text: "Preparing secure workspace…"
                color: appController.darkMode ? "#fb923c" : "#c2410c"
                font.family: "Roboto"
                font.pixelSize: 12
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
