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
 * Declaration of the base class of all order 1 potentials.
 * Subclasses have to implement calculateEnergy, calculateForce and calculateForceAndEnergy.
 * The first three methods take a modifiable reference and a particle's position. The last method is for replication
 * of the potential, so that it can be assigned to multiple particle types.
 *
 * @file PotentialOrder1.h
 * @brief Declaration of the base class of all order 1 potentials
 * @author clonker
 * @date 31.05.16
 */

#ifndef READDY_MAIN_POTENTIALORDER1_H
#define READDY_MAIN_POTENTIALORDER1_H

#include <readdy/model/potentials/Potential.h>

namespace readdy {
namespace model {
namespace potentials {
class PotentialOrder1 : public Potential {

public:
    PotentialOrder1(const std::string &name) : Potential(name, 1) {}

    virtual double calculateEnergy(const Vec3 &position) const = 0;

    virtual void calculateForce(Vec3 &force, const Vec3 &position) const = 0;

    virtual void calculateForceAndEnergy(Vec3 &force, double &energy, const Vec3 &position) const = 0;

    virtual void configureForType(const unsigned int type) {}

    virtual double getRelevantLengthScale() const noexcept = 0;

};
}
}
}
#endif //READDY_MAIN_POTENTIALORDER1_H