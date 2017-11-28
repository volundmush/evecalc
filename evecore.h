#ifndef EVECORE_H
#define EVECORE_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QCompleter>
#include <QTabWidget>

#include "evedatabase.h"
#include "esibase.h"
#include "evemarket.h"
#include "eveblueprint.h"

class EveCoreTab : public QTabWidget
{
    Q_OBJECT
public:
    EveCoreTab(QWidget *parent = Q_NULLPTR);
    EveMarketTab *tab_market;
    EveBlueprintTab *tab_blueprint;
};

class EveCore : public QMainWindow
{
    Q_OBJECT

public:
    EveCore();

private:
    QWidget *tab_center;


};

#endif // EVECORE_H
