#include "stdafx.h"
#include "qtcircularapp.h"
#include <QtWidgets/QApplication>
#include <stdio.h>
#include <tchar.h>
#include "Circus.h"
#include <thread>
#include <chrono>
#include <numeric>

void calculate_optimal_sizes(int totalSize, int numThreads, std::vector<int>& sizes) 
{
	int baseSize = totalSize / numThreads;
	int remainder = totalSize % numThreads;

	for (int i = 0; i < numThreads; ++i) 
	{
		sizes.push_back(baseSize + (remainder > 0 ? 1 : 0));
		if (remainder > 0) remainder--;
	}
}

int main(int argc, char *argv[])
{
	std::cout << "Prosze, podaj ktora biblioteke chcesz wybrac: " << std::endl;
	std::cout << "1. Circular_DLL.dll (ASM)" << std::endl;
	std::cout << "2. Circus.dll (C++)" << std::endl;
	std::string choice;
	std::cin >> choice;

	int choiceInt = 0;
	HMODULE hLibASM{};
	HINSTANCE hDLLCPP{};

	if (choice == "1")
	{
		std::cout << "Wybrano Circular_DLL.dll (ASM)" << std::endl;

		hLibASM = LoadLibrary(L"Circular_DLL.dll");
		if (hLibASM == NULL)
		{
			printf("Nie mozna zaladowac biblioteki Circular_DLL.dll\n");
			return 1;
		}
		choiceInt = 1;

	}
	else if (choice == "2")
	{
		std::cout << "Wybrano Circus.dll (C++)" << std::endl;

		hDLLCPP = LoadLibrary(L"Circus.dll");
		if (hDLLCPP == NULL)
		{
			printf("Nie mozna zaladowac biblioteki Circus.dll\n");
			return 1;
		}
		choiceInt = 2;

	}
	else
	{
		std::cout << "Niepoprawny wybor" << std::endl;
		return 1;
	}

	choice = "";
	std::string file_path = "";
	int threadsToUse{4};

	while (choice != "5")
	{
		std::cout << "Lorem Ipsum, co chcesz zrobic" << std::endl;
		std::cout << "1. Podaj sciezke do obrazu" << std::endl;
		std::cout << "2. Wybierz DLL'ke" << std::endl;
		std::cout << "3. Wykonaj kompresje" << std::endl;
		std::cout << "4. Podaj ilosc watkow do uzycia. Aktualnie: " << threadsToUse << std::endl;
		std::cout << "5. Wyjdz" << std::endl;
		std::cin >> choice;

		if (choice == "1")
		{
			std::cout << "Aktualna sciezka do pliku: " << file_path << std::endl;
			std::cout << "Prosze o podanie sciezki do obrazu: ";
			std::cin >> file_path;
		}
		else if(choice == "2")
		{
			std::string dllChoice;
			std::cout << "Prosze, podaj ktora biblioteke chcesz wybrac: " << std::endl;
			std::cout << "1. Circular_DLL.dll (ASM)" << std::endl;
			std::cout << "2. Circus.dll (C++)" << std::endl;
			std::cin >> dllChoice;

			if (dllChoice == "1")
			{
				std::cout << "Wybrano Circular_DLL.dll (ASM)" << std::endl;

				if (hDLLCPP != NULL)
				{
					FreeLibrary(hDLLCPP);
				}

				hLibASM = LoadLibrary(L"Circular_DLL.dll");
				if (hLibASM == NULL)
				{
					printf("Nie mozna zaladowac biblioteki Circular_DLL.dll\n");
					return 1;
				}
				choiceInt = 1;

			}
			else if(dllChoice == "2")
			{
				std::cout << "Wybrano Circus.dll (C++)" << std::endl;

				if (hLibASM != NULL)
				{
					FreeLibrary(hLibASM);
				}

				hDLLCPP = LoadLibrary(L"Circus.dll");
				if (hDLLCPP == NULL)
				{
					printf("Nie mozna zaladowac biblioteki Circus.dll\n");
					return 1;
				}
				choiceInt = 2;

			}
			else
			{
				std::cout << "Niepoprawny wybor" << std::endl;
			}
		}
		else if(choice == "3")
		{
			if (hLibASM != NULL and choiceInt == 1)
			{
				typedef int* (*MyFunctionType3)(QRgb*, int, int);
				auto CompressionFuncReal = (MyFunctionType3)GetProcAddress(hLibASM, "CompressionFuncReal");


				if (CompressionFuncReal == NULL)
				{
					std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL ASM." << std::endl;
					break;
				}

				QImage img(file_path.c_str());
				if (img.isNull())
				{
					std::cout << "Image is null" << std::endl;
				}
				else
				{
					int bytesPerLine = img.bytesPerLine();
					int height = img.height();
					int segmentHeight = height / threadsToUse;
					int width = img.width();

					std::cout << "Bytes per line: " << bytesPerLine << std::endl;
					std::cout << "Height: " << height << std::endl;
					std::cout << "Segment height: " << segmentHeight << std::endl;
					std::cout << "Width: " << width << std::endl;


					auto byteDataPointer = img.bits();
					QRgb* rgbData = (QRgb*)byteDataPointer;
					std::vector<QRgb> rgbDataSegments;
				
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

					auto startTime = std::chrono::high_resolution_clock::now();

					int currentIdx = 0;
					for (int j = 0; j < threadsToUse; j++)
					{
						int segmentSize = sizes[j];

						threads.emplace_back([&rgbDataSegments, currentIdx, segmentSize, width, segmentHeight, CompressionFuncReal]()
							{
								CompressionFuncReal(&rgbDataSegments[currentIdx], segmentSize, segmentHeight);
							});

						currentIdx += segmentSize;

					}

					for (auto& t : threads)
					{
						t.join();
					}

					auto endTime = std::chrono::high_resolution_clock::now();

					int index = 0;
					for (int i = 0; i < height; ++i)
					{
						for (int j = 0; j < width; ++j)
						{
							rgbData[i * width + j] = rgbDataSegments[i * width + j];
						}
					}

					std::filesystem::path savePath(file_path);
					savePath = savePath.remove_filename();
					std::string savePathStr = savePath.string() + "outputASM.bmp";

					img.save(savePathStr.c_str());

					std::chrono::duration<double> duration = endTime - startTime;
					std::cout << "Filtering completed in " << duration.count() << " seconds.\n";

					std::cout << "Zapisano obraz pod podana sciezka, pod nazwa outputASM.bmp" << std::endl;
				}
			
				
			}
			else if(hDLLCPP != NULL and choiceInt == 2)
			{
				typedef int* (*MyFunctionType3)(int*, int, int, int);
				auto CompressionFuncCircus = (MyFunctionType3)GetProcAddress(hDLLCPP, "CompressionFuncCircus");

				if (CompressionFuncCircus == NULL)
				{
					std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL CPP." << std::endl;
					break;
				}

				QImage img(file_path.c_str());
				if (img.isNull())
				{
					std::cout << "Image is null" << std::endl;
					
				}
				else
				{
					int bytesPerLine = img.bytesPerLine();
					int height = img.height();
					int segmentHeight = height / threadsToUse;
					int width = img.width();

					std::cout << "Image format: " << img.format() << std::endl; // 4
					std::cout << "Bytes per line: " << bytesPerLine << std::endl; // 9600
					std::cout << "Height: " << height << std::endl; // 1600
					std::cout << "Segment height: " << segmentHeight << std::endl; // 400
					std::cout << "Width: " << width << std::endl; // 2400

					auto byteDataPointer = img.bits();
					// na tym powinienem pracowac ale nie moge bo zewnetrzne dll'ki nie maja dostepu do zmiennych Qt
					QRgb* rgbData = (QRgb*)byteDataPointer;
					std::vector<int> intBMP;

					for (int i = 0; i < height; i++)
					{
						for (int j = 0; j < width; j++)
						{
							QRgb pixel = rgbData[i * width + j];
							intBMP.push_back(qRed(pixel));
							intBMP.push_back(qGreen(pixel));
							intBMP.push_back(qBlue(pixel));
							intBMP.push_back(qAlpha(pixel)); // Should be 255 everywhere
						}
					}

					std::vector<std::thread> threads;
					std::vector<int> sizes;

					calculate_optimal_sizes(intBMP.size(), threadsToUse, sizes);

					int currentIdx = 0;
					auto startTime = std::chrono::high_resolution_clock::now();

					for (int j = 0; j < threadsToUse; j++)
					{
						int segmentSize = sizes[j];

						threads.emplace_back([&intBMP, currentIdx, segmentSize, width, segmentHeight, CompressionFuncCircus]()
							{
								CompressionFuncCircus(&intBMP[currentIdx], segmentSize, width, segmentHeight);
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
							int r = intBMP[index++];
							int g = intBMP[index++];
							int b = intBMP[index++];
							int a = intBMP[index++];

							rgbData[i * width + j] = qRgba(r, g, b, a);
						}
					}

					std::filesystem::path savePath(file_path);
					savePath = savePath.remove_filename();
					std::string savePathStr = savePath.string() + "outputCPP.bmp";

					img.save(savePathStr.c_str());

					
					std::cout << "Filtering completed in " << duration.count() << " seconds.\n";

					std::cout << "Zapisano obraz pod podana sciezka, pod nazwa outputCPP.bmp" << std::endl;

				}

				
			}
			else
			{
				std::cout << "Nie wybrano biblioteki" << std::endl;
			}
		}
		else if (choice == "4")
		{
			std::cout << "Podaj liczbe watkow: ";
			std::string threadCount;
			bool isCountDigits = true;
			std::cin >> threadCount;
			std::string::const_iterator it = threadCount.begin();
			while (it != threadCount.end() && isCountDigits)
			{
				if (!std::isdigit(*it))
				{
					isCountDigits = false;
				}
				it++;
			}
			if (!isCountDigits)
			{
				std::cout << "Zostaly podane inne znaki niz liczby!" << std::endl;
			}
			else
			{
				threadsToUse = std::stoi(threadCount);
			}
		}
		else if(choice == "5")
		{
			std::cout << "Koniec programu" << std::endl;
		}
		else
		{
			std::cout << "Niepoprawny wybor" << std::endl;
		}
	}

	return 0;
    QApplication a(argc, argv);
    QtCircularApp w;
    w.show();
    return a.exec();
}
