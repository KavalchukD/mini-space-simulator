#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>

#ifndef OM_DECLSPEC
#define OM_DECLSPEC
#endif
namespace om
{
using myGlfloat = float;
using myUint    = unsigned int;
// Attention! TODO Vector and Matrix below want optimization. Because of
// using stack memory when operator*, operator+ make an additional copy when
// return ResultVec is called. More efficient but not so beatiful is to
// return value as third reference parameter If you want to change storage
// of elements of Vector or Matrix you should correct functions sum and
// multiplicate for Matrix
template <std::size_t n>
struct OM_DECLSPEC Vector
{
    constexpr std::size_t      size() const { return n; };
    constexpr const myGlfloat* begin() const { return elements; }
    constexpr const myGlfloat* end() const { return (elements + n); }
    constexpr myGlfloat*       begin() { return elements; }
    constexpr myGlfloat*       end() { return (elements + n); }
    myGlfloat                  elements[n];
};

template <std::size_t n>
OM_DECLSPEC constexpr void multiplicate(const Vector<n>& vec1,
                                        const Vector<n>& vec2, Vector<n>& r_vec)
{
    auto elementMyltiplication = [](myGlfloat v1element, myGlfloat v2element) {
        return v1element * v2element;
    };

    std::transform(vec1.begin(), vec1.end(), vec2.begin(), r_vec.begin(),
                   elementMyltiplication);
}

template <std::size_t n>
OM_DECLSPEC constexpr void multiplicate(const Vector<n>&    vec1,
                                        const om::myGlfloat koef,
                                        Vector<n>&          r_vec)
{
    auto elementMyltiplication = [&koef](myGlfloat v1element) {
        return v1element * koef;
    };

    std::transform(vec1.begin(), vec1.end(), r_vec.begin(),
                   elementMyltiplication);
}

template <std::size_t n>
OM_DECLSPEC constexpr void sum(const Vector<n>& vec1, const Vector<n>& vec2,
                               Vector<n>& r_vec)
{
    std::transform(vec1.begin(), vec1.end(), vec2.begin(), r_vec.begin(),
                   [](myGlfloat v1element, myGlfloat v2element) {
                       return v1element + v2element;
                   });
}

template <std::size_t n>
OM_DECLSPEC constexpr void dif(const Vector<n>& vec1, const Vector<n>& vec2,
                               Vector<n>& r_vec)
{
    std::transform(vec1.begin(), vec1.end(), vec2.begin(), r_vec.begin(),
                   [](myGlfloat v1element, myGlfloat v2element) {
                       return v1element - v2element;
                   });
}

template <std::size_t n>
OM_DECLSPEC constexpr void unaryMinus(const Vector<n>& vec1, Vector<n>& r_vec)
{
    std::transform(vec1.begin(), vec1.end(), r_vec.begin(),
                   [](myGlfloat v1element) { return -v1element; });
}

template <std::size_t n>
OM_DECLSPEC constexpr Vector<n> operator*(const Vector<n>& vec1,
                                          const Vector<n>& vec2)
{
    Vector<n> outVec{};
    multiplicate(vec1, vec2, outVec);
    return outVec;
}

template <std::size_t n>
OM_DECLSPEC constexpr Vector<n> operator*(const Vector<n>&    vec1,
                                          const om::myGlfloat koef)
{
    Vector<n> outVec{};
    multiplicate(vec1, koef, outVec);
    return outVec;
}

template <std::size_t n>
OM_DECLSPEC constexpr Vector<n> operator+(const Vector<n>& vec1,
                                          const Vector<n>& vec2)
{
    Vector<n> resultVec{};
    sum(vec1, vec2, resultVec);
    return resultVec;
}

template <std::size_t n>
OM_DECLSPEC constexpr Vector<n> operator-(const Vector<n>& vec1,
                                          const Vector<n>& vec2)
{
    Vector<n> resultVec{};
    dif(vec1, vec2, resultVec);
    return resultVec;
}

template <std::size_t n>
OM_DECLSPEC constexpr Vector<n> operator-(const Vector<n>& vec)
{
    Vector<n> resultVec{};
    unaryMinus(vec, resultVec);
    return resultVec;
}

template <std::size_t n>
OM_DECLSPEC constexpr bool operator==(const Vector<n>& vec1,
                                      const Vector<n>& vec2)
{
    return std::equal(vec1.begin(), vec1.end(), vec2.begin());
}

template <std::size_t n, std::size_t m>
struct OM_DECLSPEC Matrix
{
public:
    constexpr std::size_t numberOfLines() const { return n; };
    constexpr std::size_t numberOfColumns() const { return m; };

