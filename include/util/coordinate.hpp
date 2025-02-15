/*

Copyright (c) 2016, Project OSRM contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef COORDINATE_HPP_
#define COORDINATE_HPP_

#include "util/strong_typedef.hpp"

#include <boost/numeric/conversion/cast.hpp>

#include <cstddef>
#include <cstdint>
#include <iosfwd> //for std::ostream
#include <string>
#include <type_traits>

namespace osrm
{

constexpr const double COORDINATE_PRECISION = 1e6;

namespace util
{

OSRM_STRONG_TYPEDEF(int32_t, FixedLatitude)
OSRM_STRONG_TYPEDEF(int32_t, FixedLongitude)
OSRM_STRONG_TYPEDEF(double, FloatLatitude)
OSRM_STRONG_TYPEDEF(double, FloatLongitude)

/**
 * Converts a typed latitude from floating to fixed representation.
 *
 * \param floating typed latitude in floating representation.
 * \return typed latitude in fixed representation
 * \see Coordinate, toFloating
 */
inline FixedLatitude toFixed(const FloatLatitude floating)
{
    const auto latitude = static_cast<double>(floating);
    const auto fixed = boost::numeric_cast<std::int32_t>(latitude * COORDINATE_PRECISION);
    return FixedLatitude{fixed};
}

/**
 * Converts a typed longitude from floating to fixed representation.
 *
 * \param floating typed longitude in floating representation.
 * \return typed latitude in fixed representation
 * \see Coordinate, toFloating
 */
inline FixedLongitude toFixed(const FloatLongitude floating)
{
    const auto longitude = static_cast<double>(floating);
    const auto fixed = boost::numeric_cast<std::int32_t>(longitude * COORDINATE_PRECISION);
    return FixedLongitude{fixed};
}

/**
 * Converts a typed latitude from fixed to floating representation.
 *
 * \param fixed typed latitude in fixed representation.
 * \return typed latitude in floating representation
 * \see Coordinate, toFixed
 */
inline FloatLatitude toFloating(const FixedLatitude fixed)
{
    const auto latitude = static_cast<std::int32_t>(fixed);
    const auto floating = boost::numeric_cast<double>(latitude / COORDINATE_PRECISION);
    return FloatLatitude{floating};
}

/**
 * Converts a typed longitude from fixed to floating representation.
 *
 * \param fixed typed longitude in fixed representation.
 * \return typed longitude in floating representation
 * \see Coordinate, toFixed
 */
inline FloatLongitude toFloating(const FixedLongitude fixed)
{
    const auto longitude = static_cast<std::int32_t>(fixed);
    const auto floating = boost::numeric_cast<double>(longitude / COORDINATE_PRECISION);
    return FloatLongitude{floating};
}

// fwd. decl.
struct FloatCoordinate;

/**
 * Represents a coordinate based on longitude and latitude in fixed representation.
 *
 * To prevent accidental longitude and latitude flips, we provide typed longitude and latitude
 * wrappers. You can cast these wrappers back to their underlying representation or convert them
 * from one representation to the other.
 *
 * The two representation we provide are:
 *  - Fixed point
 *  - Floating point
 *
 * \see FloatCoordinate, toFixed, toFloating
 */
struct Coordinate
{
    FixedLongitude lon;
    FixedLatitude lat;

    Coordinate() : lon{std::numeric_limits<int>::min()}, lat{std::numeric_limits<int>::min()} {}

    Coordinate(const FloatCoordinate &other);

    Coordinate(const FloatLongitude lon_, const FloatLatitude lat_)
        : Coordinate(toFixed(lon_), toFixed(lat_))
    {
    }

    Coordinate(const FixedLongitude lon_, const FixedLatitude lat_) : lon(lon_), lat(lat_) {}

    template <class T> Coordinate(const T &coordinate) : lon(coordinate.lon), lat(coordinate.lat)
    {
        static_assert(!std::is_same<T, Coordinate>::value,
                      "This constructor should not be used for Coordinates");
        static_assert(std::is_same<decltype(lon), decltype(coordinate.lon)>::value,
                      "coordinate types incompatible");
        static_assert(std::is_same<decltype(lat), decltype(coordinate.lat)>::value,
                      "coordinate types incompatible");
    }

    bool IsValid() const;
    friend bool operator==(const Coordinate lhs, const Coordinate rhs);
    friend bool operator!=(const Coordinate lhs, const Coordinate rhs);
    friend std::ostream &operator<<(std::ostream &out, const Coordinate coordinate);
};

/**
 * Represents a coordinate based on longitude and latitude in floating representation.
 *
 * To prevent accidental longitude and latitude flips, we provide typed longitude and latitude
 * wrappers. You can cast these wrappers back to their underlying representation or convert them
 * from one representation to the other.
 *
 * The two representation we provide are:
 *  - Fixed point
 *  - Floating point
 *
 * \see Coordinate, toFixed, toFloating
 */
struct FloatCoordinate
{
    FloatLongitude lon;
    FloatLatitude lat;

    FloatCoordinate()
        : lon{std::numeric_limits<double>::min()}, lat{std::numeric_limits<double>::min()}
    {
    }

    FloatCoordinate(const Coordinate other)
        : FloatCoordinate(toFloating(other.lon), toFloating(other.lat))
    {
    }

    FloatCoordinate(const FixedLongitude lon_, const FixedLatitude lat_)
        : FloatCoordinate(toFloating(lon_), toFloating(lat_))
    {
    }

    FloatCoordinate(const FloatLongitude lon_, const FloatLatitude lat_) : lon(lon_), lat(lat_) {}

    bool IsValid() const;
    friend bool operator==(const FloatCoordinate lhs, const FloatCoordinate rhs);
    friend bool operator!=(const FloatCoordinate lhs, const FloatCoordinate rhs);
    friend std::ostream &operator<<(std::ostream &out, const FloatCoordinate coordinate);
};

bool operator==(const Coordinate lhs, const Coordinate rhs);
bool operator==(const FloatCoordinate lhs, const FloatCoordinate rhs);
std::ostream &operator<<(std::ostream &out, const Coordinate coordinate);
std::ostream &operator<<(std::ostream &out, const FloatCoordinate coordinate);

inline Coordinate::Coordinate(const FloatCoordinate &other)
    : Coordinate(toFixed(other.lon), toFixed(other.lat))
{
}
}
}

#endif /* COORDINATE_HPP_ */
