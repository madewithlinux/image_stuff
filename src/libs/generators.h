// (c) Copyright 2015 Josh Wright
#pragma once

#ifndef IMAGE_UTILS_GENERATORS
#define IMAGE_UTILS_GENERATORS

#include <iostream>
#include "types.h"
//#include "../libs/cpp_containers/lib/debug.h"

namespace image_utils {

    /////////////////////
    // wave generators //
    /////////////////////
    class wave {
    public:
        virtual long double operator()(const long double &x) const = 0;
    };

    class wave_triangle : public wave {
    public:
        virtual long double operator()(const long double &x) const;
    };

    class wave_sine : public wave {
    public:
        virtual long double operator()(const long double &x) const;
    };

    class wave_square : public wave {
    public:
        virtual long double operator()(const long double &x) const;
    };

    class wave_sawtooth : public wave {
    public:
        virtual long double operator()(const long double &x) const;
    };

    class wave_fourier_square : public wave {
        size_t n;
    public:
        wave_fourier_square(size_t _n) : n(_n) { }

        virtual long double operator()(const long double &x) const;
    };

    class wave_2d {
    public:
        virtual long double operator()(const long double &x,
                                       const long double &y) const = 0;
    };

    class rose_dist : public wave_2d {
        /*for debugging*/
//    public:
        struct cached_value {

            /*constants needed in general*/
            const long double t;

            /*constants needed for distance*/
            const long double C1_0;
            const long double C1_x1;
            const long double C1_y1;

            /*constants needed for derivative of distance*/
            const long double C2_0;
            const long double C2_x1;
            const long double C2_y1;

            cached_value(const long double n,
                         const long double _t) : t(_t),
                                                 C1_0(pow(cos(n * t), 2) *
                                                      pow(cos(t), 2) +
                                                      pow(cos(n * t), 2) *
                                                      pow(sin(t), 2)),
                                                 C1_x1(-2 * cos(n * t) *
                                                       cos(t)),
                                                 C1_y1(-2 * cos(n * t) *
                                                       sin(t)),
                                                 C2_0(-2 * n * cos(n * t) *
                                                      pow(cos(t), 2) *
                                                      sin(n * t) -
                                                      2 * n * cos(n * t) *
                                                      sin(n * t) *
                                                      pow(sin(t), 2)),
                                                 C2_x1(2 * n * cos(t) *
                                                       sin(n * t) +
                                                       2 * cos(n * t) * sin(t)),
                                                 C2_y1(2 * n * sin(n * t) *
                                                       sin(t) - 2 * cos(n * t) *
                                                                cos(t)) { }

            long double dist2(const long double x, const long double y) const {
                return C1_0
                       + C1_x1 * x + x * x
                       + C1_y1 * y + y * y;
            }

            long double dist(const long double x, const long double y) const {
                return std::sqrt(C1_0
                                 + C1_x1 * x + x * x
                                 + C1_y1 * y + y * y);
            }

            long double diff(const long double x,
                             const long double y) const {
                return C2_0 + C2_x1 * x + C2_y1 * y;
            }
        };

        std::vector<cached_value> lookup_table;
        long double max_t;
        size_t wid;
        wave *w;

        long double _find_min(size_t left, size_t right,
                              const long double &x,
                              const long double &y) const;

    public:
        /*TODO: wave size*/
        rose_dist(const long double n, const long double d,
                  const size_t table_size, const long double _max_t);

        virtual long double operator()(const long double &x,
                                       const long double &y) const;;
    };


    /////////////
    // fillers //
    /////////////
    void image_fill_concentric_waves(matrix<long double> &grid,
                                     const long double &mul,
                                     wave *wave_func);

    void image_fill_pointing_out(matrix<long double> &grid,
                                 const long double &mul,
                                 wave *wave_func);


    /*
     * theta_mul: larger => more angular ripples (each ripple is smaller)
     * dist_mult: larger => more radial ripples (each ripple is smaller)
     */
    void image_fill_circle_grid(matrix<long double> &grid,
                                const long double &theta_mul,
                                const long double &dist_mul,
                                wave *w1 = nullptr,
                                wave *w2 = nullptr);

    void image_fill_2d_wave(matrix<long double> &grid, wave_2d *w_2d);


}


#endif /*IMAGE_UTILS_GENERATORS*/