import QtQuick
import QtQuick.Controls

Button {
    id: control

    property var theme
    property bool selected: false

    implicitHeight: 42
    hoverEnabled: true
    contentItem: Text {
        text: control.text
        color: control.selected ? control.theme.accentText : control.theme.textSecondary
        font.family: "Roboto"
        font.pixelSize: 14
        font.weight: control.selected ? Font.DemiBold : Font.Normal
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        leftPadding: 12
    }
    background: Rectangle {
        radius: 6
        color: control.selected ? control.theme.accentSoft
                                : (control.hovered ? control.theme.surfaceMuted : "transparent")
    }
}
