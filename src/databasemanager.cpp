#include "databasemanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{}

bool DatabaseManager::initDatabase(const QString &path)
{
    bool exists = QFileInfo::exists(path);

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        setLastError(m_db.lastError().text());
        qWarning() << "Failed to open database:" << m_lastError;
        return false;
    }

    QSqlQuery pragma(m_db);
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    pragma.exec(QStringLiteral("PRAGMA foreign_keys=ON"));

    createTables();

    if (!exists) {
        seedData();
    }

    return true;
}

void DatabaseManager::createTables()
{
    QSqlQuery q(m_db);

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS species ("
                          "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "  name TEXT NOT NULL,"
                          "  variety TEXT DEFAULT '',"
                          "  family TEXT DEFAULT '',"
                          "  sowing_season TEXT DEFAULT '',"
                          "  harvest_days INTEGER DEFAULT 0,"
                          "  row_spacing_cm INTEGER DEFAULT 0,"
                          "  plant_spacing_cm INTEGER DEFAULT 0,"
                          "  notes TEXT DEFAULT ''"
                          ")"));

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS areas ("
                          "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "  name TEXT NOT NULL,"
                          "  grid_row INTEGER NOT NULL,"
                          "  grid_col INTEGER NOT NULL,"
                          "  grid_rows INTEGER DEFAULT 1,"
                          "  grid_cols INTEGER DEFAULT 1,"
                          "  width_m REAL DEFAULT 1.0,"
                          "  length_m REAL DEFAULT 1.0,"
                          "  notes TEXT DEFAULT ''"
                          ")"));

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS plantings ("
                          "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "  area_id INTEGER NOT NULL,"
                          "  species_id INTEGER NOT NULL,"
                          "  plants_count INTEGER DEFAULT 0,"
                          "  planted_date TEXT,"
                          "  sowing_type TEXT DEFAULT '',"
                          "  sowing_detail INTEGER DEFAULT 0,"
                          "  notes TEXT DEFAULT '',"
                          "  FOREIGN KEY (area_id) REFERENCES areas(id) ON DELETE CASCADE,"
                          "  FOREIGN KEY (species_id) REFERENCES species(id) ON DELETE CASCADE"
                          ")"));

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS activities ("
                          "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                          "  activity_type_id INTEGER,"
                          "  date TEXT NOT NULL,"
                          "  notes TEXT DEFAULT '',"
                          "  species_id INTEGER,"
                          "  quantity REAL DEFAULT 0,"
                          "  FOREIGN KEY (species_id) REFERENCES species(id) ON DELETE SET NULL"
                          ")"));

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS activity_areas ("
                          "  activity_id INTEGER NOT NULL,"
                          "  area_id INTEGER NOT NULL,"
                          "  PRIMARY KEY (activity_id, area_id),"
                          "  FOREIGN KEY (activity_id) REFERENCES activities(id) ON DELETE CASCADE,"
                          "  FOREIGN KEY (area_id) REFERENCES areas(id) ON DELETE CASCADE"
                          ")"));

    q.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS settings ("
                          "  key TEXT PRIMARY KEY,"
                          "  value TEXT NOT NULL"
                          ")"));
}

void DatabaseManager::seedData()
{
    QSqlQuery q(m_db);

    struct SpeciesData
    {
        QString name;
        QString family;
        QString sowing;
        int days;
    };
    QVector<SpeciesData> seedSpecies = {
        {QStringLiteral("Tomato"), QStringLiteral("Solanaceae"), QStringLiteral("Mar-May"), 80},
        {QStringLiteral("Zucchini"), QStringLiteral("Cucurbitaceae"), QStringLiteral("Apr-Jun"), 60},
        {QStringLiteral("Eggplant"), QStringLiteral("Solanaceae"), QStringLiteral("Mar-May"), 90},
        {QStringLiteral("Bell pepper"), QStringLiteral("Solanaceae"), QStringLiteral("Mar-May"), 75},
        {QStringLiteral("Lettuce"), QStringLiteral("Asteraceae"), QStringLiteral("Mar-Sep"), 45},
        {QStringLiteral("Carrot"), QStringLiteral("Apiaceae"), QStringLiteral("Feb-Aug"), 80},
        {QStringLiteral("Onion"), QStringLiteral("Amaryllidaceae"), QStringLiteral("Feb-Apr"), 120},
        {QStringLiteral("Garlic"), QStringLiteral("Amaryllidaceae"), QStringLiteral("Oct-Dec"), 180},
        {QStringLiteral("Basil"), QStringLiteral("Lamiaceae"), QStringLiteral("Apr-Jun"), 30},
        {QStringLiteral("Parsley"), QStringLiteral("Apiaceae"), QStringLiteral("Mar-Aug"), 60},
        {QStringLiteral("Bean"), QStringLiteral("Fabaceae"), QStringLiteral("Apr-Jun"), 60},
        {QStringLiteral("Pea"), QStringLiteral("Fabaceae"), QStringLiteral("Feb-Apr"), 90},
        {QStringLiteral("Cucumber"), QStringLiteral("Cucurbitaceae"), QStringLiteral("Apr-Jun"), 60},
        {QStringLiteral("Pumpkin"), QStringLiteral("Cucurbitaceae"), QStringLiteral("Apr-May"), 120},
        {QStringLiteral("Cabbage"), QStringLiteral("Brassicaceae"), QStringLiteral("Jun-Aug"), 90},
        {QStringLiteral("Spinach"), QStringLiteral("Amaranthaceae"), QStringLiteral("Mar-May"), 45},
        {QStringLiteral("Chard"), QStringLiteral("Amaranthaceae"), QStringLiteral("Mar-Aug"), 60},
        {QStringLiteral("Radish"), QStringLiteral("Brassicaceae"), QStringLiteral("Mar-Sep"), 25},
        {QStringLiteral("Fennel"), QStringLiteral("Apiaceae"), QStringLiteral("Jul-Aug"), 80},
        {QStringLiteral("Celery"), QStringLiteral("Apiaceae"), QStringLiteral("Mar-Jun"), 100},
    };

    m_db.transaction();

    for (const auto &s : seedSpecies) {
        q.prepare(QStringLiteral("INSERT INTO species (name, family, sowing_season, harvest_days) "
                                 "VALUES (?, ?, ?, ?)"));
        q.addBindValue(s.name);
        q.addBindValue(s.family);
        q.addBindValue(s.sowing);
        q.addBindValue(s.days);
        if (!q.exec()) {
            qWarning() << "Seed species error:" << q.lastError().text();
        }
    }

    QSqlQuery settings(m_db);
    settings.prepare(QStringLiteral("INSERT OR IGNORE INTO settings (key, value) VALUES (?, ?)"));

    settings.addBindValue(QStringLiteral("garden_rows"));
    settings.addBindValue(QStringLiteral("8"));
    settings.exec();

    settings.addBindValue(QStringLiteral("garden_cols"));
    settings.addBindValue(QStringLiteral("10"));
    settings.exec();

    m_db.commit();
}

void DatabaseManager::setLastError(const QString &error)
{
    if (m_lastError != error) {
        m_lastError = error;
        emit lastErrorChanged();
    }
}
