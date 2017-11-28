#include "evedatabase.h"
#include <QSqlQuery>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantHash>


EveDB::EveDB()
{
    db = QSqlDatabase::addDatabase("QMYSQL", "default");
    db.setHostName("localhost");
    db.setDatabaseName("evecalc");
    db.setUserName("root");
    db.setPassword("Draculina7");
    if (!db.open())
    {
        qDebug() << "We are not database-ok!";
    }
    readyTables();
    loadIconMap();
    loadMap();
    loadItems();
}

EveDB::~EveDB()
{
    db.close();
}

void EveDB::loadMap()
{
    EveRegions = new QSqlRelationalTableModel(this, db);
    EveRegions->setTable("MarketRegions");
    EveRegions->setSort(1, Qt::AscendingOrder);
    EveRegions->setHeaderData(0, Qt::Horizontal, tr("ID"), Qt::UserRole);
    EveRegions->setHeaderData(1, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
    EveRegions->select();

    EveConstellations = new QSqlRelationalTableModel(this, db);
    EveConstellations->setTable("MarketConstellations");
    EveConstellations->select();

    EveSystems = new QSqlRelationalTableModel(this, db);
    EveSystems->setTable("MarketSystems");
    EveSystems->select();

    EveStations = new QSqlRelationalTableModel(this, db);
    EveStations->setTable("v_MarketLocations");
    EveStations->setSort(11, Qt::AscendingOrder);
    EveStations->setRelation(8, QSqlRelation("mapSolarSystems", "solarSystemID", "solarSystemName"));
    EveStations->setRelation(9, QSqlRelation("mapConstellations", "constellationID", "constellationName"));
    EveStations->setRelation(10, QSqlRelation("mapRegions", "regionID", "regionName"));
    EveStations->select();

}



void EveDB::loadIconMap()
{
    QString mapfile("iconmap.txt");
    QFile mapping(mapfile);
    mapping.open(QFile::ReadOnly);

    QTextStream in(&mapping);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList qsl = line.split(' ');
        EveIconMaps.insert(qsl.at(0), qsl.at(1));
    }
    mapping.close();
}

QIcon EveDB::getIcon(QString fileName)
{
    QString real_filename = EveIconMaps.value(fileName);
    if (EveTypeIcons.contains(real_filename)) {
        return EveTypeIcons.value(real_filename);
    }
    QString path("icons/types/" + real_filename);
    QIcon new_icon(path);
    EveTypeIcons.insert(real_filename, new_icon);
    return new_icon;
}

QIcon EveDB::getPic(QString fileName)
{
    if (EveMarketIcons.contains(fileName)) {
        return EveMarketIcons.value(fileName);
    }
    QString path = "icons/market/" + fileName;
    QIcon new_icon(path);
    EveMarketIcons.insert(fileName, new_icon);
    return new_icon;
}

