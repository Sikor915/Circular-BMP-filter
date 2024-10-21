#pragma once

#include <QtWidgets/QWidget>
#include "ui_qtcircularapp.h"
#include <iostream>
#include <windows.h>

class QtCircularApp : public QWidget
{
    Q_OBJECT

public:
    QtCircularApp(QWidget *parent = nullptr);
    ~QtCircularApp();

private:
    Ui::QtCircularAppClass ui;
};
