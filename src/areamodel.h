#pragma once
#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVector>

struct AreaEntry
{
    int id = 0;
    QString name;
    int gridRow = 0;
    int gridCol = 0;
    int gridRows = 1;
    int gridCols = 1;
    double widthM = 1.0;
    double lengthM = 1.0;
    QString notes;
    QString plants;
};

class AreaModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        GridRowRole,
        GridColRole,
        GridRowsRole,
        GridColsRole,
        WidthMRole,
        LengthMRole,
        NotesRole,
        PlantsRole
    };

    explicit AreaModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void loadAll();
    Q_INVOKABLE bool addArea(const QString &name,
                             int gridRow,
                             int gridCol,
                             int gridRows,
                             int gridCols,
                             double widthM,
                             double lengthM,
                             const QString &notes);
    Q_INVOKABLE bool updateArea(int id,
                                const QString &name,
                                int gridRow,
                                int gridCol,
                                int gridRows,
                                int gridCols,
                                double widthM,
                                double lengthM,
                                const QString &notes);
    Q_INVOKABLE bool deleteArea(int id);
    Q_INVOKABLE QVariantMap getById(int id) const;
    Q_INVOKABLE QVariantMap get(int index) const;

    Q_INVOKABLE int gardenRows() const;
    Q_INVOKABLE int gardenCols() const;
    Q_INVOKABLE void setGardenRows(int rows);
    Q_INVOKABLE void setGardenCols(int cols);

    int count() const { return m_entries.size(); }

signals:
    void countChanged();

private:
    QSqlDatabase m_db;
    QVector<AreaEntry> m_entries;
};
