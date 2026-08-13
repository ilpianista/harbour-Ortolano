import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0

Dialog {
    id: dialog
    allowedOrientations: Orientation.All

    property int speciesId: -1
    property var speciesData: speciesId > 0 ? speciesModel.getById(speciesId) : ({})

    canAccept: nameField.text.trim().length > 0

    onAccepted: {
        var name = nameField.text.trim();
        var variety = varietyField.text.trim();
        var family = familyField.text.trim();
        var sowing = sowingField.text.trim();
        var days = parseInt(daysField.text) || 0;
        var rowSp = parseInt(rowSpacingField.text) || 0;
        var plantSp = parseInt(plantSpacingField.text) || 0;
        var notes = notesField.text;

        if (speciesId > 0) {
            speciesModel.updateSpecies(speciesId, name, variety, family, sowing, days, rowSp, plantSp, notes);
        } else {
            speciesModel.addSpecies(name, variety, family, sowing, days, rowSp, plantSp, notes);
        }
    }

    Component.onCompleted: {
        if (speciesId > 0) {
            nameField.text = speciesData.name || "";
            varietyField.text = speciesData.variety || "";
            familyField.text = speciesData.family || "";
            sowingField.text = speciesData.sowingSeason || "";
            daysField.text = speciesData.harvestDays ? speciesData.harvestDays.toString() : "";
            rowSpacingField.text = speciesData.rowSpacingCm ? speciesData.rowSpacingCm.toString() : "";
            plantSpacingField.text = speciesData.plantSpacingCm ? speciesData.plantSpacingCm.toString() : "";
            notesField.text = speciesData.notes || "";
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height + Theme.paddingLarge

        Column {
            id: content
            width: parent.width

            DialogHeader {
                title: speciesId > 0 ? qsTr("Edit species") : qsTr("New species")
                acceptText: speciesId > 0 ? qsTr("Save") : qsTr("Create")
            }

            TextField {
                id: nameField
                width: parent.width
                label: qsTr("Common name")
                placeholderText: qsTr("e.g. Tomato")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: varietyField.focus = true
            }

            TextField {
                id: varietyField
                width: parent.width
                label: qsTr("Variety")
                placeholderText: qsTr("e.g. San Marzano")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: familyField.focus = true
            }

            TextField {
                id: familyField
                width: parent.width
                label: qsTr("Botanical family")
                placeholderText: qsTr("e.g. Solanaceae")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: sowingField.focus = true
            }

            TextField {
                id: sowingField
                width: parent.width
                label: qsTr("Sowing season")
                placeholderText: qsTr("e.g. Mar-May")
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: daysField.focus = true
            }

            TextField {
                id: daysField
                width: parent.width
                label: qsTr("Days to harvest")
                placeholderText: qsTr("e.g. 80")
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: rowSpacingField.focus = true
            }

            SectionHeader {
                text: qsTr("Sowing spacing")
            }

            TextField {
                id: rowSpacingField
                width: parent.width
                label: qsTr("Row spacing (cm)")
                placeholderText: qsTr("e.g. 40")
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: plantSpacingField.focus = true
            }

            TextField {
                id: plantSpacingField
                width: parent.width
                label: qsTr("Plant spacing (cm)")
                placeholderText: qsTr("e.g. 30")
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: notesField.focus = true
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
