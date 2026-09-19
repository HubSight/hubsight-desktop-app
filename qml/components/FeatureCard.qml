import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

AppCard {
    id: card

    property url iconSource
    property string title
    property string description

    implicitHeight: 156

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 6

        Rectangle {
            Layout.preferredWidth: 48
            Layout.preferredHeight: 48
            Layout.alignment: Qt.AlignHCenter
            radius: 8
            color: card.theme.accentSoft

            Image {
                anchors.centerIn: parent
                width: 28
                height: 28
                source: card.iconSource
                sourceSize: Qt.size(28, 28)
                fillMode: Image.PreserveAspectFit
            }
        }
        Label {
            text: card.title
            color: card.theme.text
            font.family: "Roboto"
            font.pixelSize: 14
            font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Label {
            text: card.description
            color: card.theme.textMuted
            font.family: "Roboto"
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
}
