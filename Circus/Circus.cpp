#include "pch.h"
#include "Circus.h"

/*
* Temat: Filtr dolnoprzepustowy kolowy
* Algorytm usrednia wartosci pikseli bazujac na macierzy sasiadow 5x5, tworzac efekt rozmycia.
* Ta implementacja pomija piksele na brzegach obrazu.
* Data wykonania: 2025-01-14
* Autor: Kacper Sikorski, INF/5 semestr
* ---------------------------------------------------------------------------------------------------------------
* Wersja 1.0 - 2024-11-03                                                                                       |
* Dzialajacy algorytm w wersji C++. Wykorzystuje potrojna petle do iteracji po kazdym sasiedzie kazdego piksela.|
* Algorytm przechodzi przez caly obraz, odpowiednio unikajac czesci poza obrazem.                               |
* ---------------------------------------------------------------------------------------------------------------            
* Wersja 1.1 - 2025-01-04                                                                                       |
* Zmienilem dzialanie algorytmu by przechodzil on na bazie ilosci wierszy do przetworzenia, a nie na podstawie  |
* wielkosci vectora.                                                                                            |
* ---------------------------------------------------------------------------------------------------------------
*/

void CompressionFuncCircus(int* pixelData, int rowsToProcess, int width, const int* endPixelAddress, int* OGData) 
{
    // Precomputed offsets for the 5x5 neighborhood excluding corners
    const int offsets[21][2] = {
                  {-2, -1}, {-2, 0}, {-2, 1},
        {-1, -2}, {-1, -1}, {-1, 0}, {-1, 1}, {-1, 2},
        {0, -2},  {0, -1} , {0, 0} , {0, 1} , {0, 2},
        {1, -2},  {1, -1} , {1, 0} , {1, 1} , {1, 2},
                  {2, -1} , {2, 0} , {2, 1}
    };
    for (int row = 0; row < rowsToProcess; ++row) 
    {
        for (int col = 2; col < width - 2; ++col) 
        {
            // Calculate the current pixel's address
            int* OGPixelAddress{ OGData + ((row)*width + (col - 2)) },
                * updatedPixelAddress{ pixelData + ((row)*width + (col - 2)) };

            if (OGPixelAddress > endPixelAddress)
                return;

            int redSum{ 0 }, greenSum{ 0 }, blueSum{ 0 }, neighborPixel{ 0 },
                avgRed{ 0 }, avgGreen{ 0 }, avgBlue{ 0 };

            for (int i = 0; i < 21; i++)
            {
                int y = offsets[i][0];
				int x = offsets[i][1];
				neighborPixel = *(OGPixelAddress + (y * width + x));
				redSum += (neighborPixel >> 16) & 0xFF;
				greenSum += (neighborPixel >> 8) & 0xFF;
				blueSum += (neighborPixel & 0xFF);
            }

            // Compute averages
            avgRed = (redSum / 21);
            avgGreen = (greenSum / 21);
            avgBlue = (blueSum / 21);

            // Update the current pixel
            *updatedPixelAddress = (0xFF << 24) | (avgRed << 16) | (avgGreen << 8) | avgBlue;
        }

    }
}



