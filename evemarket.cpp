#include "evemarket.h"

EveMarketDelegate::EveMarketDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //pass
}

QString EveMarketDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if(value.userType() == QVariant::Double)
    {

        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << value.toDouble();
        stream.imbue(std::locale(""));
        return QString::fromStdString(stream.str());

    }
    return value.toString();
}

EveMarketTreeFilter::EveMarketTreeFilter(QObject *parent) : QSortFilterProxyModel(parent)
{
    // Alright this should do it?
}

bool EveMarketTreeFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

        if (!index.isValid())
            return false;

        if (index.data().toString().contains(filterRegExp()))
            return true;

        int rows = sourceModel()->rowCount(index);
        for (int row = 0; row < rows; row++)
            if (filterAcceptsRow(row, index))
                return true;

        return false;
}

void EveMarketTab::filterMarket(QString find)
{
    filter_markettree->setFilterRegExp(QRegExp(find, Qt::CaseInsensitive, QRegExp::Wildcard));
    filter_markettree->setFilterKeyColumn(0);
    int find_length = find.length();
    if (find_length) {
        view_markettree->expandAll();
    }
    else {
        view_markettree->collapseAll();
    }
}

void EveMarketTab::displayItem(QModelIndex idx)
{
    QModelIndex src = filter_markettree->mapToSource(idx);
    QStandardItem *wdg = EVE->market_model_tree->itemFromIndex(src);
    if (wdg->isSelectable()) {
        int typeID = wdg->data().toInt();
        current_type = typeID;
        if(current_type && current_region) {
            reloadMarket();
        }
    }
}

void EveMarketTab::receiveRegionOrders()
{
    auto *rep = dynamic_cast<QNetworkReply*>(sender());
    QJsonDocument doc = QJsonDocument::fromJson(rep->readAll());
    QSqlQuery q(EVE->db);
    q.exec("DELETE FROM MarketOrders");
    QJsonArray arr = doc.array();
    q.prepare("INSERT INTO MarketOrders (orderID,typeID,location_id,volume_total,volume_remain,volume_minimum,price,is_buy_order,duration,issued,sale_range) VALUES (:order_id, :typeID, :location_id, :volume_total, :volume_remain, :min_volume, :price, :is_buy_order, :duration, :issued, :range)");
    QVariantList order_ids, durations, is_buy_orders, issueds, location_ids, typeIDs, volume_remains, volume_totals, prices, ranges, min_volumes;

    for (int i = 0; i  != arr.size(); ++i)
    {
        QJsonValue val = arr.at(i);
        QJsonObject obj = val.toObject();
        QVariantHash obj_map = obj.toVariantHash();

        order_ids.append(obj_map.value("order_id"));
        durations.append(obj_map.value("duration"));
        is_buy_orders.append(obj_map.value("is_buy_order"));
        issueds.append(obj_map.value("issued").toDateTime());
        location_ids.append(obj_map.value("location_id"));
        typeIDs.append(obj_map.value("type_id"));
        volume_remains.append(obj_map.value("volume_remain"));
        volume_totals.append(obj_map.value("volume_total"));
        prices.append(obj_map.value("price"));
        ranges.append(obj_map.value("range"));
        min_volumes.append(obj_map.value("min_volume"));
    }

    q.bindValue(":order_id", order_ids);
    q.bindValue(":duration", durations);
    q.bindValue(":is_buy_order", is_buy_orders);
    q.bindValue(":issued", issueds);
    q.bindValue(":location_id", location_ids);
    q.bindValue(":typeID", typeIDs);
    q.bindValue(":volume_remain", volume_remains);
    q.bindValue(":volume_total", volume_totals);
    q.bindValue(":price", prices);
    q.bindValue(":range", ranges);
    q.bindValue(":min_volume", min_volumes);

    if(!q.execBatch())
    {
        qDebug() << q.lastError();
    }

    EVE->EveBuyOrders->select();
    EVE->EveSellOrders->select();

}

void EveMarketTab::reloadMarket()
{
    QNetworkReply *rep = ESI->getMarketsRegionOrdersType(current_region, current_type);
    connect(rep, &QNetworkReply::finished, this, &EveMarketTab::receiveRegionOrders);
}

