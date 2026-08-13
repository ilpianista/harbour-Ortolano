import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property int gridRows: 8
    property int gridCols: 10

    canAccept: {
        var r = parseInt(rowsField.text);
        var c = parseInt(colsField.text);
        return r > 0 && c > 0 && r <= 20 && c <= 20;
    }

    onAccepted: {
        var r = parseInt(rowsField.text);
        var c = parseInt(colsField.text);
        if (r > 0)
            areaModel.setGardenRows(r);
        if (c > 0)
            areaModel.setGardenCols(c);
        gridRows = r;
        gridCols = c;
    }

    Component.onCompleted: {
        rowsField.text = gridRows.toString();
        colsField.text = gridCols.toString();
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: qsTr("Garden size")
                acceptText: qsTr("Save")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: qsTr("Change the number of rows and columns in the garden grid.")
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.WordWrap
            }

            TextField {
                id: rowsField
                width: parent.width
                label: qsTr("Rows")
                placeholderText: "8"
                inputMethodHints: Qt.ImhDigitsOnly
            }

            TextField {
                id: colsField
                width: parent.width
                label: qsTr("Columns")
                placeholderText: "10"
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }

        VerticalScrollDecorator {}
    }
}
