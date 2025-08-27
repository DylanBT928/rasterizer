#pragma once

#include <cassert>
#include <iostream>

template <int n>
struct vec
{
    double data[n]{0};

    double& operator[](const int i)
    {
        assert(0 <= i && i < n);
        return data[i];
    }

    double operator[](const int i) const
    {
        assert(0 <= i && i < n);
        return data[i];
    }
};

template <int n>
std::ostream& operator<<(std::ostream& out, const vec<n>& v)
{
    for (int i{0}; i < n; ++i) out << v[i] << ' ';
    return out;
}

template <>
struct vec<3>
{
    double x{0};
    double y{0};
    double z{0};

    double& operator[](const int i)
    {
        assert(0 <= i && i < 3);
        return i ? (i == 1 ? y : z) : x;
    }

    double operator[](const int i) const
    {
        assert(0 <= i && i < 3);
        return i ? (i == 1 ? y : z) : x;
    }
};

typedef vec<3> vec3;
