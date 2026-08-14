TARGET = harbour-ortolano

QT += sql quick

CONFIG += sailfishapp
PKGCONFIG += sailfishapp

SOURCES += \
    src/main.cpp \
    src/databasemanager.cpp \
    src/speciesmodel.cpp \
    src/areamodel.cpp \
    src/plantingmodel.cpp \
    src/activitymodel.cpp

HEADERS += \
    src/databasemanager.h \
    src/speciesmodel.h \
    src/areamodel.h \
    src/plantingmodel.h \
    src/activitymodel.h

DISTFILES += \
    qml/harbour-ortolano.qml \
    qml/cover/CoverPage.qml \
    qml/pages/MainPage.qml \
    qml/pages/GardenCanvas.qml \
    qml/pages/AreaDetailPage.qml \
    qml/pages/AreaEditPage.qml \
    qml/pages/PlantingEditPage.qml \
    qml/pages/SpeciesListPage.qml \
    qml/pages/SpeciesDetailPage.qml \
    qml/pages/SpeciesPickerPage.qml \
    qml/pages/ActivityEditPage.qml \
    qml/pages/AreaPickerPage.qml \
    qml/pages/GardenSettingsDialog.qml \
    rpm/harbour-ortolano.spec \
    rpm/harbour-ortolano.changes \
    translations/*.ts \
    harbour-ortolano.desktop

CONFIG += sailfishapp_i18n

TRANSLATIONS += \
    translations/harbour-ortolano-it.ts

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172
