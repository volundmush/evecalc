#include "evecore.h"

EveCoreTab::EveCoreTab(QWidget *parent) : QTabWidget(parent)
{
    setObjectName("EveCoreTab");
    tab_market = new EveMarketTab(this);
    addTab(tab_market, "Market Browser");
    tab_blueprint = new EveBlueprintTab(this);
    addTab(tab_blueprint, "Blueprint Manager");
}

EveCore::EveCore()
{
    setObjectName("EveCore");
    tab_center = new EveCoreTab(this);
    setCentralWidget(tab_center);
}
