#pragma once

#include <cassert>
#include <cmath>
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

    const double& operator[](const int i) const
    {
        assert(0 <= i && i < n);
        return data[i];
    }
};

template <int n>
double operator*(const vec<n>& lhs, const vec<n>& rhs)
{
    double res{0};
    for (int i{n}; i--; res += lhs[i] * rhs[i]);
    return res;
}

template <int n>
vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> res{lhs};
    for (int i{n}; i--; res[i] += rhs[i]);
    return res;
}

template <int n>
vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs)
{
    vec<n> res{lhs};
    for (int i{n}; i--; res[i] -= rhs[i]);
    return res;
}

template <int n>
vec<n> operator*(const vec<n>& lhs, const double& rhs)
{
    vec<n> res{lhs};
    for (int i{n}; i--; res[i] *= rhs);
    return res;
}

template <int n>
vec<n> operator*(const double& lhs, const vec<n>& rhs)
{
    return rhs * lhs;
}

template <int n>
vec<n> operator/(const vec<n>& lhs, const double& rhs)
{
    vec<n> res{lhs};
    for (int i{n}; i--; res[i] /= rhs);
    return res;
}

template <int n>
std::ostream& operator<<(std::ostream& out, const vec<n>& v)
{
    for (int i{0}; i < n; ++i) out << v[i] << ' ';
    return out;
}

template <>
struct vec<2>
{
    double x{0};
    double y{0};

    double& operator[](const int i)
    {
        assert(0 <= i && i < 2);
        return i ? y : x;
    }

    double operator[](const int i) const
    {
        assert(0 <= i && i < 2);
        return i ? y : x;
    }
};

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

template <>
struct vec<4>
{
    double x{0};
    double y{0};
    double z{0};
    double w{0};

    double& operator[](const int i)
    {
        assert(0 <= i && i < 4);
        return i < 2 ? (i ? y : x) : (i == 2 ? z : w);
    }

    double operator[](const int i) const
    {
        assert(0 <= i && i < 4);
        return i < 2 ? (i ? y : x) : (i == 2 ? z : w);
    }

    vec<2> xy() const { return {x, y}; }

    vec<3> xyz() const { return {x, y, z}; }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;

template <int n>
double norm(const vec<n>& v)
{
    return std::sqrt(v * v);
}

template <int n>
vec<n> normalized(const vec<n>& v)
{
    const double len{norm(v)};

    if (len == 0.0)
        return v;

    return v / len;
}

inline vec3 cross(const vec3& v1, const vec3& v2)
{
    return {v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x};
}

template <int n>
struct dt;

template <int nrows, int ncols>
struct mat
{
    vec<ncols> rows[nrows]{};

    vec<ncols>& operator[](const int idx)
    {
        assert(0 <= idx && idx < nrows);
        return rows[idx];
    }

    const vec<ncols>& operator[](const int idx) const
    {
        assert(0 <= idx && idx < nrows);
        return rows[idx];
    }

    double det() const { return dt<ncols>::det(*this); }

    double cofactor(const int row, const int col) const
    {
        mat<nrows - 1, ncols - 1> submatrix;

        for (int i{nrows - 1}; i--;)
            for (int j{ncols - 1}; j--;
                 submatrix[i][j] = rows[i + int(row <= i)][j + int(col <= j)]);

        return submatrix.det() * ((row + col) % 2 ? -1 : 1);
    }

    mat<nrows, ncols> invertTranspose() const
    {
        mat<nrows, ncols> adjugateTranspose;

        for (int i{nrows}; i--;)
            for (int j{ncols}; j--; adjugateTranspose[i][j] = cofactor(i, j));

        return adjugateTranspose / (adjugateTranspose[0] * rows[0]);
    }

    mat<nrows, ncols> invert() const { return invertTranspose().transpose(); }

    mat<ncols, nrows> transpose() const
    {
        mat<ncols, nrows> res;

        for (int i{ncols}; i--;)
            for (int j{nrows}; j--; res[i][j] = rows[j][i]);

        return res;
    }
};

template <int nrows, int ncols>
vec<ncols> operator*(const vec<nrows>& lhs, const mat<nrows, ncols>& rhs)
{
    return (mat<1, nrows>{{lhs}} * rhs)[0];
}

template <int nrows, int ncols>
vec<nrows> operator*(const mat<nrows, ncols>& lhs, const vec<ncols>& rhs)
{
    vec<nrows> res;
    for (int i{nrows}; i--; res[i] = lhs[i] * rhs);
    return res;
}

template <int R1, int C1, int C2>
mat<R1, C2> operator*(const mat<R1, C1>& lhs, const mat<C1, C2>& rhs)
{
    mat<R1, C2> res;

    for (int i{R1}; i--;)
        for (int j{C2}; j--;)
            for (int k{C1}; k--; res[i][j] += lhs[i][k] * rhs[k][j]);

    return res;
}

template <int nrows, int ncols>
mat<nrows, ncols> operator*(const mat<nrows, ncols>& lhs, const double& val)
{
    mat<nrows, ncols> res;
    for (int i{nrows}; i--; res[i] = lhs[i] * val);
    return res;
}

template <int nrows, int ncols>
mat<nrows, ncols> operator/(const mat<nrows, ncols>& lhs, const double& val)
{
    mat<nrows, ncols> res;
    for (int i{nrows}; i--; res[i] = lhs[i] / val);
    return res;
}

template <int nrows, int ncols>
mat<nrows, ncols> operator+(const mat<nrows, ncols>& lhs,
                            const mat<nrows, ncols>& rhs)
{
    mat<nrows, ncols> res;

    for (int i{nrows}; i--;)
        for (int j{ncols}; j--; res[i][j] = lhs[i][j] + rhs[i][j]);

    return res;
}

template <int nrows, int ncols>
mat<nrows, ncols> operator-(const mat<nrows, ncols>& lhs,
                            const mat<nrows, ncols>& rhs)
{
    mat<nrows, ncols> res;

    for (int i{nrows}; i--;)
        for (int j{ncols}; j--; res[i][j] = lhs[i][j] - rhs[i][j]);

    return res;
}

template <int nrows, int ncols>
std::ostream& operator<<(std::ostream& out, const mat<nrows, ncols>& m)
{
    for (int i{0}; i < nrows; ++i) out << m[i] << std::endl;
    return out;
}

template <int n>
struct dt
{
    static double det(const mat<n, n>& src)
    {
        double res{0};
        for (int i{n}; i--; res += src[0][i] * src.cofactor(0, i));
        return res;
    }
};

template <>
struct dt<1>
{
    static double det(const mat<1, 1>& src) { return src[0][0]; }
};
