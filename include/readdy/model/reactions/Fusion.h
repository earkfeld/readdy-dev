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
 * Declaration of fusion reactions, i.e., A+B->C. Additionally to the types, they also have a rate, an educt
 * distance and two weights, which determine where (on a line between A and B) C should be placed.
 *
 * @file Fusion.h
 * @brief Header file containing the declaration for fusion reactions, i.e., A+B->C.
 * @author clonker
 * @date 20.06.16
 */

#ifndef READDY_MAIN_FUSION_H
#define READDY_MAIN_FUSION_H

#include "Reaction.h"

namespace readdy {
namespace model {
namespace reactions {

class Fusion : public Reaction<2> {
    using super = Reaction<2>;
public:
    Fusion(const std::string &name, unsigned int from1, unsigned int from2, unsigned int to,
           const double rate, const double eductDistance, const double weight1 = 0.5,
           const double weight2 = 0.5) :
            Reaction(name, rate, eductDistance, 0, 1){
        super::weight1 = weight1;
        super::weight2 = weight2;
        educts = {from1, from2};
        products = {to};

        const auto sum = weight1 + weight2;
        if (sum != 1) {
            this->weight1 /= sum;
            this->weight2 /= sum;
            log::console()->warn("The weights did not add up to 1, they were changed to weight1={}, weight2={}",
                                 this->weight1, this->weight2);
        }
    }

    const unsigned int getFrom1() const {
        return educts[0];
    }

    const unsigned int getFrom2() const {
        return educts[1];
    }

    const unsigned int getTo() const {
        return products[0];
    }

    virtual const ReactionType getType() override {
        return ReactionType::Fusion;
    }


};
}
}
}

#endif //READDY_MAIN_FUSION_H