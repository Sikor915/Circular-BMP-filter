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

    const int filterSum = 20;  

    int rowBytes = width * 4;

    for (int i = 0; i < size; i += 4) 
    {
        int redSum = 0, greenSum = 0, blueSum = 0;
        int filterCount = 0;

        int row = (i / 4) / width;
        int col = (i / 4) % width;

        for (int y = -2; y <= 2; ++y) 
        {
            for (int x = -2; x <= 2; ++x) 
            {
                if (filterMask[y + 2][x + 2] == 0) continue;

                int neighborRow = row + y;
                int neighborCol = col + x;

                if (neighborRow >= 0 && neighborRow < height && neighborCol >= 0 && neighborCol < width) 
                {
                    int neighborIndex = (neighborRow * width + neighborCol) * 4;

                    redSum += pixelData[neighborIndex];
                    greenSum += pixelData[neighborIndex + 1];
                    blueSum += pixelData[neighborIndex + 2];
                    filterCount++;
                }
            }
        }

        pixelData[i] = redSum / filterCount;
        pixelData[i + 1] = greenSum / filterCount;
        pixelData[i + 2] = blueSum / filterCount;
    }
    
}


