// (c) Copyright 2016 Josh Wright
#pragma once

#include <util/debug.h>
#include <functional>
#include <map>
#include <memory>
#include "fractal_info.h"
#include "types.h"

namespace image_utils {
const double NOT_DEFINED = -1.0;

#define FRACTAL_POLYNOMIAL(name, expr)                                 \
  template <typename numeric>                                          \
  struct name {                                                        \
    typedef std::complex<numeric> complex;                             \
    std::complex<numeric> operator()(const std::complex<numeric> &z,   \
                                     const std::complex<numeric> &c) { \
      return expr;                                                     \
    }                                                                  \
  };

FRACTAL_POLYNOMIAL(func_standard, pow(z, numeric(2)) + c)
FRACTAL_POLYNOMIAL(func_cubic, pow(z, numeric(3)) + c)
FRACTAL_POLYNOMIAL(func_quadratic_rational, pow(z, numeric(2)) + pow(c, 2) / (pow(c, 4) - numeric(0.25)))
FRACTAL_POLYNOMIAL(func_inv_c, pow(z, numeric(2)) + numeric(1.0) / (c - numeric(1)))
FRACTAL_POLYNOMIAL(func_inv_c_parabola, pow(z, numeric(2)) + numeric(1.0) / c + numeric(0.25))

enum polynomial_t {
  STANDARD,
  CUBIC,
  QUADRATIC_RATIONAL,
  INV_C,
  INV_C_PARABOLA,
};
const std::map<std::string, polynomial_t> polynomial_names{
    {"standard", STANDARD},
    {"cubic", CUBIC},
    {"quadratic-rational", QUADRATIC_RATIONAL},
    {"inv-c", INV_C},
    {"inv-c-parabola", INV_C_PARABOLA},
};

void sine_transform(matrix<double> &in, const double multiplier = 1, const double rel_phase = 0,
                    bool preserve_zero = true);
void log_transform(matrix<double> &in, const double multiplier = 1);

/** t on range [0,1]*/
complex complex_circle(const complex center, const double r, const double t);

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct line;

struct rectangle;

struct split_rectangle;

struct rectangle_stack;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct line {
  vec_ull start_point;
  vec_ull end_point;
};

struct rectangle {
  size_t xmin, xmax, ymin, ymax;

  rectangle(){};

  rectangle(const size_t x_min, const size_t x_max, const size_t y_min, const size_t y_max)
      : xmin(x_min), xmax(x_max), ymin(y_min), ymax(y_max) {}

  std::array<line, 4> get_sides() const {
    return std::array<line, 4>{line{{xmin, ymin}, {xmax, ymin}}, line{{xmin, ymin}, {xmin, ymax}},
                               line{{xmax, ymin}, {xmax, ymax}}, line{{xmin, ymax}, {xmax, ymax}}};
  };
};

struct split_rectangle {
  // basically an option type holding either 4 rectangles or nothing
  bool did_split;
  rectangle rectangles[4];
};

struct rectangle_stack : public std::vector<rectangle> {
  rectangle_stack(size_t sz) : std::vector<rectangle>() { reserve(sz); };

  rectangle pop() {
    rectangle ret = back();
    pop_back();
    return ret;
  }

  void push(const rectangle &rect) { push_back(rect); }
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class fractal {
 public:
  matrix<double> iterations;
  matrix<bool> grid_mask;
  size_t max_iterations = 512;
  bool do_grid = false;
  bool is_julia = false;
  bool smooth = false;
  bool do_sine_transform = true;
  bool subsample = false;
  double mul = 1;

  void run();
  virtual void run_singlethread() = 0;
  virtual void run_multithread() = 0;

  virtual void read_config(const fractal_info &cfg) = 0;

  fractal();
  fractal(const size_t w, const size_t h) : iterations(w, h, NOT_DEFINED), grid_mask(w, h, 0) {}
  fractal(const fractal &rhs)
      : iterations(rhs.iterations),
        grid_mask(rhs.grid_mask),
        max_iterations(rhs.max_iterations),
        do_grid(rhs.do_grid),
        is_julia(rhs.is_julia),
        smooth(rhs.smooth),
        do_sine_transform(rhs.do_sine_transform),
        subsample(rhs.subsample),
        mul(rhs.mul) {}
};

typedef std::shared_ptr<fractal> fractal_ref;

fractal_ref get_fractal(const fractal_info &cfg);

//////////////////////////////////////////////////////////////////

template <typename numeric = double, class polynomial = func_standard<numeric>>
class fractal_impl : public fractal {
  polynomial poly;

