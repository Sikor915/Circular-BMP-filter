#include "stdafx.h"
#include "qtcircularapp.h"


QtCircularApp::QtCircularApp(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    QPixmap pm("C:/Users/sikor/OneDrive/Desktop/Szkola/img.bmp");
	ui.imageLabel->setPixmap(pm);
    ui.imageLabel->setScaledContents(true);
}

QtCircularApp::~QtCircularApp()
{}

