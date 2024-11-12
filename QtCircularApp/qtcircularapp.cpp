#include "stdafx.h"
#include "qtcircularapp.h"
#include <QInputDialog>
#include <QDebug>


void QtCircularApp::calculate_optimal_sizes(int totalSize, int numThreads, std::vector<int>& sizes)
{
    int baseSize = totalSize / numThreads;
    int remainder = totalSize % numThreads;

    for (int i = 0; i < numThreads; ++i)
    {
        sizes.push_back(baseSize + (remainder > 0 ? 1 : 0));
        if (remainder > 0) remainder--;
    }
}

QtCircularApp::QtCircularApp(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.threadNumberLabel->setText("Number of threads: " + QString::number(threadsToUse));
    connect(ui.libChooseButton, &QPushButton::clicked, this, &QtCircularApp::onLibChooseButton_clicked);
	connect(ui.pathChooseButton, &QPushButton::clicked, this, &QtCircularApp::onPathChooseButton_clicked);
	connect(ui.threadChooseButton, &QPushButton::clicked, this, &QtCircularApp::onThreadChooseButton_clicked);
    connect(ui.filterApplyButton, &QPushButton::clicked, this, &QtCircularApp::onApplyFilterButton_clicked);
}

QtCircularApp::~QtCircularApp()
{

}

void QtCircularApp::onLibChooseButton_clicked() {
    QStringList libraries = { "ASM Library", "C++ Library" };
    bool ok;
    QString choice = QInputDialog::getItem(this, "Choose Library", "Library:", libraries, 0, false, &ok);

    if (ok && !choice.isEmpty())
    {
        QString libPath;
        if (choice == "ASM Library")
        {
            qDebug() << "ASM Library selected.";
            libPath = "C:\\Users\\sikor\\source\\repos\\QtCircularSOL\\x64\\Debug\\Circular_DLL.dll";
            chosenLib = 1;
            ui.chosenLibLabel->setText("Chosen library: ASM");
        }
        else if (choice == "C++ Library")
        {
            qDebug() << "C++ Library selected.";
            libPath = "C:\\Users\\sikor\\source\\repos\\QtCircularSOL\\x64\\Debug\\Circus.dll";
			chosenLib = 2;
            ui.chosenLibLabel->setText("Chosen library: CPP");
        }

        library.setFileName(libPath);
        if (!library.load())
        {
            qDebug() << "Could not load library." << library.errorString();
            return;
        }
        else
        {
            qDebug() << "Library loaded successfully.";
        }

        filterFunc = (CircusFunc)library.resolve("CompressionFuncCircus");
        if (filterFunc)
        {
            qDebug() << "Function resolved successfully.";
        }
        else
        {
            qDebug() << "Could not resolve function.";
        }
    }
}

void QtCircularApp::onPathChooseButton_clicked() 
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Choose Image"), "", tr("Image Files (*.bmp)"));
    if (!filePath.isEmpty()) 
    {
        chosenImage = QImage(filePath);
		imagePath = filePath.toStdString();
        if (chosenImage.isNull()) 
        {
            QMessageBox::critical(this, "Error", "Failed to load image");
            return;
        }
		QGraphicsScene* scene = new QGraphicsScene(this);
        scene->addPixmap(QPixmap::fromImage(chosenImage).scaled(ui.chosenImg->size(), Qt::KeepAspectRatio));
		ui.chosenImg->setScene(scene);
    }
}

void QtCircularApp::onThreadChooseButton_clicked()
{
    bool ok;
    int threads = QInputDialog::getInt(this, tr("Choose Threads"), tr("Number of Threads:"), 4, 1, 64, 1, &ok);
    if (ok) 
    {
        threadsToUse = threads;
        ui.threadNumberLabel->setText("Number of threads: " + QString::number(threadsToUse));
    }
}

void QtCircularApp::onApplyFilterButton_clicked()
{
    if (chosenImage.isNull() || !filterFunc) 
    {
        QMessageBox::warning(this, "Error", "Please load an image and select a library");
        return;
    }

    int bytesPerLine = chosenImage.bytesPerLine();
    int height = chosenImage.height();
    int segmentHeight = height / threadsToUse;
    int width = chosenImage.width();
    QImage processedImage = chosenImage;

    auto byteDataPointer = processedImage.bits();
    QRgb* rgbData = (QRgb*)byteDataPointer;
    std::vector<int> rgbDataSegments;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            rgbDataSegments.push_back(rgbData[i * width + j]);
        }
    }

    std::vector<std::thread> threads;
    std::vector<int> sizes;
    calculate_optimal_sizes(rgbDataSegments.size(), threadsToUse, sizes);

    if (chosenLib == 1)
	{
		auto startTime = std::chrono::high_resolution_clock::now();

		int currentIdx = 0;
		for (int j = 0; j < threadsToUse; j++)
		{
			int segmentSize = sizes[j];

			threads.emplace_back([this, &rgbDataSegments, currentIdx, segmentSize, width, segmentHeight]()
				{
					filterFunc(&rgbDataSegments[currentIdx], segmentSize, width, segmentHeight);
				});

			currentIdx += segmentSize;

		}

		for (auto& t : threads)
		{
			t.join();
		}

		auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = endTime - startTime;

        int index = 0;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                rgbData[i * width + j] = rgbDataSegments[i * width + j];
            }
        }

        QMessageBox::information(this, "Success", "Filter applied successfully");

        processedImage.save((imagePath.remove_filename().string() + "outputASM.bmp").c_str());

    }
	else if (chosenLib == 2)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        int currentIdx = 0;
        for (int j = 0; j < threadsToUse; j++)
        {
            int segmentSize = sizes[j];

            threads.emplace_back([this, &rgbDataSegments, currentIdx, segmentSize, width, segmentHeight]()
                {
                    filterFunc(&rgbDataSegments[currentIdx], segmentSize, width, segmentHeight);
                });

            currentIdx += segmentSize;

        }

        for (auto& t : threads)
        {
            t.join();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = endTime - startTime;



        int index = 0;
        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                rgbData[i * width + j] = rgbDataSegments[i * width + j];
            }
        }

        QMessageBox::information(this, "Success", "Filter applied successfully");

        processedImage.save((imagePath.remove_filename().string() + "outputCPP.bmp").c_str());

    }

    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->addPixmap(QPixmap::fromImage(processedImage).scaled(ui.filteredImg->size(), Qt::KeepAspectRatio));
    ui.filteredImg->setScene(scene);
}
