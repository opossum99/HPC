#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <omp.h>
#include <numeric>

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



void animateImage_parallel(PPMImage &img){
    PPMPixel *add;
    add = new PPMPixel[img.y];
    #pragma omp for
    for(int j = 0; j < img.y; j++){
        add[j] = img.data[img.x-1 + j*img.x];
	//printf("j = %d, thread = %d\n", j, omp_get_thread_num());
    }
    //printf("\n\n\n\n");
    for(int i = img.x-1; i > 0; i--){
        #pragma omp for
        for(int j = 0; j < img.y; j++){
	    img.data[i + j*img.x] =img.data[i + j*img.x - 1];
	}
    }
    #pragma omp for
    for(int j = 0; j < img.y; j++){
        img.data[j*img.x] = add[j];
    }
}


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

void animateImage(PPMImage &img){
    PPMPixel *add;
    add = new PPMPixel[img.y];
    for(int j = 0; j < img.y; j++){
        add[j] = img.data[img.x-1 + j*img.x];
    }
    for(int i = img.x-1; i > 0; i--){
        for(int j = 0; j < img.y; j++){
	    img.data[i + j*img.x] =img.data[i + j*img.x - 1];
	}
    }
    for(int j = 0; j < img.y; j++){
        img.data[j*img.x] = add[j];
    }
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
    omp_set_dynamic(0);
    omp_set_num_threads(10);
    PPMImage image;
    double start, end, start_par, end_par;
    readPPM("movie_car/car.ppm", image);
    start_par = omp_get_wtime();
    for(auto i = 0; i < 100; i++)
    {
	#pragma omp parallel shared(image)
        animateImage_parallel(image);
    }
    end_par = omp_get_wtime();
    
    start = omp_get_wtime();
    for(auto i = 0; i < 100; i++)
    {
        animateImage(image);
    }
    end = omp_get_wtime();
    printf("Time_par = %f\n",end_par - start_par);
    printf("Time = %f\n",end - start);
    writePPM("movie_car/car.ppm", image);
    return 0;
}




