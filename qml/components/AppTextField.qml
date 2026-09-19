import QtQuick
import QtQuick.Controls

TextField {
    id: control

    property var theme

    implicitHeight: 44
    color: theme.text
    font.family: "Roboto"
    font.pixelSize: 14
    selectByMouse: true
    leftPadding: 12
    rightPadding: 12

    background: Rectangle {
        radius: 6
        color: theme.surface
        border.width: control.activeFocus ? 2 : 1
        border.color: control.activeFocus ? theme.accent : theme.border
    }
}
