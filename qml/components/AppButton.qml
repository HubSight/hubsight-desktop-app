import QtQuick
import QtQuick.Controls

Button {
    id: control

    property var theme
    property bool primary: true
    property bool compact: false

    implicitHeight: compact ? 40 : 44
    implicitWidth: Math.max(compact ? 40 : 120, contentItem.implicitWidth + 30)
    hoverEnabled: true

    contentItem: Text {
        text: control.text
        color: control.enabled ? (control.primary ? "#ffffff" : control.theme.textSecondary)
                               : control.theme.textMuted
        font.family: "Roboto"
        font.pixelSize: control.compact ? 13 : 14
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        radius: 6
        color: !control.enabled ? control.theme.surfaceMuted
                                : control.primary
                                  ? (control.hovered ? control.theme.accentHover : control.theme.accent)
                                  : (control.hovered ? control.theme.surfaceMuted : control.theme.surface)
        border.width: control.primary ? 0 : 1
        border.color: control.primary ? "transparent" : control.theme.border
    }
}