void EveDB::readyTables()
{
    QSqlQuery q(db);
    q.exec("CREATE TABLE IF NOT EXISTS calcBlueprints (typeID BIGINT UNSIGNED NOT NULL,research_material INTEGER UNSIGNED DEFAULT 0,research_time INTEGER DEFAULT 0,FOREIGN KEY(typeID) REFERENCES evesde.invTypes(typeID))");
    q.exec("CREATE TABLE IF NOT EXISTS calcProduction (typeID BIGINT UNSIGNED NOT NULL,quantity INTEGER IMSOGMED DEFAULT 1,FOREIGN KEY(typeID) REFERENCES evesde.invTypes(typeID))");
    q.exec("DELETE FROM MarketOrders");

    q.exec("DELETE from MarketOrders");
    q.exec("DELETE FROM Structures");
    q.exec("DELETE FROM DockableMarkets");

    q.exec("INSERT INTO MarketRegions (regionID,regionName) SELECT regionID,regionName FROM evesde.mapRegions WHERE regionID<11000001");

    q.exec("INSERT INTO MarketConstellations (constellationID,constellationName,regionID) SELECT constellationID,constellationName,regionID FROM evesde.mapConstellations WHERE regionID IN (SELECT regionID FROM MarketRegions)");

    q.exec("INSERT INTO MarketSystems (solarSystemID,solarSystemName,security,constellationID) SELECT solarSystemID,solarSystemName,security,constellationID FROM evesde.mapSolarSystems WHERE constellationID IN (SELECT constellationID FROM MarketConstellations)");

    q.exec("INSERT INTO DockableMarkets (locationID,stationName,solarSystemID,source_type) SELECT stationID,stationName,solarSystemID,0 FROM v_MarketLocations");


    EveBuyOrders = new QSqlRelationalTableModel(this, db);
    EveBuyOrders->setTable("v_FinalMarketOrders");
    EveBuyOrders->setSort(5, Qt::DescendingOrder);
    EveBuyOrders->setFilter("is_buy_order=1");
    EveBuyOrders->select();

    EveSellOrders = new QSqlRelationalTableModel(this, db);
    EveSellOrders->setTable("v_FinalMarketOrders");
    EveSellOrders->setSort(5, Qt::DescendingOrder);
    EveSellOrders->setFilter("is_buy_order=0");
    EveSellOrders->select();



}


void EveDB::loadItems()
{
    EveTypes = new QSqlRelationalTableModel(this, db);
    EveTypes->setTable("v_MarketItems");
    EveTypes->select();

    EveReprocess = new QSqlRelationalTableModel(this, db);
    EveReprocess->setTable("invMaterials");
    EveReprocess->select();


    QStandardItem *to_ins, *new_mar;
    QSqlQuery q(db);
    q.exec("SELECT g.parentGroupID,g.marketGroupID,g.marketGroupName,i.iconFile FROM evesde.invMarketGroups AS g LEFT JOIN evesde.eveIcons AS i ON i.iconID=g.iconID ORDER BY g.parentGroupID,g.marketGroupName");

    market_model_tree = new QStandardItemModel(this);
    QHash<qint64, QStandardItem*> market_map;

    while (q.next()) {
        QSqlRecord rec = q.record();
        QStringList filepath = rec.value(3).toString().split('/');

        new_mar = new QStandardItem(getPic(filepath.value(4)), rec.value(2).toString());
        new_mar->setFlags(Qt::ItemIsEnabled);
        market_map.insert(rec.value(1).toLongLong(), new_mar);
    }

    q.exec("SELECT parentGroupID,marketGroupID FROM evesde.invMarketGroups ORDER BY marketGroupName");
    while (q.next()) {
        QSqlRecord rec = q.record();
        int parent_id = rec.value(0).toLongLong();
        new_mar = market_map.value(rec.value(1).toLongLong());
        if (parent_id) {
            to_ins = market_map.value(parent_id);
            to_ins->appendRow(new_mar);
        }
        else {
            market_model_tree->appendRow(new_mar);
        }

    }

    q.exec("SELECT t.marketGroupID,t.typeID,t.typeName FROM v_MarketItems AS t ORDER BY t.typeName");
    market_model_flat = new QStandardItemModel(this);

    while (q.next()) {
        QSqlRecord rec = q.record();
        QString fileName = rec.value(1).toString() + "_32.png";
        QIcon typeIcon = getIcon(fileName);
        QString itemName = rec.value(2).toString();
        new_mar = new QStandardItem(getIcon(fileName), rec.value(2).toString());
        new_mar->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren);
        new_mar->setData(rec.value(1).toLongLong());
        market_model_flat->appendRow(new_mar);

        new_mar = new QStandardItem(getIcon(fileName), rec.value(2).toString());
        new_mar->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren);
        new_mar->setData(rec.value(1).toLongLong());
        to_ins = market_map.value(rec.value(0).toLongLong());
        to_ins->appendRow(new_mar);
    }

}
