#include "pch.h"
#include "Circus.h"

void CompressionFuncCircus(int* pixelData, int rowsToProcess, int width, const int* endPixelAddress, int* OGPixelData) 
{
    for (int row = 0; row < rowsToProcess; ++row) 
    {
        for (int col = 2; col < width - 2; ++col) 
        {
            // Calculate the current pixel's address
            int* currentPixelAddress = OGPixelData + ((row)*width + (col - 2)),
                * processedPixelAddress = pixelData + ((row)*width + col - 2);

            if (currentPixelAddress > endPixelAddress)
                return;

            int redSum = 0, greenSum = 0, blueSum = 0;

            // Iterate over the 5x5 neighborhood
            for (int y = -2; y <= 2; ++y) 
            {
                for (int x = -2; x <= 2; ++x) 
                {
                    if (!((y == -2 && x == -2) || 
                        (y == 2 && x == -2) || 
                        (y == -2 && x == 2) || 
                        (y == 2 && x == 2))) 
                    {

                        // Extract color components
                        int neighborPixel = *(currentPixelAddress + (y * width + x));

                        // Accumulate values
                        redSum += ((neighborPixel >> 16) & 0xFF);
                        greenSum += ((neighborPixel >> 8) & 0xFF);
                        blueSum += (neighborPixel & 0xFF);
                    }
                }
            }

            // Compute averages
            int avgRed = redSum / 21;
            int avgGreen = greenSum / 21;
            int avgBlue = blueSum / 21;

            // Update the current pixel
            *processedPixelAddress = (0xFF << 24) | (avgRed << 16) | (avgGreen << 8) | avgBlue;
        }

    }
}



