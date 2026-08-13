import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property var areaModel: null
    property int editAreaId: -1
    property var editInfo: editAreaId > 0 ? areaModel.getById(editAreaId) : ({})

    property int gridCols: 10
    property int gridRows: 8
    property real cellSize: Math.min((parent ? parent.width : Screen.width) - 4 * Theme.horizontalPageMargin, 280) / gridCols

    property int previewRow: parseInt(rowField.text) || 0
    property int previewCol: parseInt(colField.text) || 0
    property int previewRows: parseInt(rowSpanField.text) || 1
    property int previewCols: parseInt(colSpanField.text) || 1

    property string _boundsError: {
        if (previewRow + previewRows > gridRows)
            return qsTr("Height exceeds garden bounds");
        if (previewCol + previewCols > gridCols)
            return qsTr("Width exceeds garden bounds");
        return "";
    }

    property bool _dimsValid: {
        var w = parseFloat(widthField.text);
        var l = parseFloat(lengthField.text);
        return w > 0 && l > 0;
    }

    canAccept: nameField.text.trim().length > 0 && _boundsError === "" && _dimsValid

    onAccepted: {
        var name = nameField.text.trim();
        var gRow = previewRow;
        var gCol = previewCol;
        var gRows = previewRows;
        var gCols = previewCols;
        var w = parseFloat(widthField.text) || 0;
        var l = parseFloat(lengthField.text) || 0;

        if (editAreaId > 0) {
            areaModel.updateArea(editAreaId, name, gRow, gCol, gRows, gCols, w, l, notesField.text);
        } else {
            areaModel.addArea(name, gRow, gCol, gRows, gCols, w, l, notesField.text);
        }
    }

    Component.onCompleted: {
        gridRows = areaModel.gardenRows();
        gridCols = areaModel.gardenCols();

        if (editAreaId > 0) {
            nameField.text = editInfo.name || "";
            rowField.text = editInfo.gridRow !== undefined ? editInfo.gridRow.toString() : "0";
            colField.text = editInfo.gridCol !== undefined ? editInfo.gridCol.toString() : "0";
            rowSpanField.text = editInfo.gridRows !== undefined ? editInfo.gridRows.toString() : "1";
            colSpanField.text = editInfo.gridCols !== undefined ? editInfo.gridCols.toString() : "1";
            widthField.text = editInfo.widthM ? editInfo.widthM.toLocaleString(Qt.locale(), 'f', 1) : "";
            lengthField.text = editInfo.lengthM ? editInfo.lengthM.toLocaleString(Qt.locale(), 'f', 1) : "";
            notesField.text = editInfo.notes || "";
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationActive)
                previewCanvas.requestPaint();
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height + Theme.paddingLarge

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: editAreaId > 0 ? qsTr("Edit bed") : qsTr("New bed")
                acceptText: editAreaId > 0 ? qsTr("Save") : qsTr("Create")
            }

            SectionHeader {
                text: qsTr("Preview")
            }

            Rectangle {
                width: gridCols * cellSize + 4
                height: gridRows * cellSize + 4
                anchors.horizontalCenter: parent.horizontalCenter
                color: "transparent"
                border {
                    width: 2
                    color: Theme.secondaryColor
                }

                Canvas {
                    id: previewCanvas
                    anchors.fill: parent
                    anchors.margins: 2

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);

                        ctx.strokeStyle = Theme.rgba(Theme.secondaryColor, 0.15);
                        ctx.lineWidth = 1;

                        for (var r = 0; r < gridRows; r++) {
                            for (var c = 0; c < gridCols; c++) {
                                ctx.strokeRect(c * cellSize, r * cellSize, cellSize, cellSize);
                            }
                        }

                        if (areaModel) {
                            for (var i = 0; i < areaModel.rowCount(); i++) {
                                var a = areaModel.get(i);
                                if (a.areaId === editAreaId)
                                    continue;
                                var ax = a.gridCol * cellSize + 1;
                                var ay = a.gridRow * cellSize + 1;
                                var aw = a.gridCols * cellSize - 2;
                                var ah = a.gridRows * cellSize - 2;
                                ctx.fillStyle = Theme.rgba(Theme.secondaryColor, 0.3);
                                ctx.fillRect(ax, ay, aw, ah);
                                ctx.fillStyle = Theme.secondaryColor;
                                ctx.font = Math.min(cellSize * 0.3, 10) + "px sans-serif";
                                ctx.textAlign = "center";
                                ctx.textBaseline = "middle";
                                ctx.fillText(a.name, ax + aw / 2, ay + ah / 2);
                            }
                        }

                        var px = previewCol * cellSize + 1;
                        var py = previewRow * cellSize + 1;
                        var pw = previewCols * cellSize - 2;
                        var ph = previewRows * cellSize - 2;

                        ctx.fillStyle = "#8bc34a";
                        ctx.globalAlpha = 0.6;
                        ctx.fillRect(px, py, pw, ph);
                        ctx.globalAlpha = 1.0;

                        ctx.fillStyle = Theme.darkPrimaryColor;
                        ctx.font = Math.min(cellSize * 0.4, 14) + "px sans-serif";
                        ctx.textAlign = "center";
                        ctx.textBaseline = "middle";
                        if (nameField.text.length > 0) {
                            ctx.fillText(nameField.text, px + pw / 2, py + ph / 2);
                        }

                        ctx.strokeStyle = Theme.highlightColor;
                        ctx.lineWidth = 2;
                        ctx.strokeRect(px - 1, py - 1, pw + 2, ph + 2);
                    }
                }
            }

            TextField {
                id: nameField
                width: parent.width
                label: qsTr("Name")
                placeholderText: qsTr("e.g. Bed 1")
                onTextChanged: previewCanvas.requestPaint()
            }

            SectionHeader {
                text: qsTr("Grid position")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: qsTr("Garden: %1 x %2 cells").arg(gridRows).arg(gridCols)
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: _boundsError
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                visible: _boundsError !== ""
            }

            TextField {
                id: rowField
                width: parent.width
                label: qsTr("Row")
                placeholderText: "0"
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: previewCanvas.requestPaint()
            }

            TextField {
                id: colField
                width: parent.width
                label: qsTr("Column")
                placeholderText: "0"
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: previewCanvas.requestPaint()
            }

            TextField {
                id: rowSpanField
                width: parent.width
                label: qsTr("Height (cells)")
                placeholderText: "1"
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: previewCanvas.requestPaint()
            }

            TextField {
                id: colSpanField
                width: parent.width
                label: qsTr("Width (cells)")
                placeholderText: "1"
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: previewCanvas.requestPaint()
            }

            SectionHeader {
                text: qsTr("Actual size (meters)")
            }

            TextField {
                id: widthField
                width: parent.width
                label: qsTr("Width (m)")
                placeholderText: "1.0"
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            TextField {
                id: lengthField
                width: parent.width
                label: qsTr("Length (m)")
                placeholderText: "1.0"
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            TextArea {
                id: notesField
                width: parent.width
                label: qsTr("Notes")
                placeholderText: qsTr("Optional notes...")
            }
        }

        VerticalScrollDecorator {}
    }
}
