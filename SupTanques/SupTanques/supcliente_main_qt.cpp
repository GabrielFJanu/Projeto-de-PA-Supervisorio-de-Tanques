#include "supcliente_qt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  SupClienteQt w;
  w.show();
  return a.exec();
}
