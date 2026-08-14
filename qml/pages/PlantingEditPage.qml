import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property int areaId: -1
    property int editId: -1
    property int _pickedSpeciesId: -1
    property int _rowSpacing: 0
    property int _plantSpacing: 0
    property date _plantingDate: new Date()

    function _loadSpacing(spId) {
        if (spId > 0) {
            var sp = speciesModel.getById(spId);
            _rowSpacing = sp.rowSpacingCm ? sp.rowSpacingCm : 0;
            _plantSpacing = sp.plantSpacingCm ? sp.plantSpacingCm : 0;
        }
    }

    property bool _needsDetail: sowTypeList.currentIndex === 1 || sowTypeList.currentIndex === 3

    function _formatDate(d) {
        return Qt.formatDate(d, "yyyy-MM-dd");
    }

    canAccept: _pickedSpeciesId > 0

    Component.onCompleted: {
        _plantingDate = new Date();
        if (editId > 0) {
            var p = plantingModel.getById(editId);
            if (p && p.speciesId) {
                _pickedSpeciesId = p.speciesId;
                speciesButton.value = p.speciesName || "";
                _loadSpacing(p.speciesId);
                _plantingDate = p.plantedDate ? new Date(p.plantedDate) : new Date();
                var st = p.sowingType || "";
                if (st === "file")
                    sowTypeList.currentIndex = 1;
                else if (st === "spaglio")
                    sowTypeList.currentIndex = 2;
                else if (st === "buchette")
                    sowTypeList.currentIndex = 3;
                else
                    sowTypeList.currentIndex = 0;
                sowDetailField.text = p.sowingDetail ? p.sowingDetail.toString() : "";
                notesField.text = p.notes || "";
            }
        }
    }

    onAccepted: {
        var spId = _pickedSpeciesId;
        var d = _formatDate(_plantingDate);
        var sType = sowTypeList.currentIndex === 0 ? "" : sowTypeList.value;
        var sDetail = _needsDetail ? (parseInt(sowDetailField.text) || 0) : 0;
        var notes = notesField.text;

        if (editId > 0) {
            plantingModel.updatePlanting(editId, spId, 0, d, sType, sDetail, notes);
        } else {
            plantingModel.addPlanting(areaId, spId, 0, d, sType, sDetail, notes);
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height + Theme.paddingLarge

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: editId > 0 ? qsTr("Edit planting") : qsTr("New planting")
                acceptText: editId > 0 ? qsTr("Save") : qsTr("Add")
            }

            ValueButton {
                id: speciesButton
                width: parent.width
                label: qsTr("Species")
                value: qsTr("Select…")
                onClicked: {
                    var picker = pageStack.push(Qt.resolvedUrl("SpeciesPickerPage.qml"));
                    picker.speciesPicked.connect(function (speciesId, name) {
                        _pickedSpeciesId = speciesId;
                        speciesButton.value = name;
                        _loadSpacing(speciesId);
                    });
                }
            }

            DetailItem {
                label: qsTr("Spacing")
                value: (_rowSpacing > 0 || _plantSpacing > 0) ? _rowSpacing + " \u00d7 " + _plantSpacing + " cm" : qsTr("Not specified")
                visible: _pickedSpeciesId > 0
            }

            ValueButton {
                id: dateButton
                width: parent.width
                label: qsTr("Planting date")
                value: Qt.formatDate(_plantingDate, Qt.DefaultLocaleShortDate)
                onClicked: {
                    var picker = pageStack.push("Sailfish.Silica.DatePickerDialog", {
                        date: _plantingDate
                    });
                    picker.accepted.connect(function () {
                        _plantingDate = picker.date;
                    });
                }
            }

            ComboBox {
                id: sowTypeList
                width: parent.width
                label: qsTr("Sowing type")
                currentIndex: 0
                property string value: currentIndex > 0 ? _sowTypes[currentIndex - 1] : ""
                property var _sowTypes: ["file", "spaglio", "buchette"]

                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Unspecified")
                    }
                    MenuItem {
                        text: qsTr("Rows")
                    }
                    MenuItem {
                        text: qsTr("Broadcast")
                    }
                    MenuItem {
                        text: qsTr("Holes")
                    }
                }
            }

            TextField {
                id: sowDetailField
                width: parent.width
                label: sowTypeList.currentIndex === 1 ? qsTr("Number of rows") : qsTr("Number of holes")
                placeholderText: "0"
                inputMethodHints: Qt.ImhDigitsOnly
                visible: _needsDetail
            }

            TextArea {
                id: notesField
                width: parent.width
                label: qsTr("Notes")
                placeholderText: qsTr("Optional notes…")
            }
        }

        VerticalScrollDecorator {}
    }
}
