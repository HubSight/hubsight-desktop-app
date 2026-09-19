import QtQuick

Rectangle {
    property var theme

    radius: 8
    color: theme.surface
    border.width: 1
    border.color: theme.borderSubtle
}
