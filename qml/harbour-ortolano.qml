import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.ortolano 1.0
import "pages"

ApplicationWindow {
    id: app

    initialPage: Component {
        MainPage {}
    }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")
    allowedOrientations: defaultAllowedOrientations

    SpeciesModel {
        id: speciesModel
    }
    AreaModel {
        id: areaModel
    }
    PlantingModel {
        id: plantingModel
    }
    ActivityModel {
        id: activityModel
    }
    ActivityModel {
        id: mainActivityModel
    }
}
