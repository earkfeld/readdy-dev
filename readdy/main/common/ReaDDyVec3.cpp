/********************************************************************
 * Copyright © 2016 Computational Molecular Biology Group,          *
 *                  Freie Universität Berlin (GER)                  *
 *                                                                  *
 * This file is part of ReaDDy.                                     *
 *                                                                  *
 * ReaDDy is free software: you can redistribute it and/or modify   *
 * it under the terms of the GNU Lesser General Public License as   *
 * published by the Free Software Foundation, either version 3 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU Lesser General Public License for more details.              *
 *                                                                  *
 * You should have received a copy of the GNU Lesser General        *
 * Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                  *
 ********************************************************************/


/**
 * << detailed description >>
 *
 * @file ReaDDyVec3.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 21.04.16
 */

#include <readdy/common/ReaDDyVec3.h>
#include <cassert>
#include <ostream>

namespace readdy {
namespace _internal {
ReaDDyVec3 &ReaDDyVec3::operator+=(const ReaDDyVec3 &rhs) {
    for(auto i=0; i < 3; ++i) { data[i] += rhs[i]; }
    return *this;
}

ReaDDyVec3 &ReaDDyVec3::operator*=(const scalar a) {
    for(auto i=0; i < 3; ++i) { data[i] *= a; }
    return *this;
}

ReaDDyVec3 &ReaDDyVec3::operator/=(const scalar a) {
    for(auto i=0; i < 3; ++i) { data[i] /= a; }
    return *this;
}

ReaDDyVec3::ReaDDyVec3(const std::array<scalar, 3> &xyz) : data(xyz) {}

ReaDDyVec3::ReaDDyVec3(scalar x, scalar y, scalar z) : x(x), y(y), z(z) {}

readdy::scalar ReaDDyVec3::operator[](const unsigned int i) const {
    assert(0 <= i && i < 3);
    return data[i];
}

readdy::scalar &ReaDDyVec3::operator[](const unsigned int i) {
    assert(0 <= i && i < 3);
    return data[i];
}

ReaDDyVec3::ReaDDyVec3() : ReaDDyVec3(0, 0, 0) {

}

bool ReaDDyVec3::operator==(const ReaDDyVec3 &rhs) const {
    bool result {true};
    for(auto i=0; i < 3; ++i) { result &= data[i] == rhs[i]; }
    return result;
}

bool operator<=(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    bool result {true};
    for(auto i=0; i < 3; ++i) { result &= lhs[i] <= rhs[i]; }
    return result;
}

bool operator>(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    return !(lhs <= rhs);
}

bool operator>=(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    bool result {true};
    for(auto i=0; i < 3; ++i) { result &= lhs[i] >= rhs[i]; }
    return result;
}

bool operator<(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    return !(lhs >= rhs);
}

bool ReaDDyVec3::operator!=(const ReaDDyVec3 &rhs) const {
    bool result {true};
    for(auto i=0; i < 3; ++i) { result &= data[i] != rhs[i]; }
    return result;
}

ReaDDyVec3 operator+(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    return {lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]};
}

ReaDDyVec3 operator+(const ReaDDyVec3 &lhs, const readdy::scalar rhs) {
    return {lhs[0] + rhs, lhs[1] + rhs, lhs[2] + rhs};
}

ReaDDyVec3 operator-(const ReaDDyVec3 &lhs, const readdy::scalar rhs) {
    return lhs + (-1 * rhs);
}

ReaDDyVec3 operator/(const ReaDDyVec3 &lhs, const readdy::scalar rhs) {
    return {lhs[0] / rhs, lhs[1] / rhs, lhs[2] / rhs};
}

ReaDDyVec3 operator-(const ReaDDyVec3 &lhs, const ReaDDyVec3 &rhs) {
    return {lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]};
}

std::ostream &operator<<(std::ostream &os, const ReaDDyVec3 &vec) {
    os << "ReaDDyVec3(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ")";
    return os;
}

ReaDDyVec3 ReaDDyVec3::cross(const ReaDDyVec3 &other) const {
    return {
            data[1] * other.data[2] - data[2] * other.data[1],
            data[2] * other.data[0] - data[0] * other.data[2],
            data[0] * other.data[1] - data[1] * other.data[0]
    };
}

scalar ReaDDyVec3::norm() const {
    return std::sqrt(normSquared());
}

scalar ReaDDyVec3::normSquared() const {
    scalar result {0};
    for(auto i=0; i < 3; ++i) { result += data[i]*data[i]; }
    return result;
}

ReaDDyVec3 &ReaDDyVec3::invertElementWise() {
    for(auto i=0; i < 3; ++i) {
        data[i] = static_cast<scalar>(1.) / data[i];
    }
    return *this;
}

ReaDDyVec3 &ReaDDyVec3::operator-=(const ReaDDyVec3 &rhs) {
    for(auto i=0; i < 3; ++i) {
        data[i] -= rhs.data[i];
    }
    return *this;
}

}
}