#include "stdafx.h"
#include "qtcircularapp.h"
#include <QtWidgets/QApplication>
#include <stdio.h>
#include <tchar.h>
#include "Circus.h"
#include <thread>

int main(int argc, char *argv[])
{

//	int x = 3, y = 4, z = 0;
//
//
//	HMODULE hLib = LoadLibrary(L"Circular_DLL.dll");
//	if (hLib == NULL)
//	{
//		printf("Nie mozna zaladowac biblioteki Circular_DLL.dll\n");
//		return 1;
//	}
//
//	typedef int (*MyFunctionType1)(long long, long long);
//	auto CompressionFunc = (MyFunctionType1)GetProcAddress(hLib, "CompressionFunc");
//
//	typedef uchar (*MyFunctionType3)(uchar);
//	auto CompressionFuncReal = (MyFunctionType3)GetProcAddress(hLib, "CompressionFuncReal");
//
//	if (CompressionFunc == NULL)
//	{
//		std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
//		return 1;
//	}
//	if (CompressionFuncReal == NULL)
//	{
//		std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
//	}
//
//	z = CompressionFunc(x, y);		// wywo³anie procedury asemblerowej z biblioteki
//
//	std::cout << CompressionFunc(x, y) << std::endl;
//
//
///*
//	HINSTANCE hDLLCPP = LoadLibrary(L"Circus.dll");
//	if (hDLLCPP == NULL)
//	{
//		printf("Nie mozna zaladowac biblioteki Circus.dll\n");
//		return 1;
//	}
//
//	typedef void (*MyFunctionType2)(long long, long long);
//	auto Circus = (MyFunctionType2)GetProcAddress(hDLLCPP, "Circus");
//	auto CompressionFuncCircus = (MyFunctionType1)GetProcAddress(hDLLCPP, "CompressionFuncCircus");
//
//	if (Circus == NULL)
//	{
//		std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
//		return 1;
//	}
//
//	Circus(21, 37);
//	int xyz = CompressionFuncCircus(21, 37);
//
//*/
//
//	QImage img("C:/Users/sikor/OneDrive/Desktop/Szkola/img.bmp"); //To dzia³a i wczytuje obraz
//
//	if (img.isNull())
//	{
//		std::cout << "Image is null" << std::endl;
//	}
//
//	uchar* data = img.bits();
//
//	int bytesPerLine = img.bytesPerLine();
//	int height = img.height();
//	int width = img.width();
//
//	std::cout << "Bytes per line: " << bytesPerLine << std::endl;
//	std::cout << "Height: " << height << std::endl;
//	std::cout << "Width: " << width << std::endl;
//
//	for (int y = 0; y < height; y++)
//	{
//		for (int x = 0; x < width; x++)
//		{
//			uchar* pixel = data + y * bytesPerLine + x * 4;
//			uchar* pixel2 = data + y * bytesPerLine + x * 4;
//
//			std::cout << "Pixel (BGR): " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;
//
//			pixel2[0] = CompressionFuncReal(pixel[0]);  // Kana³ niebieski
//			pixel2[1] = CompressionFuncReal(pixel[1]);  // Kana³ zielony
//			pixel2[2] = CompressionFuncReal(pixel[2]);  // Kana³ czerwony
//
//			std::cout << "Pixel after \"Compression\" (BGR): " << (int)pixel2[0] << " " << (int)pixel2[1] << " " << (int)pixel2[2] << std::endl;
//		}
//	}

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

		typedef int (*MyFunctionType1)(long long, long long);
		auto CompressionFunc = (MyFunctionType1)GetProcAddress(hLibASM, "CompressionFunc");
		typedef uchar(*MyFunctionType3)(uchar);
		auto CompressionFuncReal = (MyFunctionType3)GetProcAddress(hLibASM, "CompressionFuncReal");

		//if (CompressionFunc == NULL)
		//{
		//	std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
		//	return 1;
		//}

		//int x = 3, y = 4;
		//std::cout << CompressionFunc(x, y) << std::endl;


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

		//typedef void (*MyFunctionType2)(long long, long long);
		//auto Circus = (MyFunctionType2)GetProcAddress(hDLLCPP, "Circus");

		//if (Circus == NULL)
		//{
		//	std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
		//	return 1;
		//}

		//Circus(21, 37);

	}
	else
	{
		std::cout << "Niepoprawny wybor" << std::endl;
		return 1;
	}

	choice = "";
	std::string file_path;

	while (choice != "4")
	{
		std::cout << "Lorem Ipsum, co chcesz zrobic" << std::endl;
		std::cout << "1. Podaj sciezke do obrazu " << std::endl;
		std::cout << "2. Wybierz DLL'ke" << std::endl;
		std::cout << "3. Wykonaj kompresje" << std::endl;
		std::cout << "4. Wyjdz" << std::endl;
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

				//typedef int (*MyFunctionType1)(long long, long long);
				//auto CompressionFunc = (MyFunctionType1)GetProcAddress(hLibASM, "CompressionFunc");
				//typedef uchar(*MyFunctionType3)(uchar);
				//auto CompressionFuncReal = (MyFunctionType3)GetProcAddress(hLibASM, "CompressionFuncReal");

				//if (CompressionFunc == NULL)
				//{
				//	std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
				//	return 1;
				//}

				//int x = 8, y = 12;
				//std::cout << CompressionFunc(x, y) << std::endl;

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

				/*typedef void (*MyFunctionType2)(long long, long long);
				auto Circus = (MyFunctionType2)GetProcAddress(hDLLCPP, "Circus");

				if (Circus == NULL)
				{
					std::cerr << "Nie udalo sie pobrac adresu funkcji z biblioteki DLL." << std::endl;
					return 1;
				}

				Circus(37, 21);*/

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

				QImage img("C:/Users/sikor/OneDrive/Desktop/Szkola/img.bmp"); //To dzia³a i wczytuje obraz
				if (img.isNull())
				{
					std::cout << "Image is null" << std::endl;
					break;
				}
				
				uchar* data = img.bits();
				
				int bytesPerLine = img.bytesPerLine();
				int height = img.height();
				int width = img.width();
				
				std::cout << "Bytes per line: " << bytesPerLine << std::endl;
				std::cout << "Height: " << height << std::endl;
				std::cout << "Width: " << width << std::endl;
				
				std::thread t1([&data, &bytesPerLine, &height, &width, &CompressionFuncReal]()
					{
						for (int y = 0; y < height / 2; y++)
						{
							for (int x = 0; x < width; x++)
							{
								uchar* pixel = data + y * bytesPerLine + x * 4;
								uchar* pixel2 = data + y * bytesPerLine + x * 4;

								std::cout << "Pixel (BGR): " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;

								pixel2[0] = CompressionFuncReal(pixel[0]);  // Kana³ niebieski
								pixel2[1] = CompressionFuncReal(pixel[1]);  // Kana³ zielony
								pixel2[2] = CompressionFuncReal(pixel[2]);  // Kana³ czerwony

								std::cout << "Pixel after \"Compression\" (BGR): " << (int)pixel2[0] << " " << (int)pixel2[1] << " " << (int)pixel2[2] << std::endl;
							}
						}
					});
				
				//for (int y = 0; y < height; y++)
				//{
				//	for (int x = 0; x < width; x++)
				//	{
				//		uchar* pixel = data + y * bytesPerLine + x * 4;
				//		uchar* pixel2 = data + y * bytesPerLine + x * 4;
				//
				//		std::cout << "Pixel (BGR): " << (int)pixel[0] << " " << (int)pixel[1] << " " << (int)pixel[2] << std::endl;

				//		pixel2[0] = CompressionFuncReal(pixel[0]);  // Kana³ niebieski
				//		pixel2[1] = CompressionFuncReal(pixel[1]);  // Kana³ zielony
				//		pixel2[2] = CompressionFuncReal(pixel[2]);  // Kana³ czerwony
				//
				//		std::cout << "Pixel after \"Compression\" (BGR): " << (int)pixel2[0] << " " << (int)pixel2[1] << " " << (int)pixel2[2] << std::endl;
				//	}
				//}
				t1.join();
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
		else if(choice == "4")
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