 public:
  typedef vect<numeric, 2> vec2;
  typedef vect<numeric, 3> vec3;
  typedef vect<numeric, 4> vec4;
  typedef std::complex<numeric> complex;

  static vec2 calc_pixel_widths(size_t x, size_t y, const numeric &zoom) {
    numeric dx = numeric(2) / zoom;
    numeric dy = numeric(2) / zoom;
    if (x > y) {
      /* widescreen image */
      dx = numeric(1.0) * x / y * dy;
    } else if (y > x) {
      /* portrait */
      dy = numeric(1.0) * y / x * dx;
    }  // otherwise square
    return vec2{dx / x, dy / y};
  }

  static vec4 calc_bounds(size_t x, size_t y, const vec2 &center, const numeric &zoom) {
    numeric dx = numeric(2.0) / numeric(zoom);
    numeric dy = numeric(2.0) / numeric(zoom);
    if (x > y) {
      /* widescreen image */
      dx = numeric(1.0) * x / y * dy;
    } else if (y > x) {
      /* portrait */
      dy = numeric(1.0) * y / x * dx;
    }  // otherwise square
    return vec4{
        center[0] - dx, center[0] + dx, center[1] - dy, center[1] + dy,
    };
  }

  template <bool smooth>
  double fractal_cell_(const complex &_z, const complex &c, const size_t max_iterations) {
    const numeric cap = numeric(max_iterations) * numeric(max_iterations);
    complex z = _z;
    for (size_t i = 0; i < max_iterations; i++) {
      z = poly(z, c);
      if (norm(z) > cap) {
        if (smooth) {
          // return double( numeric(i) - log(log(norm(z) + numeric(1)) + numeric(1)) + numeric(4.0));
          return i - log2(log2(norm(z) + 1) + 1) + 4.0;
        } else {
          return double(i);
        }
      }
    }
    return double(0.0);
  }

  double fractal_cell(const complex &z, const complex &c, const size_t max_iter,
                      const bool smooth) {
    if (smooth) {
      return fractal_cell_<true>(z, c, max_iter);
    } else {
      return fractal_cell_<false>(z, c, max_iter);
    }
  }

  double iterate_cell(const complex pos) {
    if (subsample) {
      double out[] = {0, 0, 0, 0};
      if (is_julia) {
        out[0] = fractal_cell(pos + complex(-pixel_width_x, 0), c, max_iterations, smooth);
        out[1] = fractal_cell(pos + complex(pixel_width_x, 0), c, max_iterations, smooth);
        out[2] = fractal_cell(pos + complex(0, -pixel_width_y), c, max_iterations, smooth);
        out[3] = fractal_cell(pos + complex(0, pixel_width_y), c, max_iterations, smooth);
      } else {
        constexpr complex c0 = complex(0, 0);
        out[0] = fractal_cell(c0, pos + complex(-pixel_width_x, 0), max_iterations, smooth);
        out[1] = fractal_cell(c0, pos + complex(pixel_width_x, 0), max_iterations, smooth);
        out[2] = fractal_cell(c0, pos + complex(0, -pixel_width_y), max_iterations, smooth);
        out[3] = fractal_cell(c0, pos + complex(0, pixel_width_y), max_iterations, smooth);
      }
      return ((out[0] + out[1]) + (out[2] + out[3])) / 4;
    } else {
      if (is_julia) {
        return fractal_cell(pos, c, max_iterations, smooth);
      } else {
        return fractal_cell(complex(0, 0), pos, max_iterations, smooth);
      }
    }
  }

