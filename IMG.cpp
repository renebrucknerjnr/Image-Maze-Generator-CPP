/*
    Image-Maze-Generator using C++
    Copyright (C) 2025  Rene Bruckner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// email:  renebrucknerjnr@gmail.com
// g++ IMG.cpp -o IMG.exe -static-libstdc++ -static-libgcc -static

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <vector>


#pragma pack(push, 1) // ensure no padding
struct BITMAPFILEHEADER {
    unsigned short bfType = 0x4D42; // file type ("BM" = 0x4D42)
    unsigned int bfSize; // size of file in bytes
    unsigned short bfReserved1 = 0; // must be 0
    unsigned short bfReserved2 = 0; // must be 0
    unsigned int bfOffBits; // offset to pixel data
};

struct BITMAPINFOHEADER {
    unsigned int biSize; // size of BITMAPINFOHEADER structure
    int biWidth; // width of image in pixels
    int biHeight; // height of image in pixels
    unsigned short biPlanes = 1; // number of planes, must be 1
    unsigned short biBitCount; // number of bits per pixel (24 for rgb)
    unsigned int biCompression = 0; // type of compression (0 = none)
    unsigned int biSizeImage; // size of image data in bytes
    int biXPelsPerMeter; // horizontal resolution
    int biYPelsPerMeter; // vertical resolution
    unsigned int biClrUsed; // number of colors in color palette
    unsigned int biClrImportant; // number of important colors
};
#pragma pack(pop)
// BMP stores in BGR (not RGB)
// bottom-left to top-right
// each scanline (row of pixels) must be padded w/ a multiple of 4 bytes

void createImage(const std::vector<unsigned char>& rgbData, int width, int height, const char* fileName) {
    int row_padded_size = (width * 3 + 3) & (~3); // mult of 4 bytes
    int image_size = row_padded_size * height;

    // populate BITMAPFILEHEADER
    BITMAPFILEHEADER bmfh;
    // size already set
    bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + image_size;
    // reserved1 and 2 already set
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // populate BITMAPINFOHEADER
    BITMAPINFOHEADER bmih;
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = width; // of image
    bmih.biHeight = height;
    // planes already set
    bmih.biBitCount = 24;
    // compression already set
    bmih.biSizeImage = image_size;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    // Create and open a .bmp file
    // std::ofstream MyFile(fileName);
    FILE* file = fopen(fileName, "wb");
    if (!file) {
        std::cout << "Failed to create or edit file!" << std::endl;
        return;
    }

    // Write to the file    
    fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&bmih, sizeof(BITMAPINFOHEADER), 1, file);

    unsigned char padding[3] = {0, 0, 0};
    int padding_bytes = row_padded_size - (width * 3);
    for (int y = height - 1; y >= 0; --y) { // iterate from bottom row to top
        for (int x = 0; x < width; ++x) {
            int original_index = (y*width + x) * 3;
            fwrite(&rgbData[original_index + 2], 1, 1, file); // b
            fwrite(&rgbData[original_index + 1], 1, 1, file); // g
            fwrite(&rgbData[original_index + 0], 1, 1, file); // r
        }
        fwrite(padding, 1, padding_bytes, file); // write padding?
    }

    // Close the file
    fclose(file);
}

void mazeToImage(const std::vector<char>& maze, int width, int height, std::string fileName) {
    int dataLength = width*height*3;
    std::vector<unsigned char> data(dataLength); // rgb(a)
    for (int i = 0; i < dataLength; i+=3) {
        // 1 = path, 0 = wall, 2 = solution
        char c = maze[i/3];
        // data[i + 0] = (c == 1 ? rand()%255 : 0);
        data[i + 0] = (c == 0 ? 0 : (c == 1 ? 255 : 128));
        data[i + 1] = (c == 0 ? 0 : (c == 1 ? 255 : 128));
        data[i + 2] = (c == 0 ? 0 : (c == 1 ? 255 : 128));
    }

    std::cout << "creating \"" << fileName << "\" as image..." << std::endl;
    createImage(data, width, height, fileName.c_str());
    std::cout << "Done!" << std::endl;
}

void createMaze(std::vector<char>& arr, int width, int height) {
    // set array to unvisited
    for (int i = 0; i < width * height; i++) {
        arr[i] = 0;
    }
    for (int i = 0; i < width; i++) arr[i] = 2;
    for (int i = width*height-width; i < width*height; i++) arr[i] = 2;
    for (int i = 0; i < width*height; i+=width) arr[i] = 2;
    for (int i = width-1; i < width*height; i+=width) arr[i] = 2;

    // create stack of visited and "neighborless" nodes
    std::vector<unsigned int> stack = {(unsigned int)(1 + width)}; // push_back(n); pop_back(); back();

    // set starting pos to visited
    arr[1] = 1;
    arr[stack.back()] = 1;

    while (true) {
        int currI = stack.back();

        std::vector<char> availableDirs(0); // NESW
        if ((currI - 2 >= 0) && (arr[currI - 2] == 0))                   availableDirs.push_back('W');
        if ((currI + 2 < width*height) && (arr[currI + 2] == 0))         availableDirs.push_back('E');
        if ((currI - width*2 >= 0) && (arr[currI - width*2] == 0))           availableDirs.push_back('N');
        if ((currI + width*2 < width*height) && (arr[currI + width*2] == 0)) availableDirs.push_back('S');
    
        if (availableDirs.size()) { // not surrounded
            char choice = availableDirs[rand()%availableDirs.size()];

            switch (choice) {
            case 'N':
                arr[currI-width] = 1;
                currI = currI - width*2;
                break;

            case 'E':
                arr[currI+1] = 1;
                currI = currI + 2;
                break;

            case 'S':
                arr[currI+width] = 1;
                currI = currI + width*2;
                break;

            case 'W':
                arr[currI-1] = 1;
                currI = currI - 2;
                break;

            default:break;}

            arr[currI] = 1;
            stack.push_back(currI);
                    
        } else {
            stack.pop_back(); // no open neighbors
        }

        if (stack.size() <= 0) break;
    } // maze complete

    // fill edges with walls (not neccessary, but I like)
    for (int i = 0; i < width; i++) if (arr[i]==2) arr[i] = 0;
    for (int i = width*height-width; i < width*height; i++) if (arr[i]==2) arr[i] = 0;
    for (int i = 0; i < width*height; i+=width) if (arr[i]==2) arr[i] = 0;
    for (int i = width-1; i < width*height; i+=width) if (arr[i]==2) arr[i] = 0;
    arr[1] = 1;

    if (arr[width*height-1 - width*1 - 1]==1) { // open exit
        if (rand()%2) {
            arr[width*height-1 - 1] = 1;
        } else {
            arr[width*height-1 - width] = 1;
        }
    } else if (arr[width*height-1 - width*2 - 1]==1) {
        arr[width*height-1 - width*2] = 1;
    } else if (arr[width*height-1 - width*3 - 1]==1) {
        arr[width*height-1 - width*3] = 1;
    } else if (arr[width*height-1 - width*1 - 2]==1) {
        arr[width*height-1 - 2] = 1;
    } else if (arr[width*height-1 - width*1 - 3]==1) {
        arr[width*height-1 - 3] = 1;
    }
}

void solveMaze(std::vector<char>& arr, int width, int height) {
    // 1 = path
    // 0 = wall
    // 2 = solution

    std::vector<unsigned int> stack = {(unsigned int)(1 + width)}; // push_back(n); pop_back(); back();
    arr[1] = 2;
    arr[stack.back()] = 2;

    int lastI = stack.back();

    // go through maze, starting at start, until end found
    while (true) {
        int currI = stack.back();

        std::vector<char> availableDirs(0); // NESW
        if ((currI - 2 >= 0) && (arr[currI - 1] == 1) && (arr[currI - 2] == 1))                   availableDirs.push_back('W');
        if ((currI + 2 < width*height) && (arr[currI + 1] == 1) && (arr[currI + 2] == 1))         availableDirs.push_back('E');
        if ((currI - width*2 >= 0) && (arr[currI - width*1] == 1) && (arr[currI - width*2] == 1))           availableDirs.push_back('N');
        if ((currI + width*2 < width*height) && (arr[currI + width*1] == 1) && (arr[currI + width*2] == 1)) availableDirs.push_back('S');
    
        if (availableDirs.size()) { // not surrounded
            lastI = currI;
            char choice = availableDirs[rand()%availableDirs.size()];

            switch (choice) {
            case 'N':
                arr[currI-width] = 7;
                currI = currI - width*2;
                break;

            case 'E':
                arr[currI+1] = 7;
                currI = currI + 2;
                break;

            case 'S':
                arr[currI+width] = 7;
                currI = currI + width*2;
                break;

            case 'W':
                arr[currI-1] = 7;
                currI = currI - 2;
                break;

            default:break;}

            // set parrent
            if (currI == lastI) { // start
                arr[currI] = 2;
            } else if (currI == lastI - width*2) { // current N of last
                arr[currI] = 3;
            } else if (currI == lastI + 2) { // current E of last
                arr[currI] = 4;
            } else if (currI == lastI + width*2) { // current S of last
                arr[currI] = 5;
            } else if (currI == lastI - 2) { // current W of last
                arr[currI] = 6;
            }

            stack.push_back(currI);
                    
        } else {
            stack.pop_back(); // no open neighbors
        }

        if (stack.size() <= 0 || currI == width*height-1-width) break;
    } // maze filled

    // start at end, trace back through parrents to find solution
    int currI = width * height - width - 2;
    while (true) {
        if (arr[currI] == 3) { // move S
            arr[currI] = 2;
            arr[currI + width] = 2;
            currI = currI + width*2;
        } else if (arr[currI] == 4) { // move W
            arr[currI] = 2;
            arr[currI - 1] = 2;
            currI = currI - 2;            
        } else if (arr[currI] == 5) { // move N
            arr[currI] = 2;
            arr[currI - width] = 2;
            currI = currI - width*2;            
        } else if (arr[currI] == 6) { // move E
            arr[currI] = 2;
            arr[currI + 1] = 2;
            currI = currI + 2;            
        } else {
            arr[currI] = 2;
            break;
        }
        
    }

    // clear temp values to normal traversable path cell
    for (int i = 0; i < arr.size(); i++) {
        if (arr[i] != 0 && arr[i] != 1 && arr[i] != 2) arr[i] = 1;
    }

    // set maze exit as solution, instead of normal path
    for (int i = width * height - 1 - 2; i < width*height; i++) if (arr[i]==1) arr[i] = 2;
    for (int i = width * height - 1 - width*2; i < width*height; i+=width) if (arr[i]==1) arr[i] = 2;
}


int main() {
    srand(time(0));

    // char 1 byte
    // int 4 bytes
    int imgW = 64;
    int imgH = 36;
    bool makeSolution = true;
    std::string imgName = "Tester.bmp";

    // get user input
    std::cout << "Image-Maze-Generator  Copyright (C) 2025  Rene Bruckner" << std::flush;
    std::cout << "This program comes with ABSOLUTELY NO WARRANTY." << std::flush;
    std::cout << "This is free software, and you are welcome to redistribute it" << std::flush;
    std::cout << "under certain conditions; GNU General Public License." << std::flush;
    
    std::cout << std::endl << "Enter your desired width: " << std::flush;
    std::cin >> imgW;

    std::cout << std::endl << "Enter your desired height: " << std::flush;
    std::cin >> imgH;

    std::cout << std::endl << "Enter your desired file name for the " << imgW << "x" << imgH << " bmp (include .bmp at end): " << std::flush;
    std::cin >> imgName;

    std::cout << std::endl << "Enter 0 if you don't want to generate solution, else 1: " << std::flush;
    std::cin >> makeSolution;

    std::cout << std::endl;

    imgW = std::min(std::max(imgW + (1 - imgW % 2), 5), 3841);
    imgH = std::min(std::max(imgH + (1 - imgH % 2), 5), 3841);


    std::cout << "generating..." << std::endl;
    std::vector<char> mazeArr(imgW*imgH); // generate maze
    createMaze(mazeArr, imgW, imgH);
    mazeToImage(mazeArr, imgW, imgH, imgName);

    if (makeSolution) {
        std::cout << "solving..." << std::endl;
        std::vector<char> mazeArrSolved = mazeArr; // generate maze
        solveMaze(mazeArrSolved, imgW, imgH);
        mazeToImage(mazeArrSolved, imgW, imgH, ("SOLVED_" + imgName));        
    }
    
    std::cout << std::endl << "Use the following in the development mode of a browser to view small mazes (after opening maze.bmp in new tab):" << std::endl;
    std::cout << "document.getElementsByTagName(\"img\")[0].style.width=\"100vw\";document.getElementsByTagName(\"img\")[0].style.height=\"100vh\";" << std::endl;
    std::cout << std::endl << "Press enter to exit..." << std::flush;
    std::cin >> imgName;

    return 0;
}
