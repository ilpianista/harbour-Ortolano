import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    property int areaId: -1
    property var areaInfo: ({})
    property bool _firstActivation: true
    property int _deletePlantingId: -1

    function _fmtDate(isoStr) {
        return Qt.formatDate(new Date(isoStr), Qt.DefaultLocaleShortDate);
    }

    ActivityModel {
        id: areaActivityModel
    }

    RemorsePopup {
        id: remorsePopup
        onTriggered: {
            if (_deletePlantingId > 0)
                plantingModel.deletePlanting(_deletePlantingId);
            _deletePlantingId = -1;
        }
    }

    function _loadArea() {
        areaInfo = areaModel.getById(areaId);
        plantingModel.loadByArea(areaId);
        areaActivityModel.loadByArea(areaId);
    }

    Component.onCompleted: _loadArea()

    onStatusChanged: {
        if (status === PageStatus.Active) {
            if (_firstActivation) {
                _firstActivation = false;
            } else {
                _loadArea();
            }
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Edit bed")
                onClicked: pageStack.push(Qt.resolvedUrl("AreaEditPage.qml"), {
                    "areaModel": areaModel,
                    "editAreaId": areaId
                })
            }
            MenuItem {
                text: qsTr("Add planting")
                onClicked: pageStack.push(Qt.resolvedUrl("PlantingEditPage.qml"), {
                    "areaId": areaId
                })
            }
        }

        Column {
            id: column
            width: page.width

            PageHeader {
                title: qsTr("Bed %1").arg(areaInfo.name)
            }

            DetailItem {
                label: qsTr("Actual dimensions")
                value: {
                    var w = areaInfo.widthM || 0;
                    var l = areaInfo.lengthM || 0;
                    if (w || l)
                        return w.toLocaleString(Qt.locale(), 'f', 1) + " × " + l.toLocaleString(Qt.locale(), 'f', 1) + " m";
                    return "";
                }
            }

            DetailItem {
                label: qsTr("Notes")
                value: areaInfo.notes || ""
                visible: !!areaInfo.notes
            }

            SectionHeader {
                text: qsTr("Plantings")
            }

            InfoLabel {
                text: qsTr("No plants")
                visible: plantingModel.count === 0
            }

            Column {
                width: parent.width

                Repeater {
                    model: plantingModel

                    ListItem {
                        width: parent.width
                        contentHeight: Theme.itemSizeLarge

                        onClicked: pageStack.push(Qt.resolvedUrl("PlantingEditPage.qml"), {
                            areaId: areaId,
                            editId: model.plantingId
                        })

                        Column {
                            anchors {
                                left: parent.left
                                leftMargin: Theme.horizontalPageMargin
                                verticalCenter: parent.verticalCenter
                            }
                            width: parent.width - 2 * Theme.horizontalPageMargin

                            Label {
                                text: model.speciesName || ""
                                color: parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                                truncationMode: TruncationMode.Fade
                            }
                            Label {
                                text: {
                                    var parts = [];
                                    if (model.plantedDate)
                                        parts.push(qsTr("Planted: %1").arg(_fmtDate(model.plantedDate)));
                                    if (model.sowingType) {
                                        var t = model.sowingType === "file" ? qsTr("in rows") : model.sowingType === "spaglio" ? qsTr("broadcast") : model.sowingType === "buchette" ? qsTr("in holes") : model.sowingType;
                                        if (model.sowingDetail > 0)
                                            t += " (" + model.sowingDetail + ")";
                                        parts.push(t);
                                    }
                                    return parts.join(" — ");
                                }
                                font.pixelSize: Theme.fontSizeSmall
                                color: parent.parent.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                                truncationMode: TruncationMode.Fade
                            }
                        }

                        menu: ContextMenu {
                            MenuItem {
                                text: qsTr("Delete")
                                onClicked: {
                                    _deletePlantingId = model.plantingId;
                                    remorsePopup.execute(qsTr("Deleting planting"));
                                }
                            }
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Recent activities")
            }

            InfoLabel {
                text: qsTr("No activities")
                visible: areaActivityModel.count === 0
            }

            Column {
                width: parent.width
                visible: areaActivityModel.count > 0

                Repeater {
                    model: areaActivityModel

                    ListItem {
                        width: parent.width
                        contentHeight: Theme.itemSizeMedium

                        Column {
                            anchors {
                                left: parent.left
                                leftMargin: Theme.horizontalPageMargin
                                verticalCenter: parent.verticalCenter
                            }
                            width: parent.width - 2 * Theme.horizontalPageMargin

                            Label {
                                text: model.activityTypeName || ""
                                color: parent.parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                                truncationMode: TruncationMode.Fade
                            }
                            Label {
                                text: {
                                    var parts = [_fmtDate(model.date)];
                                    if (model.speciesName)
                                        parts.push(model.speciesName);
                                    if (model.quantity > 0)
                                        parts.push(model.quantity.toLocaleString(Qt.locale(), 'f', 1) + " kg");
                                    return parts.join(" \u00b7 ");
                                }
                                font.pixelSize: Theme.fontSizeSmall
                                color: parent.parent.highlighted ? Theme.secondaryHighlightColor : Theme.secondaryColor
                                truncationMode: TruncationMode.Fade
                            }
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
