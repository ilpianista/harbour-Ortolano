import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property int editId: -1
    property var _pickedAreas: []
    property int _pickedSpeciesId: -1

    property date _activityDate: new Date()

    property bool _isHarvest: {
        if (typeList.currentIndex < 0)
            return false;
        var hid = activityModel.harvestTypeId();
        return activityModel.activityTypeId(typeList.currentIndex) === hid;
    }

    function _formatDate(d) {
        return Qt.formatDate(d, "yyyy-MM-dd");
    }

    canAccept: typeList.currentIndex >= 0

    Component.onCompleted: {
        _activityDate = new Date();
        if (editId > 0) {
            for (var i = 0; i < activityModel.rowCount(); i++) {
                var a = activityModel.get(i);
                if (a && a.activityId === editId) {
                    for (var j = 0; j < activityModel.typeCount; j++) {
                        if (activityModel.activityTypeId(j) === a.activityTypeId) {
                            typeList.currentIndex = j;
                            break;
                        }
                    }
                    _activityDate = a.date ? new Date(a.date) : new Date();
                    notesField.text = a.notes || "";
                    _pickedAreas = a.areaIds || [];
                    _pickedSpeciesId = a.speciesId || -1;
                    if (a.speciesId > 0) {
                        speciesModel.loadAll();
                        speciesButton.value = a.speciesName || "";
                    }
                    if (a.quantity > 0)
                        quantityField.text = a.quantity.toString();
                    areasButton.value = a.areaNames || qsTr("None");
                    break;
                }
            }
        }
    }

    onAccepted: {
        var typeId = activityModel.activityTypeId(typeList.currentIndex);
        var d = _formatDate(_activityDate);
        var notes = notesField.text;
        var spId = _isHarvest ? _pickedSpeciesId : 0;
        var qty = _isHarvest ? (parseFloat(quantityField.text) || 0) : 0;

        var areaList = [];
        for (var i = 0; i < _pickedAreas.length; i++)
            areaList.push(_pickedAreas[i]);

        if (editId > 0) {
            activityModel.updateActivity(editId, typeId, areaList, d, notes, spId, qty);
        } else {
            activityModel.addActivity(typeId, areaList, d, notes, spId, qty);
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height + Theme.paddingLarge

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: editId > 0 ? qsTr("Edit activity") : qsTr("New activity")
                acceptText: editId > 0 ? qsTr("Save") : qsTr("Add")
            }

            ComboBox {
                id: typeList
                width: parent.width
                label: qsTr("Activity type")
                menu: ContextMenu {
                    Repeater {
                        model: activityModel.typeCount
                        MenuItem {
                            text: activityModel.activityTypeName(index)
                        }
                    }
                }
            }

            SectionHeader {
                text: qsTr("Details")
            }

            ValueButton {
                id: dateButton
                width: parent.width
                label: qsTr("Date")
                value: Qt.formatDate(_activityDate, Qt.DefaultLocaleShortDate)
                onClicked: {
                    var picker = pageStack.push("Sailfish.Silica.DatePickerDialog", {
                        date: _activityDate
                    });
                    picker.accepted.connect(function () {
                        _activityDate = picker.date;
                    });
                }
            }

            ValueButton {
                id: areasButton
                width: parent.width
                label: qsTr("Beds")
                value: _pickedAreas.length > 0 ? qsTr("%1 selected").arg(_pickedAreas.length) : qsTr("None")
                onClicked: {
                    var picker = pageStack.push(Qt.resolvedUrl("AreaPickerPage.qml"), {
                        "selectedIds": _pickedAreas
                    });
                    picker.areasPicked.connect(function (areaIds) {
                        _pickedAreas = areaIds;
                        areasButton.value = areaIds.length > 0 ? qsTr("%1 selected").arg(areaIds.length) : qsTr("None");
                    });
                }
            }

            Column {
                width: parent.width
                visible: _isHarvest

                ValueButton {
                    id: speciesButton
                    width: parent.width
                    label: qsTr("Species")
                    value: _pickedSpeciesId > 0 ? (speciesButton.value || qsTr("Select...")) : qsTr("Select...")
                    onClicked: {
                        var picker = pageStack.push(Qt.resolvedUrl("SpeciesPickerPage.qml"));
                        picker.speciesPicked.connect(function (speciesId, name) {
                            _pickedSpeciesId = speciesId;
                            speciesButton.value = name;
                        });
                    }
                }

                TextField {
                    id: quantityField
                    width: parent.width
                    label: qsTr("Quantity (kg)")
                    placeholderText: "0"
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
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
