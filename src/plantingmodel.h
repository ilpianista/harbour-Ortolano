#pragma once
#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVector>

struct PlantingEntry
{
    int id = 0;
    int areaId = 0;
    int speciesId = 0;
    QString speciesName;
    QString areaName;
    int plantsCount = 0;
    QString plantedDate;
    QString sowingType;
    int sowingDetail = 0;
    QString notes;
};

class PlantingModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        AreaIdRole,
        SpeciesIdRole,
        SpeciesNameRole,
        AreaNameRole,
        PlantsCountRole,
        PlantedDateRole,
        SowingTypeRole,
        SowingDetailRole,
        NotesRole
    };

    explicit PlantingModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadByArea(int areaId);
    Q_INVOKABLE bool addPlanting(int areaId,
                                 int speciesId,
                                 int plantsCount,
                                 const QString &plantedDate,
                                 const QString &sowingType,
                                 int sowingDetail,
                                 const QString &notes);
    Q_INVOKABLE bool updatePlanting(int id,
                                    int speciesId,
                                    int plantsCount,
                                    const QString &plantedDate,
                                    const QString &sowingType,
                                    int sowingDetail,
                                    const QString &notes);
    Q_INVOKABLE bool deletePlanting(int id);
    Q_INVOKABLE QVariantMap getById(int id) const;

    int count() const { return m_entries.size(); }

signals:
    void countChanged();

private:
    QSqlDatabase m_db;
    QVector<PlantingEntry> m_entries;
};
