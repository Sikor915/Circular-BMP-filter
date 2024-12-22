#include "pch.h"
#include "Circus.h"

void Circus(long long x, long long y)
{
	std::cout << "Circus function called with arguments: " << x << " and " << y << std::endl;
}

void CompressionFuncCircus(int* pixelData, int size, int width, const int* endPixelAddress) 
{
    int height = size / width;

    // Start processing from pixelData, which points to (2,2)
    for (int row = 0; row < height; ++row) 
    {
        for (int col = 2; col < width - 2; ++col) 
        {
            // Calculate the current pixel's address
            int* currentPixelAddress = pixelData + ((row) * width + (col-2));

            // Stop processing if we reach or exceed the endPixelAddress
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

                        int* neighborPixelAddress = currentPixelAddress + (y * width + x);

                        // Extract color components
                        int neighborPixel = *neighborPixelAddress;
                        int red = (neighborPixel >> 16) & 0xFF;
                        int green = (neighborPixel >> 8) & 0xFF;
                        int blue = neighborPixel & 0xFF;

                        // Accumulate values
                        redSum += red;
                        greenSum += green;
                        blueSum += blue;
                    }
                }
            }

            // Compute averages
            int avgRed = redSum / 21;
            int avgGreen = greenSum / 21;
            int avgBlue = blueSum / 21;

            // Update the current pixel
            *currentPixelAddress = (0xFF << 24) | (avgRed << 16) | (avgGreen << 8) | avgBlue;
        }

    }
}



