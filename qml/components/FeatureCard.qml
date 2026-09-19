import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AppCard {
    property string title
    property string description

    implicitHeight: 112

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 6

        Label {
            text: parent.parent.title
            color: parent.parent.theme.text
            font.family: "Roboto"
            font.pixelSize: 14
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            text: parent.parent.description
            color: parent.parent.theme.textMuted
            font.family: "Roboto"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
