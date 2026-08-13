#include "areamodel.h"
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

AreaModel::AreaModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_db = QSqlDatabase::database();
}

int AreaModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

QVariant AreaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};

    const auto &e = m_entries.at(index.row());
    switch (role) {
    case IdRole:
        return e.id;
    case NameRole:
        return e.name;
    case GridRowRole:
        return e.gridRow;
    case GridColRole:
        return e.gridCol;
    case GridRowsRole:
        return e.gridRows;
    case GridColsRole:
        return e.gridCols;
    case WidthMRole:
        return e.widthM;
    case LengthMRole:
        return e.lengthM;
    case NotesRole:
        return e.notes;
    case PlantsRole:
        return e.plants;
    default:
        return {};
    }
}

QHash<int, QByteArray> AreaModel::roleNames() const
{
    return {{IdRole, "areaId"},
            {NameRole, "name"},
            {GridRowRole, "gridRow"},
            {GridColRole, "gridCol"},
            {GridRowsRole, "gridRows"},
            {GridColsRole, "gridCols"},
            {WidthMRole, "widthM"},
            {LengthMRole, "lengthM"},
            {NotesRole, "notes"},
            {PlantsRole, "plants"}};
}

void AreaModel::loadAll()
{
    beginResetModel();
    m_entries.clear();

    QSqlQuery q(m_db);
    q.exec(QStringLiteral("SELECT id, name, grid_row, grid_col, grid_rows, grid_cols, "
                          "width_m, length_m, notes FROM areas ORDER BY grid_row, grid_col"));
    while (q.next()) {
        AreaEntry e;
        e.id = q.value(0).toInt();
        e.name = q.value(1).toString();
        e.gridRow = q.value(2).toInt();
        e.gridCol = q.value(3).toInt();
        e.gridRows = q.value(4).toInt();
        e.gridCols = q.value(5).toInt();
        e.widthM = q.value(6).toDouble();
        e.lengthM = q.value(7).toDouble();
        e.notes = q.value(8).toString();
        m_entries.append(e);
    }

    for (auto &e : m_entries) {
        QSqlQuery pq(m_db);
        pq.prepare(QStringLiteral("SELECT s.name FROM plantings p "
                                  "JOIN species s ON p.species_id = s.id "
                                  "WHERE p.area_id = ? ORDER BY s.name"));
        pq.addBindValue(e.id);
        pq.exec();
        QStringList names;
        while (pq.next())
            names << pq.value(0).toString();
        e.plants = names.join(QStringLiteral(", "));
    }

    endResetModel();
    emit countChanged();
}

bool AreaModel::addArea(const QString &name,
                        int gridRow,
                        int gridCol,
                        int gridRows,
                        int gridCols,
                        double widthM,
                        double lengthM,
                        const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("INSERT INTO areas (name, grid_row, grid_col, grid_rows, grid_cols, "
                             "width_m, length_m, notes) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
    q.addBindValue(name);
    q.addBindValue(gridRow);
    q.addBindValue(gridCol);
    q.addBindValue(gridRows);
    q.addBindValue(gridCols);
    q.addBindValue(widthM);
    q.addBindValue(lengthM);
    q.addBindValue(notes);
    if (!q.exec()) {
        qWarning() << "addArea error:" << q.lastError().text();
        return false;
    }

    AreaEntry e;
    e.id = q.lastInsertId().toInt();
    e.name = name;
    e.gridRow = gridRow;
    e.gridCol = gridCol;
    e.gridRows = gridRows;
    e.gridCols = gridCols;
    e.widthM = widthM;
    e.lengthM = lengthM;
    e.notes = notes;
    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append(e);
    endInsertRows();
    emit countChanged();
    return true;
}

bool AreaModel::updateArea(int id,
                           const QString &name,
                           int gridRow,
                           int gridCol,
                           int gridRows,
                           int gridCols,
                           double widthM,
                           double lengthM,
                           const QString &notes)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE areas SET name=?, grid_row=?, grid_col=?, grid_rows=?, "
                             "grid_cols=?, width_m=?, length_m=?, notes=? WHERE id=?"));
    q.addBindValue(name);
    q.addBindValue(gridRow);
    q.addBindValue(gridCol);
    q.addBindValue(gridRows);
    q.addBindValue(gridCols);
    q.addBindValue(widthM);
    q.addBindValue(lengthM);
    q.addBindValue(notes);
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "updateArea error:" << q.lastError().text();
        return false;
    }

    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == id) {
            m_entries[i].name = name;
            m_entries[i].gridRow = gridRow;
            m_entries[i].gridCol = gridCol;
            m_entries[i].gridRows = gridRows;
            m_entries[i].gridCols = gridCols;
            m_entries[i].widthM = widthM;
            m_entries[i].lengthM = lengthM;
            m_entries[i].notes = notes;
            emit dataChanged(index(i), index(i));
            return true;
        }
    }
    return false;
}

