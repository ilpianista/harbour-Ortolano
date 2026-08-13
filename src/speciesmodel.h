#pragma once
#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVector>

struct SpeciesEntry
{
    int id = 0;
    QString name;
    QString variety;
    QString family;
    QString sowingSeason;
    int harvestDays = 0;
    int rowSpacingCm = 0;
    int plantSpacingCm = 0;
    QString notes;
};

class SpeciesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        VarietyRole,
        FamilyRole,
        SowingSeasonRole,
        HarvestDaysRole,
        RowSpacingRole,
        PlantSpacingRole,
        NotesRole
    };

    explicit SpeciesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE void search(const QString &text);
    Q_INVOKABLE bool addSpecies(const QString &name,
                                const QString &variety,
                                const QString &family,
                                const QString &sowingSeason,
                                int harvestDays,
                                int rowSpacingCm,
                                int plantSpacingCm,
                                const QString &notes);
    Q_INVOKABLE bool updateSpecies(int id,
                                   const QString &name,
                                   const QString &variety,
                                   const QString &family,
                                   const QString &sowingSeason,
                                   int harvestDays,
                                   int rowSpacingCm,
                                   int plantSpacingCm,
                                   const QString &notes);
    Q_INVOKABLE bool deleteSpecies(int id);
    Q_INVOKABLE QVariantMap getById(int id) const;

    int count() const { return m_entries.size(); }

signals:
    void countChanged();

private:
    QSqlDatabase m_db;
    QVector<SpeciesEntry> m_entries;
};
