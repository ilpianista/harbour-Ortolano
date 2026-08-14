#include "activitymodel.h"
#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

ActivityModel::ActivityModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_db = QSqlDatabase::database();
    m_typeNames << tr("Digging") << tr("Watering") << tr("Transplanting") << tr("Sowing")
                << tr("Fertilizing") << tr("Pruning") << tr("Weeding") << tr("Mulching")
                << tr("Harvest") << tr("Treatment");
}

int ActivityModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

QVariant ActivityModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const auto &e = m_entries.at(index.row());
    switch (role) {
    case IdRole:
        return e.id;
    case ActivityTypeIdRole:
        return e.activityTypeId;
    case ActivityTypeNameRole:
        return e.activityTypeName;
    case AreaNamesRole:
        return e.areaNames;
    case DateRole:
        return e.date;
    case SpeciesIdRole:
        return e.speciesId;
    case SpeciesNameRole:
        return e.speciesName;
    case QuantityRole:
        return e.quantity;
    case NotesRole:
        return e.notes;
    case IsFutureRole:
        return QDate::fromString(e.date.left(10), QStringLiteral("yyyy-MM-dd"))
               >= QDate::currentDate();
    case SectionRole:
        return e.section;
    default:
        return {};
    }
}

QHash<int, QByteArray> ActivityModel::roleNames() const
{
    return {{IdRole, "activityId"},
            {ActivityTypeIdRole, "activityTypeId"},
            {ActivityTypeNameRole, "activityTypeName"},
            {AreaNamesRole, "areaNames"},
            {DateRole, "date"},
            {SpeciesIdRole, "speciesId"},
            {SpeciesNameRole, "speciesName"},
            {QuantityRole, "quantity"},
            {NotesRole, "notes"},
            {IsFutureRole, "isFuture"},
            {SectionRole, "section"}};
}

QString ActivityModel::activityTypeName(int index) const
{
    if (index < 0 || index >= m_typeNames.size())
        return {};
    return m_typeNames.at(index);
}

int ActivityModel::activityTypeId(int index) const
{
    if (index < 0 || index >= m_typeNames.size())
        return -1;
    return index;
}

int ActivityModel::harvestTypeId() const
{
    return Harvest;
}

void ActivityModel::loadAreaNames(ActivityEntry &e)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT a.name FROM activity_areas aa "
                             "JOIN areas a ON aa.area_id = a.id "
                             "WHERE aa.activity_id = ? ORDER BY a.name"));
    q.addBindValue(e.id);
    q.exec();
    QStringList names;
    e.areaIds.clear();
    while (q.next()) {
        names << q.value(0).toString();
    }

    q.prepare(QStringLiteral("SELECT area_id FROM activity_areas WHERE activity_id = ?"));
    q.addBindValue(e.id);
    q.exec();
    while (q.next()) {
        e.areaIds.insert(q.value(0).toInt());
    }

    e.areaNames = names.join(QStringLiteral(", "));
}

QString ActivityModel::resolveTypeName(int typeId) const
{
    if (typeId < 0 || typeId >= m_typeNames.size())
        return {};
    return m_typeNames.at(typeId);
}

void ActivityModel::setActivityAreas(int activityId, const QVariantList &areaIds)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM activity_areas WHERE activity_id = ?"));
    q.addBindValue(activityId);
    q.exec();

    q.prepare(QStringLiteral("INSERT INTO activity_areas (activity_id, area_id) VALUES (?, ?)"));
    for (const QVariant &id : areaIds) {
        q.addBindValue(activityId);
        q.addBindValue(id.toInt());
        q.exec();
    }
}

void ActivityModel::loadPending()
{
    beginResetModel();
    m_entries.clear();
    loadSection(QStringLiteral("pending"));
    endResetModel();
    emit countChanged();
}

void ActivityModel::loadDone()
{
    beginResetModel();
    m_entries.clear();
    loadSection(QStringLiteral("completed"));
    endResetModel();
    emit countChanged();
}

void ActivityModel::loadAll()
{
    beginResetModel();
    m_entries.clear();
    loadSection(QStringLiteral("pending"));
    loadSection(QStringLiteral("completed"));
    endResetModel();
    emit countChanged();
}

void ActivityModel::loadSection(const QString &section)
{
    const bool pending = section == QStringLiteral("pending");
    const QString dateOp = pending ? QStringLiteral(">=") : QStringLiteral("<");
    const QString order = pending ? QStringLiteral("ASC") : QStringLiteral("DESC");

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT a.id, a.activity_type_id, a.date, a.notes, "
                             "a.species_id, s.name, a.quantity, 0 AS source, NULL AS "
                             "planting_area_id, '' AS planting_area_name "
                             "FROM activities a "
                             "LEFT JOIN species s ON a.species_id = s.id "
                             "WHERE a.date %1 ? "
                             "UNION ALL "
                             "SELECT -p.id, NULL, p.planted_date, '', "
                             "p.species_id, sp.name, 0, 1, p.area_id, ar.name "
                             "FROM plantings p "
                             "JOIN species sp ON p.species_id = sp.id "
                             "JOIN areas ar ON p.area_id = ar.id "
                             "WHERE p.planted_date %1 ? "
                             "ORDER BY date %2")
                  .arg(dateOp, order));
    QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    q.addBindValue(today);
    q.addBindValue(today);
    q.exec();
    while (q.next()) {
        ActivityEntry e;
        e.id = q.value(0).toInt();
        e.date = q.value(2).toString();
        e.notes = q.value(3).toString();
        e.speciesId = q.value(4).toInt();
        e.speciesName = q.value(5).toString();
        e.quantity = q.value(6).toDouble();
        e.section = section;
        int source = q.value(7).toInt();

        if (source == 0) {
            e.activityTypeId = q.value(1).toInt();
            e.activityTypeName = resolveTypeName(e.activityTypeId);
            loadAreaNames(e);
        } else {
            e.activityTypeName = tr("Planted");
            int pAreaId = q.value(8).toInt();
            QString pAreaName = q.value(9).toString();
            e.areaIds.insert(pAreaId);
            e.areaNames = pAreaName;
        }

        m_entries.append(e);
    }
}

