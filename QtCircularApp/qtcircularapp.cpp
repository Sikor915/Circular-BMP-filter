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

QImage QtCircularApp::crop(const QImage& image)
{
	int currentWidth = image.width();
	int currentHeight = image.height();

	int newWidth = (currentWidth / 4) * 4;

	if (newWidth == currentWidth) {
		return image; 
	}

	// Define the cropping rectangle
	QRect cropRect(0, 0, newWidth, currentHeight);

	// Crop the image using the defined rectangle
	return image.copy(cropRect);
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
		QString libPath;
		if (choice == "ASM Library")
		{
			qDebug() << "ASM Library selected.";
			libPath = "..\\x64\\Debug\\Circular_DLL.dll";
			chosenLib = 1;
			ui.chosenLibLabel->setText("Chosen library: ASM");
		}
		else if (choice == "C++ Library")
		{
			qDebug() << "C++ Library selected.";
			libPath = "..\\x64\\Debug\\Circus.dll";
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
		chosenImage = crop(QImage(filePath));

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
	//V3
	/*
	if (chosenImage.isNull() || !filterFunc)
	{
		QMessageBox::warning(this, "Error", "Please load an image and select a library");
		return;
	}

	const int padding = 2;
	int bytesPerLine = chosenImage.bytesPerLine();
	int heightOG = chosenImage.height();
	int segmentHeightOG = heightOG / threadsToUse;
	int widthOG = chosenImage.width();
	QImage processedImage = chosenImage;

	qDebug() << "Height: " << heightOG << "\nWidth: " << widthOG << "\nBytes per line: " << bytesPerLine << "\nSegment height: " << segmentHeightOG << "\n";

	auto byteDataPointer = processedImage.bits();
	QRgb* rgbData = (QRgb*)byteDataPointer;
	std::vector<int> processedImageRGBData;

	qDebug() << "RGB data size: " << widthOG * heightOG << "\n";

	for (int i = 0; i < heightOG; i++)
	{
		for (int j = 0; j < widthOG; j++)
		{
			processedImageRGBData.push_back(rgbData[i * widthOG + j]);
		}
	}

	std::vector<std::thread> threads;
	std::vector<int> sizes;
	calculate_optimal_sizes(processedImageRGBData.size(), threadsToUse, sizes);

	auto startTime = std::chrono::high_resolution_clock::now();

	int currentIdx = 0;
	for (int j = 0; j < threadsToUse; j++)
	{
		int segmentSize = sizes[j];

		threads.emplace_back([this, &processedImageRGBData, currentIdx, segmentSize, widthOG, segmentHeightOG]()
			{
				//              RCX								 RDX		  R8           R9
				filterFunc(&processedImageRGBData[currentIdx], segmentSize, widthOG, segmentHeightOG);
			});

		currentIdx += segmentSize;

	}

	for (auto& t : threads)
	{
		t.join();
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	//HERE LIES THE DURATION OF THE ASM ALGORITHM
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

	QMessageBox::information(this, "Success", "Filter applied successfully. " + message);

	if (chosenLib == 1) processedImage.save((imagePath.remove_filename().string() + "outputASM.bmp").c_str());
	else if (chosenLib == 2) processedImage.save((imagePath.remove_filename().string() + "outputCPP.bmp").c_str());

	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addPixmap(QPixmap::fromImage(processedImage).scaled(ui.filteredImg->size(), Qt::KeepAspectRatio));
	ui.filteredImg->setScene(scene);
	*/
	//V4
	if (chosenImage.isNull() || !filterFunc)
	{
		QMessageBox::warning(this, "Error", "Please load an image and select a library");
		return;
	}

	const int padding = 2; // Add 2-pixel black border
	int widthOG = chosenImage.width();
	int heightOG = chosenImage.height();
	int paddedWidth = widthOG + 2 * padding;
	int paddedHeight = heightOG + 2 * padding;

	// Create a padded image
	QImage paddedImage(paddedWidth, paddedHeight, chosenImage.format());
	paddedImage.fill(Qt::black);

	// Draw the original image onto the padded image
	QPainter painter(&paddedImage);
	painter.drawImage(QPoint(padding, padding), chosenImage);
	painter.end();

	int bytesPerLine = paddedImage.bytesPerLine();
	QImage processedImage = paddedImage;

	qDebug() << "Original Height: " << heightOG << "\nOriginal Width: " << widthOG
		<< "\nPadded Height: " << paddedHeight << "\nPadded Width: " << paddedWidth
		<< "\nBytes per line: " << bytesPerLine;

	auto byteDataPointer = processedImage.bits();
	QRgb* rgbData = reinterpret_cast<QRgb*>(byteDataPointer);
	std::vector<int> processedImageRGBData;

	qDebug() << "RGB data size (padded): " << paddedWidth * paddedHeight << "\n";

	for (int i = 0; i < paddedHeight; i++)
	{
		for (int j = 0; j < paddedWidth; j++)
		{
			processedImageRGBData.push_back(rgbData[i * paddedWidth + j]);
		}
	}

	std::vector<std::thread> threads;
	std::vector<int> sizes;

	calculate_optimal_sizes(processedImageRGBData.size(), threadsToUse, sizes);


	auto startTime = std::chrono::high_resolution_clock::now();

	int currentIdx = 0;
	for (int j = 0; j < threadsToUse; j++)
	{
		int segmentSize = sizes[j];

		threads.emplace_back([this, &processedImageRGBData, currentIdx, segmentSize, paddedWidth, heightOG]()
			{
				//              RCX								 RDX		  R8           R9
				filterFunc(&processedImageRGBData[currentIdx], segmentSize, paddedWidth, heightOG);
			});

		currentIdx += segmentSize;
	}

	for (auto& t : threads)
	{
		t.join();
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = endTime - startTime;

	// Remove padding and rebuild the original-size image
	QImage finalImage(widthOG, heightOG, paddedImage.format());
	int index = 0;
	for (int i = padding; i < padding + heightOG; ++i)
	{
		for (int j = padding; j < padding + widthOG; ++j)
		{
			int paddedIdx = i * paddedWidth + j;
			finalImage.setPixelColor(index % widthOG, index / widthOG, QColor::fromRgb(processedImageRGBData[paddedIdx]));
			++index;
		}
	}

	// Show elapsed time
	double rounded = std::round(duration.count() * 10000.0) / 10000.0;
	QString message = QString("Elapsed time: %1 seconds").arg(rounded, 0, 'f', 4);

	QMessageBox::information(this, "Success", "Filter applied successfully. " + message);

	// Save the final processed image
	if (chosenLib == 1) finalImage.save((imagePath.remove_filename().string() + "outputASM.bmp").c_str());
	else if (chosenLib == 2) finalImage.save((imagePath.remove_filename().string() + "outputCPP.bmp").c_str());

	// Display the final image in the UI
	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addPixmap(QPixmap::fromImage(finalImage).scaled(ui.filteredImg->size(), Qt::KeepAspectRatio));
	ui.filteredImg->setScene(scene);
}
