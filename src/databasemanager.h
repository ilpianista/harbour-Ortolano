#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(QObject *parent = nullptr);

    Q_INVOKABLE bool initDatabase(const QString &path);

private:
    void createTables();
    void seedData();

    QSqlDatabase m_db;
};
