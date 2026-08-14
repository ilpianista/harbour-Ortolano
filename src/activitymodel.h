#pragma once
#include <QAbstractListModel>
#include <QSet>
#include <QSqlDatabase>
#include <QStringList>
#include <QVector>

struct ActivityEntry
{
    int id = 0;
    int activityTypeId = 0;
    QString activityTypeName;
    QSet<int> areaIds;
    QString areaNames;
    QString date;
    int speciesId = 0;
    QString speciesName;
    double quantity = 0;
    QString notes;
    QString section;
};

class ActivityModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int typeCount READ typeCount CONSTANT)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum TypeId {
        Digging = 0,
        Watering,
        Transplanting,
        Sowing,
        Fertilizing,
        Pruning,
        Weeding,
        Mulching,
        Harvest,
        Treatment
    };

    enum Roles {
        IdRole = Qt::UserRole + 1,
        ActivityTypeIdRole,
        ActivityTypeNameRole,
        AreaNamesRole,
        DateRole,
        SpeciesIdRole,
        SpeciesNameRole,
        QuantityRole,
        NotesRole,
        IsFutureRole,
        SectionRole
    };

    explicit ActivityModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int typeCount() const { return m_typeNames.size(); }
    int count() const { return m_entries.size(); }

    Q_INVOKABLE void loadPending();
    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void loadByArea(int areaId);
    Q_INVOKABLE QString activityTypeName(int index) const;
    Q_INVOKABLE int activityTypeId(int index) const;
    Q_INVOKABLE int harvestTypeId() const;
    Q_INVOKABLE bool addActivity(int activityTypeId,
                                 const QVariantList &areaIds,
                                 const QString &date,
                                 const QString &notes,
                                 int speciesId = 0,
                                 double quantity = 0);
    Q_INVOKABLE bool updateActivity(int id,
                                    int activityTypeId,
                                    const QVariantList &areaIds,
                                    const QString &date,
                                    const QString &notes,
                                    int speciesId = 0,
                                    double quantity = 0);
    Q_INVOKABLE bool deleteActivity(int id);
    Q_INVOKABLE QVariantMap get(int index) const;

signals:
    void countChanged();

private:
    void loadAreaNames(ActivityEntry &e);
    void setActivityAreas(int activityId, const QVariantList &areaIds);
    void loadSection(const QString &section);

    QSqlDatabase m_db;
    QVector<ActivityEntry> m_entries;
    QStringList m_typeNames;
};