void ActivityModel::loadByArea(int areaId)
{
    beginResetModel();
    m_entries.clear();

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT a.id, a.activity_type_id, a.date, a.notes, "
                             "a.species_id, s.name, a.quantity "
                             "FROM activities a "
                             "JOIN activity_areas aa ON aa.activity_id = a.id "
                             "LEFT JOIN species s ON a.species_id = s.id "
                             "WHERE aa.area_id = ? "
                             "ORDER BY a.date DESC"));
    q.addBindValue(areaId);
    q.exec();
    while (q.next()) {
        ActivityEntry e;
        e.id = q.value(0).toInt();
        e.activityTypeId = q.value(1).toInt();
        e.activityTypeName = resolveTypeName(e.activityTypeId);
        e.date = q.value(2).toString();
        e.notes = q.value(3).toString();
        e.speciesId = q.value(4).toInt();
        e.speciesName = q.value(5).toString();
        e.quantity = q.value(6).toDouble();
        loadAreaNames(e);
        m_entries.append(e);
    }

    endResetModel();
    emit countChanged();
}

bool ActivityModel::addActivity(int activityTypeId,
                                const QVariantList &areaIds,
                                const QString &date,
                                const QString &notes,
                                int speciesId,
                                double quantity)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO activities (activity_type_id, date, notes, species_id, quantity) "
        "VALUES (?, ?, ?, ?, ?)"));
    q.addBindValue(activityTypeId);
    q.addBindValue(date);
    q.addBindValue(notes);
    q.addBindValue(speciesId > 0 ? speciesId : QVariant(QVariant::Int));
    q.addBindValue(quantity);
    if (!q.exec()) {
        qWarning() << "addActivity error:" << q.lastError().text();
        return false;
    }

    int newId = q.lastInsertId().toInt();
    setActivityAreas(newId, areaIds);

    ActivityEntry e;
    e.id = newId;
    e.activityTypeId = activityTypeId;
    e.activityTypeName = resolveTypeName(activityTypeId);
    e.date = date;
    e.notes = notes;
    e.speciesId = speciesId;
    e.quantity = quantity;

    if (speciesId > 0) {
        QSqlQuery spQ(m_db);
        spQ.prepare(QStringLiteral("SELECT name FROM species WHERE id=?"));
        spQ.addBindValue(speciesId);
        if (spQ.exec() && spQ.next())
            e.speciesName = spQ.value(0).toString();
    }

    loadAreaNames(e);

    beginInsertRows(QModelIndex(), 0, 0);
    m_entries.prepend(e);
    endInsertRows();
    emit countChanged();
    return true;
}

bool ActivityModel::updateActivity(int id,
                                   int activityTypeId,
                                   const QVariantList &areaIds,
                                   const QString &date,
                                   const QString &notes,
                                   int speciesId,
                                   double quantity)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE activities SET activity_type_id=?, date=?, notes=?, "
                             "species_id=?, quantity=? WHERE id=?"));
    q.addBindValue(activityTypeId);
    q.addBindValue(date);
    q.addBindValue(notes);
    q.addBindValue(speciesId > 0 ? speciesId : QVariant(QVariant::Int));
    q.addBindValue(quantity);
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "updateActivity error:" << q.lastError().text();
        return false;
    }

    setActivityAreas(id, areaIds);

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == id) {
            m_entries[i].activityTypeId = activityTypeId;
            m_entries[i].activityTypeName = resolveTypeName(activityTypeId);
            m_entries[i].date = date;
            m_entries[i].notes = notes;
            m_entries[i].speciesId = speciesId;
            m_entries[i].quantity = quantity;
            loadAreaNames(m_entries[i]);

            if (speciesId > 0) {
                QSqlQuery spQ(m_db);
                spQ.prepare(QStringLiteral("SELECT name FROM species WHERE id=?"));
                spQ.addBindValue(speciesId);
                if (spQ.exec() && spQ.next())
                    m_entries[i].speciesName = spQ.value(0).toString();
            }

            emit dataChanged(index(i), index(i));
            return true;
        }
    }
    return false;
}

bool ActivityModel::deleteActivity(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM activity_areas WHERE activity_id=?"));
    q.addBindValue(id);
    q.exec();

    q.prepare(QStringLiteral("DELETE FROM activities WHERE id=?"));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "deleteActivity error:" << q.lastError().text();
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

QVariantMap ActivityModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_entries.size())
        return map;

    const auto &e = m_entries.at(index);
    map[QStringLiteral("activityId")] = e.id;
    map[QStringLiteral("activityTypeId")] = e.activityTypeId;
    map[QStringLiteral("activityTypeName")] = e.activityTypeName;
    map[QStringLiteral("date")] = e.date;
    map[QStringLiteral("notes")] = e.notes;
    map[QStringLiteral("speciesId")] = e.speciesId;
    map[QStringLiteral("speciesName")] = e.speciesName;
    map[QStringLiteral("quantity")] = e.quantity;
    map[QStringLiteral("areaNames")] = e.areaNames;

    QVariantList ids;
    for (int id : e.areaIds)
        ids << id;
    map[QStringLiteral("areaIds")] = ids;

    return map;
}
