#pragma once
#ifndef QTCIRCULARAPP_H
#define QTCIRCULARAPP_H
#include <QtWidgets/QWidget>
#include "ui_qtcircularapp.h"
#include <QLibrary>
#include <filesystem>
#include <cmath>

typedef int* (*CircusFunc)(int*, int, int, const int*);

class QtCircularApp : public QWidget
{
    Q_OBJECT

public:
	explicit QtCircularApp(QWidget* parent = nullptr);
    ~QtCircularApp();
	void calculate_optimal_sizes(int totalSize, int numThreads, std::vector<int>& sizes);
    QImage padImage(const QImage& image);
    
private:
    Ui::QtCircularAppClass ui;
    QLibrary library;
	std::filesystem::path imagePath;
    QImage chosenImage;
    CircusFunc filterFunc{};
	int chosenLib{ 0 };
    int threadsToUse{ 4 };
private slots:
    void onLibChooseButton_clicked();
	void onPathChooseButton_clicked();
	void onThreadChooseButton_clicked();
	void onApplyFilterButton_clicked();
};

#endif // QTCIRCULARAPP_H
