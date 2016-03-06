// (c) Copyright 2016 Josh Wright
#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include "../libs/generators.h"
#include "../libs/cpp_containers/lib/debug.h"
#include "../libs/colormaps.h"
#include "../libs/io.h"

#define DEBUG 0
#define DEBUG 1

int main(int argc, char const *argv[]) {
    using namespace image_utils;

#if !DEBUG
    if (argc < 4/*TODO: arg count*/) {
        /*0*/ std::cout << argv[0];
        /*1*/ std::cout << " <output filename>";
        /*2*/ std::cout << " <image x>";
        /*3*/ std::cout << " <image y>";
        /*4*/ std::cout << " <n>";
        /*5*/ std::cout << " <d>";
        /*6*/ std::cout << " [wave size]";
        /*7*/ std::cout << " [wave type]";
        /*8*/ std::cout << " [colormap]"; /*TODO: colormap*/
        /*9*/ std::cout << " [lookup table size]"; /*TODO with default*/
        std::cout << std::endl;
        std::cout << "wave size:         default 16" << std::endl;
        std::cout << "lookup table size: 2^x, default 20" << std::endl;
        return 1;
    }
    std::string output(argv[1]);
    matrix<double> grid(std::stoull(argv[2]), std::stoull(argv[3]));
    int n = std::stoi(argv[4]);
    int d = std::stoi(argv[5]);

    double distance_multiplier = 16;
    if (argc >= 6) {
        distance_multiplier = std::stod(argv[6]);
    }
    wave *w = nullptr;
    if (argc >= 7) {
        w = parse_wave_spec(argv[7]);
    } else {
        w = new wave_sawtooth();
    }

    size_t table_size2 = 20;
    if (argc >= 9) {
        table_size2 = std::stoull(argv[9]);
    }

    std::cout << "filling lookup table" << std::endl;
    distance_wave *rose_dist1 = new rose_dist(w, std::pow(2, table_size2),
                                                  distance_multiplier, n, d);

#else
    /*constants for debugging*/
    std::string output("/home/j0sh/Dropbox/code/Cpp/image_stuff/build/out.png");
    wave *w = new wave_sawtooth();
    distance_wave *rose_dist1 = new rose_dist(w, std::pow(2,20), 4*8, 3, 7);
    size_t z = 1500;
    matrix<double> grid(z, z);
//    int g;
//    std::cin >> g;

#endif

    std::cout << "rendering image" << std::endl;
    image_fill_2d_wave(grid, rose_dist1);

    delete w;
    delete rose_dist1;

//    colormap *map = new colormap_grayscale();
    colormap *map = new colormap_threecolor();
    color_write_image(grid, map, output);
    return 0;
}
