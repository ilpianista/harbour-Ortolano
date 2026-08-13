#include "activitymodel.h"
#include "areamodel.h"
#include "databasemanager.h"
#include "plantingmodel.h"
#include "speciesmodel.h"
#include <sailfishapp.h>
#include <QDir>
#include <QGuiApplication>
#include <QSqlDatabase>
#include <QStandardPaths>

#include <QtQuick>

static void registerQmlTypes()
{
    qmlRegisterType<SpeciesModel>("harbour.ortolano", 1, 0, "SpeciesModel");
    qmlRegisterType<AreaModel>("harbour.ortolano", 1, 0, "AreaModel");
    qmlRegisterType<PlantingModel>("harbour.ortolano", 1, 0, "PlantingModel");
    qmlRegisterType<ActivityModel>("harbour.ortolano", 1, 0, "ActivityModel");
}

int main(int argc, char *argv[])
{
    QGuiApplication *app = SailfishApp::application(argc, argv);

    app->setOrganizationName(QStringLiteral("dev.scarpino"));
    app->setApplicationName(QStringLiteral("harbour-ortolano"));

    registerQmlTypes();

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dataPath);

    DatabaseManager dbManager;
    dbManager.initDatabase(dataPath + QStringLiteral("/ortolano.db"));

    return SailfishApp::main(argc, argv);
}
