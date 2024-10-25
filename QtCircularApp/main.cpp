#include "stdafx.h"
#include "qtcircularapp.h"
#include <QtWidgets/QApplication>
#include <stdio.h>
#include <tchar.h>
#include "Circus.h"
#include <thread>

void calculate_optimal_sizes(std::vector<uchar>& b, int y, std::vector<int>& z)
{
	int x = b.size()/4;
	int c = x / y;
	float reszta_z_c = x % y;
	for (int i = 0; i < y; ++i)
	{
		std::cout << c << "  " << reszta_z_c << std::endl;
		z.push_back(c + (reszta_z_c > 0 ? 1 : 0));
		if (reszta_z_c > 0)
			reszta_z_c--;
	}
}

int main(int argc, char *argv[])
{
	std::cout << "Prosze, podaj ktora biblioteke chcesz wybrac: " << std::endl;
	std::cout << "1. Circular_DLL.dll (ASM)" << std::endl;
	std::cout << "2. Circus.dll (C++)" << std::endl;
	std::string choice;
	std::cin >> choice;

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

	}
	else
	{
		std::cout << "Niepoprawny wybor" << std::endl;
		return 1;
	}

	choice = "";
	std::string file_path;
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

			}
			else
			{
				std::cout << "Niepoprawny wybor" << std::endl;
			}
		}
		else if(choice == "3")
		{
			if (hLibASM != NULL)
			{

				typedef uchar(*MyFunctionType3)(uchar);
				auto CompressionFuncReal = (MyFunctionType3)GetProcAddress(hLibASM, "CompressionFuncReal");

				
				if (CompressionFuncReal == NULL)
				{
					std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
					break;
				}

				std::vector<std::thread> threads;

				QImage img("C:/Users/sikor/OneDrive/Desktop/Szkola/img.bmp"); //To dzia�a i wczytuje obraz
				if (img.isNull())
				{
					std::cout << "Image is null" << std::endl;
					break;
				}
				
				uchar* data = img.bits();
				
				int bytesPerLine = img.bytesPerLine();
				int height = img.height();
				int segmentHeight = height / threadsToUse;
				int width = img.width();
				
				std::cout << "Bytes per line: " << bytesPerLine << std::endl;
				std::cout << "Height: " << height << std::endl;
				std::cout << "Width: " << width << std::endl;

				QByteArray ba;
				QBuffer buffer(&ba);
				buffer.open(QIODevice::WriteOnly);
				img.save(&buffer, "BMP");

				std::vector<uchar> vec(ba.begin(), ba.end());
				std::vector<uchar> subBMP;
				std::vector<int> sizes;
				int threshold = 3;
				std::cout << vec.size() % threadsToUse << std::endl;

				std::vector<uchar>::iterator bmpIt = vec.begin();
				std::vector<int>::iterator sizesIt = sizes.begin();

				calculate_optimal_sizes(vec, threadsToUse, sizes);

				// Ogolnie to vector przechowuje dane o wszystkich kanalach, wiec musze przeskakiwac co 4
				// by leciec po kolei pikselami.
				// Dziwnie wyglada fakt ze rozmiar "vec" nie dzieli sie przez 4 i nie wiem do konca co z tym zrobic


				/*TODO:
				 * 1. Sprobowac przekonwertowac "vec" na vector int�w (dla ulatwienia)
				 * 2. Zrobic petle wywolujaca funkcje asm
				 * 3. Cos zrobic w funkcji asm
				 * 4. TESTING
				 */
				for (int j = 0; j < threadsToUse; j++)
				{
					std::thread t1([&vec, threshold, bmpIt, sizesIt, sizes, j]()
					{
							for (auto i = bmpIt; i < sizes[j] + sizesIt; i++)
							{

							}
					}
					);
				}

				/*
				//std::thread t1([&data, &bytesPerLine, &height, &width, &CompressionFuncReal]()
				//	{

				//		//P�tla musi by� w assemblerze, tu tylko przekaza� wska�nik na dane

				//		for (int y = 0; y < height; y++)
				//		{
				//			for (int x = 0; x < width; x++)
				//			{
				//				uchar* pixel = data + y * bytesPerLine + x * 4;

				//				//std::cout << "Pixel (BGR): " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;

				//				pixel[0] = CompressionFuncReal(pixel[0]);  // Kana� niebieski
				//				pixel[1] = CompressionFuncReal(pixel[1]);  // Kana� zielony
				//				pixel[2] = CompressionFuncReal(pixel[2]);  // Kana� czerwony

				//				//std::cout << "Pixel after \"Compression\" (BGR): " << (int)pixel2[0] << " " << (int)pixel2[1] << " " << (int)pixel2[2] << std::endl;
				//			}
				//		}
				//	});

				//for (int i = 0; i < threadsToUse; i++)
				//{
				//	int startY = i * segmentHeight;
				//	int endY = (i == threadsToUse - 1) ? height : (i + 1) * segmentHeight;

				//	threads.push_back(std::thread([&, startY, endY]()
				//		{
				//			//uchar* startPixel = data + startY * bytesPerLine;
				//			//ProcessImageSegment(startPixel, endY - startY, bytesPerLine, CompressionFuncReal);
				//		}));

				//}
				//
				//std::cout << "Czekanie na threada..." << std::endl;

				////t1.join();

				//for (auto & a : threads)
				//{
				//	a.join();
				//}
				*/
				img.save("C:/Users/sikor/OneDrive/Desktop/Szkola/img2.bmp");

				std::cout << "Zapisano obraz pod podana sciezka, pod nazwa img2.bmp" << std::endl;
			}
			else if(hDLLCPP != NULL)
			{
				//load cpp compression
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