 public:
  numeric pixel_width_x;
  numeric pixel_width_y;
  complex c = complex(0.0, 0.0);

 protected:
  // vec4 bounds{-2, 2, -2, 2};
  vec4 bounds;

 public:
  bool process_line(const line &l) {
    const vec_ull start = l.start_point;
    const vec_ull end = l.end_point;
    // handle lines containing only a single pixel
    const vec_ull diff = ((start - end) != vec_ull{0, 0}) ? (end - start).unitV() : vec_ull{0, 0};
    const size_t length = (end - start).norm();
    bool out = true;

    for (size_t i = 0; i <= length; i++) {
      vec_ull pos = start + diff * i;
      if (iterations(pos) == NOT_DEFINED) {
        // imaginary axis is different because it points opposite our +y axis
        complex complex_pos = index_to_complex(pos);
        iterations(pos) = iterate_cell(complex_pos);
      }
      if (iterations(pos) != iterations(start)) {
        out = false;
      }
    }
    return out;
  }

  void set_zoom(const vec2 &center, const numeric &zoom) {
    bounds = calc_bounds(iterations.x(), iterations.y(), center, zoom);
    auto wid = calc_pixel_widths(iterations.x(), iterations.y(), zoom);
    pixel_width_x = wid[0];
    pixel_width_y = wid[1];
  }

  split_rectangle process_rectangle(rectangle r) {
    bool edges_equal = true;
    for (auto &side : r.get_sides()) {
      // pre-calculate to avoid lazy evaluation skipping
      bool res = process_line(side);
      edges_equal = edges_equal && res;
    }
    size_t shortest_edge = std::min(r.xmax - r.xmin, r.ymax - r.ymin);
    if (!edges_equal && shortest_edge > 1) {
      // must be careful how we round up and down because rectangles are
      // inclusive
      // on all bounds
      return {
          true,
          {
              rectangle(r.xmin, (r.xmin + r.xmax) / 2, r.ymin, (r.ymin + r.ymax) / 2),
              rectangle((r.xmin + r.xmax) / 2, r.xmax, r.ymin, (r.ymin + r.ymax) / 2),
              rectangle(r.xmin, (r.xmin + r.xmax) / 2, (r.ymin + r.ymax) / 2, r.ymax),
              rectangle((r.xmin + r.xmax) / 2, r.xmax, (r.ymin + r.ymax) / 2, r.ymax),
          },
      };
    } else if (edges_equal /*&& shortest_edge < longest_bound / 2*/) {
      numeric iter_fill = iterations(r.xmin, r.ymin);
      for (size_t i = r.xmin; i <= r.xmax; i++) {
        for (size_t j = r.ymin; j <= r.ymax; j++) {
          iterations(i, j) = double(iter_fill);
        }
      }
      if (do_grid) {
        for (size_t j = r.ymin; j < r.ymax; j++) {
          grid_mask(r.xmin, j) = true;
        }
        for (size_t i = r.xmin; i < r.xmax; i++) {
          grid_mask(i, r.ymin) = true;
        }
      }
    }
    return {false, {}};
  }

  complex index_to_complex(const vec_ull &pos) {
    return complex((pos[0] * 1.0 / iterations.x()) * (bounds[1] - bounds[0]) + bounds[0],
                   bounds[3] - (pos[1] * 1.0 / iterations.y()) * (bounds[3] - bounds[2]));
  }

  vec4 get_bounds() const { return bounds; }

  void set_bounds(vec4 bounds) { this->bounds = bounds; }

  /////////////////////////////////////////////////////////////////////////////

