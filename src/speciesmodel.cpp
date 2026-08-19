#include "speciesmodel.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

SpeciesModel::SpeciesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_db = QSqlDatabase::database();
}

int SpeciesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

QVariant SpeciesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const auto &e = m_entries.at(index.row());
    switch (role) {
    case IdRole:
        return e.id;
    case NameRole:
        return e.name;
    case VarietyRole:
        return e.variety;
    case FamilyRole:
        return e.family;
    case SowingSeasonRole:
        return e.sowingSeason;
    case HarvestDaysRole:
        return e.harvestDays;
    case RowSpacingRole:
        return e.rowSpacingCm;
    case PlantSpacingRole:
        return e.plantSpacingCm;
    case NotesRole:
        return e.notes;
    default:
        return {};
    }
}

QHash<int, QByteArray> SpeciesModel::roleNames() const
{
    return {{IdRole, "speciesId"},
            {NameRole, "name"},
            {VarietyRole, "variety"},
            {FamilyRole, "family"},
            {SowingSeasonRole, "sowingSeason"},
            {HarvestDaysRole, "harvestDays"},
            {RowSpacingRole, "rowSpacingCm"},
            {PlantSpacingRole, "plantSpacingCm"},
            {NotesRole, "notes"}};
}

void SpeciesModel::loadAll()
{
    beginResetModel();
    m_entries.clear();

    QSqlQuery q(m_db);
    q.exec(QStringLiteral("SELECT id, name, variety, family, sowing_season, harvest_days, "
                          "row_spacing_cm, plant_spacing_cm, notes "
                          "FROM species ORDER BY name COLLATE NOCASE"));
    while (q.next()) {
        SpeciesEntry e;
        e.id = q.value(0).toInt();
        e.name = q.value(1).toString();
        e.variety = q.value(2).toString();
        e.family = q.value(3).toString();
        e.sowingSeason = q.value(4).toString();
        e.harvestDays = q.value(5).toInt();
        e.rowSpacingCm = q.value(6).toInt();
        e.plantSpacingCm = q.value(7).toInt();
        e.notes = q.value(8).toString();
        m_entries.append(e);
    }

    endResetModel();
    emit countChanged();
}

void SpeciesModel::search(const QString &text)
{
    beginResetModel();
    m_entries.clear();

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, name, variety, family, sowing_season, harvest_days, "
                             "row_spacing_cm, plant_spacing_cm, notes "
                             "FROM species WHERE name LIKE ? OR variety LIKE ? OR family LIKE ? "
                             "ORDER BY name COLLATE NOCASE"));
    QString pattern = QStringLiteral("%%1%").arg(text);
    q.addBindValue(pattern);
    q.addBindValue(pattern);
    q.addBindValue(pattern);
    q.exec();

    while (q.next()) {
        SpeciesEntry e;
        e.id = q.value(0).toInt();
        e.name = q.value(1).toString();
        e.variety = q.value(2).toString();
        e.family = q.value(3).toString();
        e.sowingSeason = q.value(4).toString();
        e.harvestDays = q.value(5).toInt();
        e.rowSpacingCm = q.value(6).toInt();
        e.plantSpacingCm = q.value(7).toInt();
        e.notes = q.value(8).toString();
        m_entries.append(e);
    }

    endResetModel();
    emit countChanged();
}

bool SpeciesModel::addSpecies(const QString &name,
                              const QString &variety,
                              const QString &family,
                              const QString &sowingSeason,
                              int harvestDays,
                              int rowSpacingCm,
                              int plantSpacingCm,
                              const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(
        QStringLiteral("INSERT INTO species (name, variety, family, sowing_season, harvest_days, "
                       "row_spacing_cm, plant_spacing_cm, notes) "
                       "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(name);
    q.addBindValue(variety);
    q.addBindValue(family);
    q.addBindValue(sowingSeason);
    q.addBindValue(harvestDays);
    q.addBindValue(rowSpacingCm);
    q.addBindValue(plantSpacingCm);
    q.addBindValue(notes);
    if (!q.exec()) {
        qWarning() << "addSpecies error:" << q.lastError().text();
        return false;
    }

    loadAll();
    return true;
}

bool SpeciesModel::updateSpecies(int id,
                                 const QString &name,
                                 const QString &variety,
                                 const QString &family,
                                 const QString &sowingSeason,
                                 int harvestDays,
                                 int rowSpacingCm,
                                 int plantSpacingCm,
                                 const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(
        QStringLiteral("UPDATE species SET name=?, variety=?, family=?, sowing_season=?, "
                       "harvest_days=?, row_spacing_cm=?, plant_spacing_cm=?, notes=? WHERE id=?"));
    q.addBindValue(name);
    q.addBindValue(variety);
    q.addBindValue(family);
    q.addBindValue(sowingSeason);
    q.addBindValue(harvestDays);
    q.addBindValue(rowSpacingCm);
    q.addBindValue(plantSpacingCm);
    q.addBindValue(notes);
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "updateSpecies error:" << q.lastError().text();
        return false;
    }

    loadAll();
    return true;
}

bool SpeciesModel::deleteSpecies(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM plantings WHERE species_id=?"));
    q.addBindValue(id);
    q.exec();

    q.prepare(QStringLiteral("DELETE FROM species WHERE id=?"));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "deleteSpecies error:" << q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() == 0)
        return false;

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == id) {
            beginRemoveRows(QModelIndex(), i, i);
            m_entries.removeAt(i);
            endRemoveRows();
            emit countChanged();
            return true;
        }
    }
    return false;
}

QVariantMap SpeciesModel::getById(int id) const
{
    QVariantMap map;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, name, variety, family, sowing_season, harvest_days, "
                             "row_spacing_cm, plant_spacing_cm, notes FROM species WHERE id=?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        map[QStringLiteral("id")] = q.value(0);
        map[QStringLiteral("name")] = q.value(1);
        map[QStringLiteral("variety")] = q.value(2);
        map[QStringLiteral("family")] = q.value(3);
        map[QStringLiteral("sowingSeason")] = q.value(4);
        map[QStringLiteral("harvestDays")] = q.value(5);
        map[QStringLiteral("rowSpacingCm")] = q.value(6);
        map[QStringLiteral("plantSpacingCm")] = q.value(7);
        map[QStringLiteral("notes")] = q.value(8);
    }
    return map;
}
