#include "stdafx.h"
#include "qtcircularapp.h"
#include <QtWidgets/QApplication>
#include <qinputdialog.h>
#include <stdio.h>
#include <tchar.h>
#include "Circus.h"
#include <thread>
#include <chrono>
#include <numeric>
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtCircularApp w;
    w.show();
    return a.exec();
}
