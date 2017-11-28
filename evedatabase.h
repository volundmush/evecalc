#ifndef EVEDATABASE_H
#define EVEDATABASE_H

#include <QWidget>
#include <QString>
#include <QTreeWidgetItem>
#include <QSqlRecord>
#include <QList>
#include <QMap>
#include <QHash>
#include <QSqlDatabase>
#include <QVariant>
#include <QSqlQuery>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QIcon>
#include <QObject>
#include <QSqlRelationalTableModel>

class EveDB;
class EveMarketOrder;

class EveDB : public QObject
{
    Q_OBJECT
public:
    EveDB();
    ~EveDB();
    QSqlRelationalTableModel *EveRegions, *EveConstellations, *EveSystems, *EveStations, *EveTypes, *EveReprocess, *EveBuyOrders, *EveSellOrders;
    QStandardItemModel *market_model_tree, *market_model_flat;

    QHash<QString, QIcon> EveTypeIcons;
    QHash<QString, QIcon> EveMarketIcons;
    QHash<QString, QString> EveIconMaps;

    QHash<qint64, EveMarketOrder*> EveMarketOrders;
    QSqlDatabase db;
    QString file_path;
    QIcon getIcon(QString fileName);
    QIcon getRender(QString fileName);
    QIcon getPic(QString fileName);
    void readyTables();
    void loadIconMap();
    void loadMap();
    void loadItems();
    //void requestPrices();

public slots:
    //void receivePrices(QJsonDocument doc);
};

extern EveDB *EVE;

#endif // EVEDATABASE_H
