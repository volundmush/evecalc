#ifndef EVEMARKET_H
#define EVEMARKET_H

#include <QSortFilterProxyModel>
#include <QObject>
#include <QStandardItem>
#include <QStandardItemModel>
#include "evedatabase.h"
#include <QWidget>
#include <QTreeView>
#include <QTableView>
#include <QCompleter>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QModelIndex>
#include <QString>
#include <QTextEdit>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantHash>
#include <QSqlError>
#include <QStyledItemDelegate>
#include <iomanip>
#include <sstream>
#include <QHeaderView>

#include "esibase.h"
#include "evedatabase.h"

class EveMarketDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    EveMarketDelegate(QObject *parent);
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;

};


class EveMarketTree : public QStandardItemModel {
    // Sub-class which sets up its own data. Nothing really special about it.
    Q_OBJECT
public:
    EveMarketTree(QObject *parent = Q_NULLPTR);
};

class EveMarketList : public QStandardItemModel {
    // Sub-class that's a flat list of EveTypes. No tree data.
    Q_OBJECT
public:
    EveMarketList(QObject *parent = Q_NULLPTR);
};

class EveMarketTreeFilter : public QSortFilterProxyModel {
    // This sub-class reimplements filterAcceptsRow() so that if children match the filter, parents also match. This prevents categories from being filtered out when searching the market tree!
    Q_OBJECT
public:
    EveMarketTreeFilter(QObject *parent = Q_NULLPTR);
protected:
    virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
};

class EveMarketCategory : public QStandardItem {
    // Sub-class of QStandardItem designed to simplify the recursive filling of the Market. Hooray constructors!
public:
    EveMarketCategory(int MarketID);
};

class EveMarketItem : public QStandardItem {
    // Sub-class of QStandardItem that automates/simplifies setup of a Market browser item.
public:
    EveMarketItem(int ItemID);
};

class EveMarketTab : public QWidget
{
    Q_OBJECT
public:
    EveMarketTab(QWidget *parent = Q_NULLPTR);

    EveMarketTreeFilter *filter_markettree;
    QTreeView *view_markettree;
    QTableView *view_buyorders, *view_sellorders;
    QCompleter *completer_market;
    QVBoxLayout *market_tree_vlayout, *market_orders_vlayout;
    QHBoxLayout *hlocation_layout, *hmarket_splitter;
    QLineEdit *search;
    QTextEdit *mainbox;
    QComboBox *region_choice, *system_choice, *station_choice;
    unsigned int current_region, current_system, current_station, current_type;
    void reloadMarket();

public slots:
    void displayItem(QModelIndex idx);
    void filterMarket(QString find);
    void changeRegion(int idx);
    void reloadSystemList(int regionID);
    void receiveRegionOrders();
};

#endif // EVEMARKET_H