bool AreaModel::deleteArea(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM plantings WHERE area_id=?"));
    q.addBindValue(id);
    q.exec();

    q.prepare(QStringLiteral("DELETE FROM activity_areas WHERE area_id=?"));
    q.addBindValue(id);
    q.exec();

    q.prepare(QStringLiteral("DELETE FROM areas WHERE id=?"));
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "deleteArea error:" << q.lastError().text();
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

QVariantMap AreaModel::getById(int id) const
{
    QVariantMap map;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT id, name, grid_row, grid_col, grid_rows, grid_cols, "
                             "width_m, length_m, notes FROM areas WHERE id=?"));
    q.addBindValue(id);
    if (q.exec() && q.next()) {
        map[QStringLiteral("id")] = q.value(0);
        map[QStringLiteral("name")] = q.value(1);
        map[QStringLiteral("gridRow")] = q.value(2);
        map[QStringLiteral("gridCol")] = q.value(3);
        map[QStringLiteral("gridRows")] = q.value(4);
        map[QStringLiteral("gridCols")] = q.value(5);
        map[QStringLiteral("widthM")] = q.value(6);
        map[QStringLiteral("lengthM")] = q.value(7);
        map[QStringLiteral("notes")] = q.value(8);
    }
    return map;
}

QVariantMap AreaModel::get(int index) const
{
    QVariantMap map;
    if (index < 0 || index >= m_entries.size())
        return map;

    const auto &e = m_entries.at(index);
    map[QStringLiteral("areaId")] = e.id;
    map[QStringLiteral("name")] = e.name;
    map[QStringLiteral("gridRow")] = e.gridRow;
    map[QStringLiteral("gridCol")] = e.gridCol;
    map[QStringLiteral("gridRows")] = e.gridRows;
    map[QStringLiteral("gridCols")] = e.gridCols;
    map[QStringLiteral("widthM")] = e.widthM;
    map[QStringLiteral("lengthM")] = e.lengthM;
    map[QStringLiteral("notes")] = e.notes;
    return map;
}

int AreaModel::gardenRows() const
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT value FROM settings WHERE key = 'garden_rows'"));
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 8;
}

int AreaModel::gardenCols() const
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT value FROM settings WHERE key = 'garden_cols'"));
    if (q.exec() && q.next())
        return q.value(0).toInt();
    return 10;
}

void AreaModel::setGardenRows(int rows)
{
    QSqlQuery q(m_db);
    q.prepare(
        QStringLiteral("INSERT OR REPLACE INTO settings (key, value) VALUES ('garden_rows', ?)"));
    q.addBindValue(QString::number(rows));
    q.exec();
}

void AreaModel::setGardenCols(int cols)
{
    QSqlQuery q(m_db);
    q.prepare(
        QStringLiteral("INSERT OR REPLACE INTO settings (key, value) VALUES ('garden_cols', ?)"));
    q.addBindValue(QString::number(cols));
    q.exec();
}
