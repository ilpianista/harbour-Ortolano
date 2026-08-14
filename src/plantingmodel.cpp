#include "plantingmodel.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

PlantingModel::PlantingModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_db = QSqlDatabase::database();
}

int PlantingModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

QVariant PlantingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const auto &e = m_entries.at(index.row());
    switch (role) {
    case IdRole:
        return e.id;
    case SpeciesIdRole:
        return e.speciesId;
    case SpeciesNameRole:
        return e.speciesName;
    case PlantedDateRole:
        return e.plantedDate;
    case SowingTypeRole:
        return e.sowingType;
    case SowingDetailRole:
        return e.sowingDetail;
    case NotesRole:
        return e.notes;
    default:
        return {};
    }
}

QHash<int, QByteArray> PlantingModel::roleNames() const
{
    return {{IdRole, "plantingId"},
            {SpeciesIdRole, "speciesId"},
            {SpeciesNameRole, "speciesName"},
            {PlantedDateRole, "plantedDate"},
            {SowingTypeRole, "sowingType"},
            {SowingDetailRole, "sowingDetail"},
            {NotesRole, "notes"}};
}

void PlantingModel::loadByArea(int areaId)
{
    beginResetModel();
    m_entries.clear();

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT p.id, p.species_id, s.name, p.planted_date, p.sowing_type, "
                             "p.sowing_detail, p.notes "
                             "FROM plantings p "
                             "JOIN species s ON p.species_id = s.id "
                             "WHERE p.area_id = ? ORDER BY p.planted_date DESC"));
    q.addBindValue(areaId);
    q.exec();
    while (q.next()) {
        PlantingEntry e;
        e.id = q.value(0).toInt();
        e.speciesId = q.value(1).toInt();
        e.speciesName = q.value(2).toString();
        e.plantedDate = q.value(3).toString();
        e.sowingType = q.value(4).toString();
        e.sowingDetail = q.value(5).toInt();
        e.notes = q.value(6).toString();
        m_entries.append(e);
    }

    endResetModel();
    emit countChanged();
}

bool PlantingModel::addPlanting(int areaId,
                                int speciesId,
                                const QString &plantedDate,
                                const QString &sowingType,
                                int sowingDetail,
                                const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO plantings (area_id, species_id, planted_date, "
                             "sowing_type, sowing_detail, notes) "
                             "VALUES (?, ?, ?, ?, ?, ?)"));
    q.addBindValue(areaId);
    q.addBindValue(speciesId);
    q.addBindValue(plantedDate);
    q.addBindValue(sowingType);
    q.addBindValue(sowingDetail);
    q.addBindValue(notes);
    if (!q.exec()) {
        qWarning() << "addPlanting error:" << q.lastError().text();
        return false;
    }

    PlantingEntry e;
    e.id = q.lastInsertId().toInt();
    e.speciesId = speciesId;
    e.plantedDate = plantedDate;
    e.sowingType = sowingType;
    e.sowingDetail = sowingDetail;
    e.notes = notes;

    QSqlQuery nq(m_db);
    nq.prepare(QStringLiteral("SELECT name FROM species WHERE id = ?"));
    nq.addBindValue(speciesId);
    if (nq.exec() && nq.next())
        e.speciesName = nq.value(0).toString();

    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append(e);
    endInsertRows();
    emit countChanged();
    return true;
}

bool PlantingModel::updatePlanting(int id,
                                   int speciesId,
                                   const QString &plantedDate,
                                   const QString &sowingType,
                                   int sowingDetail,
                                   const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE plantings SET species_id=?, planted_date=?, "
                             "sowing_type=?, sowing_detail=?, notes=? WHERE id=?"));
    q.addBindValue(speciesId);
    q.addBindValue(plantedDate);
    q.addBindValue(sowingType);
    q.addBindValue(sowingDetail);
    q.addBindValue(notes);
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "updatePlanting error:" << q.lastError().text();
        return false;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == id) {
            m_entries[i].speciesId = speciesId;
            m_entries[i].plantedDate = plantedDate;
            m_entries[i].sowingType = sowingType;
            m_entries[i].sowingDetail = sowingDetail;
            m_entries[i].notes = notes;

            QSqlQuery nq(m_db);
            nq.prepare(QStringLiteral("SELECT name FROM species WHERE id = ?"));
            nq.addBindValue(speciesId);
            if (nq.exec() && nq.next())
                m_entries[i].speciesName = nq.value(0).toString();

            emit dataChanged(index(i), index(i));
            return true;
        }
    }
    return false;
}

bool PlantingModel::deletePlanting(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM plantings WHERE id=?"));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "deletePlanting error:" << q.lastError().text();
        return false;
    }

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

QVariantMap PlantingModel::getById(int id) const
{
    QVariantMap map;
    QSqlQuery q(m_db);
    q.prepare(
        QStringLiteral("SELECT p.id, p.species_id, p.planted_date, p.sowing_type, p.sowing_detail, "
                       "p.notes, s.name "
                       "FROM plantings p JOIN species s ON p.species_id = s.id WHERE p.id = ?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        map[QStringLiteral("plantingId")] = q.value(0);
        map[QStringLiteral("speciesId")] = q.value(1);
        map[QStringLiteral("plantedDate")] = q.value(2);
        map[QStringLiteral("sowingType")] = q.value(3);
        map[QStringLiteral("sowingDetail")] = q.value(4);
        map[QStringLiteral("notes")] = q.value(5);
        map[QStringLiteral("speciesName")] = q.value(6);
    }
    return map;
}
