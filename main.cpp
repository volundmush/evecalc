#include "evecore.h"
#include "esibase.h"
#include "evedatabase.h"
#include <QApplication>
#include <QDir>
#include <QDebug>

EsiBase *ESI;
EveDB *EVE;

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    QDir::setCurrent(QDir::homePath() + "/evecalc");
    ESI = new EsiBase();
    EVE = new EveDB();
    EveCore core;
    core.show();

    return a.exec();
}
