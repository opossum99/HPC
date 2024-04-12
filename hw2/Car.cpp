#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>

#define RGB_COMPONENT_COLOR 255

static const auto THREADS = std::thread::hardware_concurrency();

struct PPMPixel {
  int red;
  int green;
  int blue;
};

typedef struct {
  int x, y, all;
  PPMPixel *data;
} PPMImage;

void readPPM(const char *filename, PPMImage &img){
    std::ifstream file(filename);
    if (file) {
        std::string s;
        int rgb_comp_color;
        file >> s;
        if (s != "P3") {
            std::cout << "error in format" << std::endl;
            exit(9);
        }
        file >> img.x >> img.y;
        file >> rgb_comp_color;
        img.all = img.x * img.y;
        std::cout << s << std::endl;
        std::cout << "x=" << img.x << " y=" << img.y << " all=" << img.all
                  << std::endl;
        img.data = new PPMPixel[img.all];
        for (int i = 0; i < img.all; i++) {
            file >> img.data[i].red >> img.data[i].green >> img.data[i].blue;
        }
    } else {
        std::cout << "the file:" << filename << "was not found" << std::endl;
    }
    file.close();
}

void writePPM(const char *filename, PPMImage &img) {
    std::ofstream file(filename, std::ofstream::out);
    file << "P3" << std::endl;
    file << img.x << " " << img.y << " " << std::endl;
    file << RGB_COMPONENT_COLOR << std::endl;

    for (int i = 0; i < img.all; i++) {
        file << img.data[i].red << " " << img.data[i].green << " "
             << img.data[i].blue << (((i + 1) % img.x == 0) ? "\n" : " ");
    }
    file.close();
}

int main(int argc, char *argv[]) {
    PPMImage image;
    readPPM("car.ppm", image);
    writePPM("new_car.ppm", image);
    return 0;
}




