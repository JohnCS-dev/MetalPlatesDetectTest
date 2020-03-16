#include "testmetaldetectwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestMetalDetectWindow w;
    w.show();

    return a.exec();
}