void EveMarketTab::changeRegion(int idx)
{
    int regionID = EVE->EveRegions->record(idx).value(0).toInt();
    if(regionID==current_region) {
        return;
    }
    current_region = regionID;
    if(current_type && current_region) {
        reloadMarket();
    }
}

void EveMarketTab::reloadSystemList(int regionID) {

}

EveMarketTab::EveMarketTab(QWidget *parent) : QWidget(parent) {
    setObjectName("EveMarketTab");
    hmarket_splitter = new QHBoxLayout(this);
    setLayout(hmarket_splitter);
    market_tree_vlayout = new QVBoxLayout();
    market_orders_vlayout = new QVBoxLayout();
    hmarket_splitter->addLayout(market_tree_vlayout);
    hmarket_splitter->addLayout(market_orders_vlayout);

    // Instantiate data structures!
    filter_markettree = new EveMarketTreeFilter(this);
    filter_markettree->setSourceModel(EVE->market_model_tree);

    // Instantiate search bar.
    completer_market = new QCompleter(EVE->market_model_flat, this);
    completer_market->setCaseSensitivity(Qt::CaseInsensitive);
    completer_market->setCompletionMode(QCompleter::PopupCompletion);
    search = new QLineEdit(this);
    search->setPlaceholderText("Search Here...");
    search->setMaximumWidth(300);
    search->setMinimumWidth(300);
    search->setCompleter(completer_market);


    // Instantiate location selection boxes.
    hlocation_layout = new QHBoxLayout();

    region_choice = new QComboBox(this);
    region_choice->setModel(EVE->EveRegions);
    region_choice->setModelColumn(1);

    system_choice = new QComboBox(this);
    station_choice = new QComboBox(this);

    hlocation_layout->addWidget(region_choice);
    hlocation_layout->addWidget(system_choice);
    hlocation_layout->addWidget(station_choice);

    market_tree_vlayout->addWidget(search);
    market_tree_vlayout->addLayout(hlocation_layout);

    // Instantiate market tree.
    view_markettree = new QTreeView(this);
    view_markettree->setModel(filter_markettree);
    view_markettree->setHeaderHidden(true);
    view_markettree->setMaximumWidth(300);
    view_markettree->setMinimumWidth(300);
    view_markettree->setMinimumHeight(600);
    market_tree_vlayout->addWidget(view_markettree);

    // Instantiate description box!
    mainbox = new QTextEdit(this);
    market_orders_vlayout->addWidget(mainbox);

    EveMarketDelegate *del = new EveMarketDelegate(this);

    view_sellorders = new QTableView(this);
    view_sellorders->setMinimumWidth(800);

    view_sellorders->setModel(EVE->EveSellOrders);
    view_sellorders->setItemDelegate(del);
    view_sellorders->setSortingEnabled(true);
    view_sellorders->setShowGrid(false);
    view_sellorders->hideColumn(4);
    view_sellorders->setColumnWidth(0,120);
    view_sellorders->setColumnWidth(1,100);
    view_sellorders->setColumnWidth(2,300);
    view_sellorders->setColumnWidth(3,150);
    view_sellorders->verticalHeader()->setVisible(false);
    view_sellorders->show();

    view_buyorders = new QTableView(this);
    view_buyorders->setModel(EVE->EveBuyOrders);
    view_buyorders->setItemDelegate(del);
    view_buyorders->setSortingEnabled(true);
    view_buyorders->setShowGrid(false);
    view_buyorders->hideColumn(4);
    view_buyorders->setColumnWidth(0,120);
    view_buyorders->setColumnWidth(1,100);
    view_buyorders->setColumnWidth(2,300);
    view_buyorders->setColumnWidth(3,150);
    view_buyorders->verticalHeader()->setVisible(false);
    view_buyorders->show();

    market_orders_vlayout->addWidget(view_sellorders);
    market_orders_vlayout->addWidget(view_buyorders);

    // Setup Signals!
    connect(view_markettree, &QTreeView::clicked, this, &EveMarketTab::displayItem);
    connect(search, &QLineEdit::textChanged, this, &EveMarketTab::filterMarket);
    connect(region_choice, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [=](int index){ changeRegion(index); });

    // Final steps!
    region_choice->setCurrentText("The Forge");

}