    constexpr const Vector<n>* begin() const { return columns; }
    constexpr const Vector<n>* end() const { return (columns + m); }
    constexpr Vector<n>*       begin() { return columns; }
    constexpr Vector<n>*       end() { return (columns + m); }
    Vector<n>                  columns[m];
};

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr void transponate(const Matrix<n, m>& mat1,
                                       Matrix<m, n>&       r_mat)
{
    constexpr auto fullSize{ n * m };

    auto currentElementSrc{ mat1.begin()->begin() };
    auto currentDestinationElement{ r_mat.begin()->begin() };

    for (size_t dstLine = 0; dstLine < m; ++dstLine,
                ++currentDestinationElement,
                currentDestinationElement -= fullSize)
    {
        for (size_t dstColumn = 0; dstColumn < n;
             ++dstColumn, ++currentElementSrc, currentDestinationElement += m)
        {
            *currentDestinationElement = *currentElementSrc;
        }
    }
}

template <std::size_t n, std::size_t m>
static constexpr void saveLineAsVector(const Matrix<n, m>& mat1,
                                       Matrix<m, n>&       r_mat)
{
    constexpr auto fullSize{ n * m };

    auto currentElementSrc{ mat1.begin()->begin() };
    auto currentDestinationElement{ r_mat.begin()->begin() };

    for (size_t srcLine = 0; srcLine < n;
         ++srcLine, ++currentElementSrc, currentElementSrc -= fullSize)
    {
        for (size_t srcColumn = 0; srcColumn < m;
             ++srcColumn, ++currentDestinationElement, currentElementSrc += n)
        {
            *currentDestinationElement = *currentElementSrc;
        }
    }
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr void getOneMultipliedColumn(
    const Matrix<m, n>& mat1ReversedVectors, const Vector<m>& vec2,
    Vector<n>& r_vec)
{
    auto getOneMultipliedElement = [&vec2](const Vector<m>& mat1FirstLine) {
        Vector<m> currentMultipliedLine;
        multiplicate(mat1FirstLine, vec2, currentMultipliedLine);
        return std::accumulate(currentMultipliedLine.begin(),
                               currentMultipliedLine.end(),
                               static_cast<myGlfloat>(0.0));
    };

    std::transform(mat1ReversedVectors.begin(), mat1ReversedVectors.end(),
                   r_vec.begin(), getOneMultipliedElement);
}

template <std::size_t n, std::size_t m, std::size_t l>
OM_DECLSPEC constexpr void multiplicate(const Matrix<n, m>& mat1,
                                        const Matrix<m, l>& mat2,
                                        Matrix<n, l>&       r_mat)
{
    Matrix<m, n> mat1ReversedVectors{};
    saveLineAsVector(mat1, mat1ReversedVectors);

    auto currentR_matVector = r_mat.begin();

    for (const auto& currentColumnMatrix2 : mat2)
    {
        getOneMultipliedColumn(mat1ReversedVectors, currentColumnMatrix2,
                               *currentR_matVector);
        ++currentR_matVector;
    }
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr void multiplicate(const Matrix<n, m>& mat1,
                                        const Vector<m>& vec2, Vector<n>& r_vec)
{
    Matrix<m, n> mat1ReversedVectors{};
    saveLineAsVector(mat1, mat1ReversedVectors);

    getOneMultipliedColumn(mat1ReversedVectors, vec2, r_vec);
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr void sum(const Matrix<n, m>& mat1,
                               const Matrix<n, m>& mat2, Matrix<n, m>& r_mat)
{
    const auto firstElementMat1 = mat1.begin()->begin();
    const auto lastElementMat1  = mat1.end()->end();

    const auto firstElementMat2 = mat2.begin()->begin();

    const auto firstElementR_Mat = r_mat.begin()->begin();

    std::transform(firstElementMat1, lastElementMat1, firstElementMat2,
                   firstElementR_Mat,
                   [](myGlfloat v1element, myGlfloat v2element) {
                       return v1element + v2element;
                   });
}

template <std::size_t n, std::size_t m, std::size_t l>
OM_DECLSPEC constexpr Matrix<n, l> operator*(const Matrix<n, m>& mat1,
                                             const Matrix<m, l>& mat2)
{
    Matrix<n, l> outMat{};
    multiplicate(mat1, mat2, outMat);
    return outMat;
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr Matrix<n, m> operator+(const Matrix<n, m>& mat1,
                                             const Matrix<n, m>& mat2)
{
    Matrix<n, m> outMat{};
    sum(mat1, mat2, outMat);
    return outMat;
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr Vector<n> operator*(const Matrix<n, m>& mat1,
                                          const Vector<m>&    vec2)
{
    Vector<n> outVector{};
    multiplicate(mat1, vec2, outVector);
    return outVector;
}

template <std::size_t n, std::size_t m>
OM_DECLSPEC constexpr bool operator==(const Matrix<n, m>& mat1,
                                      const Matrix<n, m>& mat2)
{
    return std::equal(mat1.begin(), mat1.end(), mat2.begin());
}

class OM_DECLSPEC MatrixFunctor
{
public:
    // rotate other opposite clock direction in angle
    static Matrix<3, 3> getRotateMatrix(myGlfloat        angle,
                                                  const Vector<2>& center = {})
    {
        const Vector<3> column1{ std::cos(angle), std::sin(angle), 0 };
        const Vector<3> column2{ -std::sin(angle), std::cos(angle), 0 };

        const Vector<3> column3{
            -center.elements[0] * (column1.elements[0] - 1) -
                center.elements[1] * column2.elements[0],
            -center.elements[0] * column1.elements[1] -
                center.elements[1] * (column2.elements[1] - 1),
            1
        };

        return Matrix<3, 3>{ column1, column2, column3 };
    }

    // scale different for x and y
    static constexpr Matrix<3, 3> getScaleMatrix(const Vector<2>  scale,
                                                 const Vector<2>& center = {})
    {
        const Vector<3> column1{ scale.elements[0], 0, 0 };
        const Vector<3> column2{ 0, scale.elements[1], 0 };
        const Vector<3> column3{ center.elements[0] * scale.elements[0],
                                 center.elements[1] * scale.elements[1], 1 };

        return Matrix<3, 3>{ column1, column2, column3 };
    }

    // scale same for x and y
    static constexpr Matrix<3, 3> getScaleMatrix(const myGlfloat  scale,
                                                 const Vector<2>& center = {})
    {
        return getScaleMatrix({ scale, scale }, center);
    }

    static constexpr Matrix<3, 3> getOneMatrix()
    {
        return { { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } } };
    }

    static constexpr Matrix<3, 3> getShiftMatrix(const Vector<2>& shift)
    {
        auto oneMatrix                   = getOneMatrix();
        oneMatrix.columns[2].elements[0] = shift.elements[0];
        oneMatrix.columns[2].elements[1] = shift.elements[1];

        return oneMatrix;
    }
};

} // namespace om