  virtual void run_multithread() {
    std::fill(iterations.begin(), iterations.end(), NOT_DEFINED);
    const size_t stack_size = iterations.x() * iterations.y();
    std::vector<rectangle> rectange_stack;
    // four quadrants for starting
    rectange_stack.push_back(rectangle(0, iterations.x() / 2, 0, iterations.y() / 2));
    rectange_stack.push_back(
        rectangle(iterations.x() / 2, iterations.x() - 1, 0, iterations.y() / 2));
    rectange_stack.push_back(
        rectangle(0, iterations.x() / 2, iterations.y() / 2, iterations.y() - 1));
    rectange_stack.push_back(
        rectangle(iterations.x() / 2, iterations.x() - 1, iterations.y() / 2, iterations.y() - 1));

    rectange_stack.reserve(stack_size);
    while (!rectange_stack.empty()) {
      std::vector<rectangle> new_stack;
      new_stack.reserve(stack_size);
#pragma omp parallel for schedule(dynamic)
      for (size_t i = 0; i < rectange_stack.size(); i++) {
        auto v = process_rectangle(rectange_stack[i]);

        if (v.did_split) {
#pragma omp critical
          {
            new_stack.push_back(v.rectangles[0]);
            new_stack.push_back(v.rectangles[1]);
            new_stack.push_back(v.rectangles[2]);
            new_stack.push_back(v.rectangles[3]);
          }
        }
      }
      rectange_stack = new_stack;
    }

    if (do_sine_transform) {
      iterations = iterations;
      log_transform(iterations);
      sine_transform(iterations, mul);
    }

    if (do_grid) {
      /*get max iteration (that was used) and use that*/
      double grid_color = *std::max_element(iterations.begin(), iterations.end());
#pragma omp parallel for schedule(static) collapse(2)
      for (size_t i = 0; i < iterations.x(); ++i) {
        for (size_t j = 0; j < iterations.y(); ++j) {
          if (grid_mask(i, j)) {
            iterations(i, j) = grid_color;
          }
        }
      }
    }
  }

  /////////////////////////////////////////////////////////////////////////////

  void run_singlethread() {
    rectangle_stack stack(256);
    run_singlethread(stack);
  }

  virtual void run_singlethread(rectangle_stack &stack) {
    std::fill(iterations.begin(), iterations.end(), NOT_DEFINED);
    stack.push_back(rectangle(0, iterations.x() / 2, 0, iterations.y() / 2));
    stack.push_back(rectangle(iterations.x() / 2, iterations.x() - 1, 0, iterations.y() / 2));
    stack.push_back(rectangle(0, iterations.x() / 2, iterations.y() / 2, iterations.y() - 1));
    stack.push_back(
        rectangle(iterations.x() / 2, iterations.x() - 1, iterations.y() / 2, iterations.y() - 1));

    while (!stack.empty()) {
      rectangle current = stack.pop();
      split_rectangle sp = process_rectangle(current);
      if (sp.did_split) {
        stack.push(sp.rectangles[0]);
        stack.push(sp.rectangles[1]);
        stack.push(sp.rectangles[2]);
        stack.push(sp.rectangles[3]);
      }
    }
    if (do_sine_transform) {
      log_transform(iterations);
      sine_transform(iterations, mul);
    }
  }

  /////////////////////////////////////////////////////////////////////////////

  fractal_impl(const size_t w, const size_t h, polynomial poly = polynomial())
      : fractal(w, h), poly(poly) {}

  fractal_impl(polynomial poly = polynomial()) : poly(poly) {}

  fractal_impl(const fractal_impl<numeric, polynomial> &rhs)
      : fractal(rhs),
        pixel_width_x(rhs.pixel_width_x),
        pixel_width_y(rhs.pixel_width_y),
        poly(rhs.poly),
        c(rhs.c) {}

  virtual void read_config(const fractal_info &cfg) {
    iterations = matrix<double>(cfg.x, cfg.y, NOT_DEFINED);
    grid_mask = matrix<bool>(cfg.x, cfg.y, 0);
    pixel_width_x = 2.0 / cfg.x;
    pixel_width_y = 2.0 / cfg.y;
    set_zoom(vec2{cfg.r, cfg.i}, cfg.zoom);
    c = complex(cfg.cr, cfg.ci);
    subsample = cfg.subsample;
    smooth = cfg.smooth;
    do_grid = cfg.do_grid;
    is_julia = cfg.is_julia;
    max_iterations = cfg.iter;
    mul = cfg.mul;
  }
};
};
