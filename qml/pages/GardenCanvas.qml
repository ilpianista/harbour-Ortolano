import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property int gridCols: 10
    property int gridRows: 8
    property real cellSize: (page.width - 2 * Theme.horizontalPageMargin) / Math.max(gridCols, 1)
    property int _deleteAreaId: -1

    RemorsePopup {
        id: remorsePopup
        onTriggered: {
            if (_deleteAreaId > 0)
                areaModel.deleteArea(_deleteAreaId);
            _deleteAreaId = -1;
        }
    }

    function _loadDimensions() {
        gridRows = areaModel.gardenRows();
        gridCols = areaModel.gardenCols();
    }

    Timer {
        id: _loadTimer
        interval: 50
        repeat: false
        onTriggered: {
            _loadDimensions();
            areaModel.loadAll();
        }
    }

    Component.onCompleted: _loadTimer.start()

    onStatusChanged: {
        if (status === PageStatus.Active)
            _loadTimer.start();
    }

    Connections {
        target: areaModel
        onModelReset: canvas.requestPaint()
        onRowsInserted: canvas.requestPaint()
        onRowsRemoved: canvas.requestPaint()
        onDataChanged: canvas.requestPaint()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Garden size")
                onClicked: pageStack.push(Qt.resolvedUrl("GardenSettingsDialog.qml"), {
                    "gridRows": page.gridRows,
                    "gridCols": page.gridCols
                })
            }
            MenuItem {
                text: qsTr("New bed")
                onClicked: pageStack.push(Qt.resolvedUrl("AreaEditPage.qml"), {
                    "areaModel": areaModel
                })
            }
        }

        Column {
            id: column
            width: page.width

            PageHeader {
                title: qsTr("My garden")
            }

            Rectangle {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: gridRows * cellSize + 4
                color: "transparent"
                border {
                    width: 2
                    color: Theme.secondaryColor
                }

                Canvas {
                    id: canvas
                    anchors.fill: parent
                    anchors.margins: 2

                    onPaint: {
                        var ctx = getContext("2d");
                        ctx.clearRect(0, 0, width, height);

                        ctx.strokeStyle = Theme.rgba(Theme.secondaryColor, 0.2);
                        ctx.lineWidth = 1;

                        for (var r = 0; r < gridRows; r++) {
                            for (var c = 0; c < gridCols; c++) {
                                ctx.strokeRect(c * cellSize, r * cellSize, cellSize, cellSize);
                            }
                        }

                        for (var i = 0; i < areaModel.rowCount(); i++) {
                            var item = areaModel.get(i);
                            var x = item.gridCol * cellSize;
                            var y = item.gridRow * cellSize;
                            var w = item.gridCols * cellSize;
                            var h = item.gridRows * cellSize;

                            ctx.fillStyle = "#8bc34a";
                            ctx.fillRect(x + 1, y + 1, w - 2, h - 2);

                            ctx.fillStyle = Theme.darkPrimaryColor;
                            ctx.font = "bold " + Math.min(cellSize * 0.3, 13) + "px sans-serif";
                            ctx.textAlign = "center";
                            ctx.textBaseline = "middle";
                            ctx.fillText(item.name, x + w / 2, y + h / 2 - cellSize * 0.12);

                            if (item.widthM > 0 && item.lengthM > 0) {
                                ctx.font = Math.min(cellSize * 0.18, 10) + "px sans-serif";
                                ctx.fillText(item.widthM.toLocaleString(Qt.locale(), 'f', 1) + "\u00d7" + item.lengthM.toLocaleString(Qt.locale(), 'f', 1) + "m", x + w / 2, y + h / 2 + cellSize * 0.2);
                            }
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        var col = Math.floor(mouse.x / cellSize);
                        var row = Math.floor(mouse.y / cellSize);

                        for (var i = 0; i < areaModel.rowCount(); i++) {
                            var item = areaModel.get(i);
                            if (col >= item.gridCol && col < item.gridCol + item.gridCols && row >= item.gridRow && row < item.gridRow + item.gridRows) {
                                pageStack.push(Qt.resolvedUrl("AreaDetailPage.qml"), {
                                    "areaId": item.areaId
                                });
                                return;
                            }
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Beds")
            }

            Repeater {
                model: areaModel

                ListItem {
                    width: parent.width
                    contentHeight: Theme.itemSizeMedium

                    Row {
                        anchors {
                            left: parent.left
                            leftMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        spacing: Theme.paddingMedium
                        Rectangle {
                            width: 24
                            height: 24
                            color: "#8bc34a"
                            radius: 4
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Label {
                                text: model.name || ""
                                color: parent.parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                            }
                            Label {
                                text: model.plants || ""
                                font.pixelSize: Theme.fontSizeSmall
                                color: parent.parent.parent.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                                visible: text !== ""
                            }
                        }
                    }
                    onClicked: pageStack.push(Qt.resolvedUrl("AreaDetailPage.qml"), {
                        "areaId": model.areaId
                    })

                    menu: ContextMenu {
                        MenuItem {
                            text: qsTr("Delete")
                            onClicked: {
                                _deleteAreaId = model.areaId;
                                remorsePopup.execute(qsTr("Deleting bed"));
                            }
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
