#include "stdafx.h"
#include "qtcircularapp.h"
#include <QInputDialog>
#include <QDebug>

void QtCircularApp::calculate_optimal_heights(int imgHeight, int numThreads, std::vector<int>& heights)
{
	int baseHeight = imgHeight / numThreads;
	int remainder = imgHeight % numThreads;
	for (int i = 0; i < numThreads; ++i)
	{
		heights.push_back(baseHeight + (remainder > 0 ? 1 : 0));
		if (remainder > 0) remainder--;
	}
}

QImage QtCircularApp::padImage(const QImage& image)
{
	int width = image.width();
	int height = image.height();
	int newWidth = (width + 3) & ~3; // Round up to the nearest multiple of 4

	if (width == newWidth) 
	{
		return image;
	}

	QImage alignedImage(newWidth, height, image.format());

	// Copy pixel data row by row
	for (int row = 0; row < height; ++row) 
	{
		const uchar* srcLine = image.scanLine(row);
		uchar* destLine = alignedImage.scanLine(row);

		int bytesPerPixel = image.depth() / 8;
		memcpy(destLine, srcLine, width * bytesPerPixel); // Copy original row

		// Duplicate the last column into the new pixels
		const uchar* lastPixel = srcLine + (width - 1) * bytesPerPixel;
		for (int i = width; i < newWidth; ++i) 
		{
			memcpy(destLine + i * bytesPerPixel, lastPixel, bytesPerPixel);
		}
	}

	return alignedImage;
}

QtCircularApp::QtCircularApp(QWidget* parent)
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
		QString libPath, exeDir{ QCoreApplication::applicationDirPath() };
		if (choice == "ASM Library")
		{
			qDebug() << "ASM Library selected.";
			libPath = exeDir + "/Circular_DLL.dll";
			chosenLib = 1;
			ui.chosenLibLabel->setText("Chosen library: ASM");
		}
		else if (choice == "C++ Library")
		{
			qDebug() << "C++ Library selected.";
			libPath = exeDir + "/Circus.dll";
			chosenLib = 2;
			ui.chosenLibLabel->setText("Chosen library: CPP");
		}

		library.setFileName(libPath);
		if (!library.load())
		{
			qDebug() << "Could not load library." << library.errorString();
			QMessageBox::critical(this, "ERROR", "Could not load the library.");
			return;
		}
		else
		{
			qDebug() << "Library loaded successfully.";
			QMessageBox::information(this, "Success", "Loaded the library.");
		}

		filterFunc = (CircusFunc)library.resolve("CompressionFuncCircus");
		if (filterFunc)
		{
			qDebug() << "Function resolved successfully.";
			QMessageBox::information(this, "Success", "Loaded the filter function.");
		}
		else
		{
			qDebug() << "Could not resolve function.";
			QMessageBox::critical(this, "ERROR", "Could not load the function.");

		}
	}
}

void QtCircularApp::onPathChooseButton_clicked()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose Image"), "", tr("Image Files (*.bmp)"));
	if (!filePath.isEmpty())
	{
		chosenImage = padImage(QImage(filePath));

		imagePath = filePath.toStdString();
		if (chosenImage.isNull())
		{
			QMessageBox::critical(this, "Error", "Failed to load image");
			return;
		}
		QGraphicsScene* scene = new QGraphicsScene(this);
		scene->addPixmap(QPixmap::fromImage(chosenImage).scaled(ui.chosenImg->size(), Qt::KeepAspectRatio));
		ui.chosenImg->setScene(scene);
		QMessageBox::information(this, "Success", "Successfully loaded the image");
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
	//V3

	if (chosenImage.isNull() || !filterFunc)
	{
		QMessageBox::warning(this, "Error", "Please load an image and select a library");
		return;
	}

	int bytesPerLine = chosenImage.bytesPerLine(), heightOG = chosenImage.height(),
		segmentHeightOG = heightOG / threadsToUse, widthOG = chosenImage.width();

	QImage processedImage = chosenImage,
		innerPart = processedImage.copy(0, 2, widthOG, heightOG - 4);


	auto byteDataPointer{ processedImage.bits() }, innerDataPointer{ innerPart.bits() };
	QRgb* rgbData{ (QRgb*)byteDataPointer }, * innerData{ (QRgb*)innerDataPointer };
	std::vector<int> processedImageRGBData, innerDataVec, OGImageData;

	for (int i = 0; i < heightOG; i++)
	{
		for (int j = 0; j < widthOG; j++)
		{
			processedImageRGBData.push_back(rgbData[i * widthOG + j]);
		}
	}

	OGImageData = processedImageRGBData;

	qDebug() << "Height: " << heightOG <<
		"\nWidth: " << widthOG <<
		"\nBytes per line: " << bytesPerLine <<
		"\nSegment height: " << segmentHeightOG <<
		"\nCalculated RGB data size: " << widthOG * heightOG <<
		"\nVector size: " << processedImageRGBData.size() <<
		"\nAddress of OG Pixel Data: " << &OGImageData[(2 * widthOG) + 2] << 
		"\n";

	for (int i = 0; i < heightOG - 4; i++)
	{
		for (int j = 0; j < widthOG; j++)
		{
			innerDataVec.push_back(rgbData[i * widthOG + j]);
		}
	}

	std::vector<std::thread> threads;
	std::vector<int> sizes, heights;
	calculate_optimal_heights(heightOG, threadsToUse, heights);

	auto startTime = std::chrono::high_resolution_clock::now();

	int currentIdx = (2 * widthOG) + 2;
	for (int j = 0; j < threadsToUse; j++)
	{
		int rowsToProcess = heights[j];			                      // 1 after the last | last | 2 rows above | 2 pixels left = (widthOG - 2, heightOG - 2)
		const int* endPixelAddress = &OGImageData.at(OGImageData.size() - 1 - (2 * widthOG) - 2);
		threads.emplace_back([this, &processedImageRGBData, currentIdx, rowsToProcess, widthOG, endPixelAddress, &OGImageData]()
			{
				//              RCX								 RDX		  R8           R9			Stack
				filterFunc(&processedImageRGBData[currentIdx], rowsToProcess, widthOG, endPixelAddress, &OGImageData[currentIdx]);
			});
		currentIdx += rowsToProcess * widthOG;

	}

	for (auto& t : threads)
	{
		t.join();
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	//HERE LIES THE DURATION OF THE ALGORITHM
	std::chrono::duration<double> duration = endTime - startTime;

	//In seconds rounded
	double rounded = std::round(duration.count() * 10000.0) / 10000.0;

	int index = 0;
	for (int i = 0; i < heightOG; ++i)
	{
		for (int j = 0; j < widthOG; ++j)
		{
			rgbData[i * widthOG + j] = processedImageRGBData[i * widthOG + j];
		}
	}

	QString message = QString("Elapsed time: %1 seconds")
		.arg(rounded, 0, 'f', 4);

	QMessageBox::information(this, "Success", "Filter applied successfully. " + message + ". Processed image is in the same directory as provided.");

	if (chosenLib == 1) processedImage.save((imagePath.remove_filename().string() + "outputASM.bmp").c_str());
	else if (chosenLib == 2) processedImage.save((imagePath.remove_filename().string() + "outputCPP.bmp").c_str());

	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addPixmap(QPixmap::fromImage(processedImage).scaled(ui.filteredImg->size(), Qt::KeepAspectRatio));
	ui.filteredImg->setScene(scene);
	
}
