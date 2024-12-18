#include "pch.h"
#include "Circus.h"

void Circus(long long x, long long y)
{
	std::cout << "Circus function called with arguments: " << x << " and " << y << std::endl;
}

void CompressionFuncCircus(int* pixelData, int size, int width, int height)
{
    const int filterMask[5][5] = {
        {0, 1, 1, 1, 0},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1},
        {0, 1, 1, 1, 0}
    };

    int rowBytes = width;
    const int filterCount = 25;

    for (int i = 0; i < size; i++)
    {
        int redSum = 0, greenSum = 0, blueSum = 0;

        int row = i / width;
        int col = i % width;

        for (int y = -2; y <= 2; ++y)
        {
            for (int x = -2; x <= 2; ++x)
            {
                if (filterMask[y + 2][x + 2] == 0) continue;

                int neighborRow = row + y;
                int neighborCol = col + x;

                if (neighborRow >= 0 && neighborRow < height && neighborCol >= 0 && neighborCol < width)
                {
                    int neighborIndex = (neighborRow * width + neighborCol);

                    int neighborPixel = pixelData[neighborIndex];
                    int red = (neighborPixel >> 16) & 0xFF;
                    int green = (neighborPixel >> 8) & 0xFF;
                    int blue = neighborPixel & 0xFF;

                    redSum += red;
                    greenSum += green;
                    blueSum += blue;
                }
            }
        }

        int avgRed = redSum / filterCount;
        int avgGreen = greenSum / filterCount;
        int avgBlue = blueSum / filterCount;

        pixelData[i] = (0xFF << 24) | (avgRed << 16) | (avgGreen << 8) | avgBlue;
    }
    
}


