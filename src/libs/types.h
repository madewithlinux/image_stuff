//
// Created by j0sh on 11/15/15.
//

#ifndef IMAGE_STUFF_TYPES_H
#define IMAGE_STUFF_TYPES_H

#include <cstring>
#include "matrix.h"
#include "vect.h"


namespace image_utils {

    using containers::matrix;
    using containers::assert_same_size;

    using std::sqrt;
    using std::sin;
    using std::cos;
    using std::pow;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
    /*keep these constants here for conveniece even if they aren't used*/
    const double PI = 3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482;
    const double Pi = PI;
    const double pi = PI;
#pragma clang diagnostic pop

    typedef containers::vect<double,2> vect;
    typedef containers::vect<size_t,2> vect_ull;
    typedef containers::vect<long,2> vect_ll;

    struct grayscale {
        unsigned char g;
    };

    struct RGB {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    struct RGBA : RGB {
        unsigned char a;
    };

    struct grayscaled : grayscale { // d for defined
        bool d;
    };

    struct RGBd : RGB { // d for defined
        bool d;
    };

    struct RGBAd : RGBA { // d for defined
        bool d;
    };

    template<typename Iter1, typename Iter2>
    bool memcmp_iter_equal(const Iter1 it1, const Iter2 it2) {
        if (sizeof(decltype(*it1)) != sizeof(decltype(*it2))) {
            /*different sizes can't be equal*/
            return false;
        }
        return std::memcmp(&(*it1), &(it2), sizeof(decltype(*it1)));
    };
    template<typename T1, typename T2>
    bool memcmp_equal(const T1 &a, const T2 &b) {
        if (sizeof(T1) != sizeof(T2)) {
            /*different sizes can't be equal*/
            return false;
        }
        return std::memcmp(&a, &b, sizeof(a));
    };

    typedef matrix<grayscale> image_gs;
    typedef matrix<RGB> image_RGB;
    typedef matrix<RGBA> image_RGBA;
    typedef matrix<RGBd> image_RGBd;
    typedef matrix<RGBAd> image_RGBAd;
    typedef matrix<grayscaled> image_gsd;

}
#endif //IMAGE_STUFF_TYPES_H
