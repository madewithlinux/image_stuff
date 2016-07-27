//
// Created by j0sh on 5/27/16.
//

#ifndef IMAGE_STUFF_MANDELBROT_H
#define IMAGE_STUFF_MANDELBROT_H

#include "types.h"

namespace image_utils {

    class fractal {

        struct line {
            vec_ull start_point;
            vec_ull end_point;
        };

        struct rectangle {
            // bounds are inclusive
            size_t xmin, xmax, ymin, ymax;
            vec_ull corners[4];
            line sides[4];

            rectangle();

            rectangle(const size_t x_min, const size_t x_max,
                      const size_t y_min, const size_t y_max);
        };

        struct split_rectangle {
            // basically an option type holding either 4 rectangles or nothing
            bool did_split;
            rectangle rectangles[4];
        };

        matrix<double> iterations;
        matrix<bool> grid_mask;
        size_t max_iterations = 512;
        const std::array<double, 4> bounds{-2, 2, -2, 2};
        bool do_grid = false;
        bool is_julia = false;
        bool smooth = false;
        double color_multiplier = 0.15;
        bool do_sine_transform = true;
        complex c = complex(0.0, 0);

        double iterate_cell(const complex pos);

        bool process_line(const line &l);

        split_rectangle process_rectangle(rectangle r);


    public:

        fractal(const size_t w, const size_t h);

        void set_max_iterations(size_t max_iterations);

        void set_do_grid(bool do_grid);

        void set_is_julia(bool is_julia);

        void set_smooth(bool smooth);

        void set_color_multiplier(double color_multiplier);

        void set_do_sine_transform(bool do_sine_transform);

        void set_c(const complex &c);

        matrix<double> run();
    };

    /** bounds is in the form {xmin, xmax, ymin, ymax} */
    void check_bounds(const std::array<double, 4> &bounds);

    /** t on range [0,1]*/
    complex complex_circle(const complex center, const double r, const double t);
}
#endif //IMAGE_STUFF_MANDELBROT_H
