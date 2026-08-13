#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class DatabaseManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit DatabaseManager(QObject *parent = nullptr);

    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool initDatabase(const QString &path);

signals:
    void lastErrorChanged();

private:
    void createTables();
    void seedData();
    void setLastError(const QString &error);

    QSqlDatabase m_db;
    QString m_lastError;
};
