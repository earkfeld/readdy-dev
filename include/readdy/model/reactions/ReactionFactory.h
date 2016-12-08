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
 * The reaction factory is for creating reaction objects. In order to provide polymorphism, the templated
 * createReaction(args) method is executed by a dispatcher, that can be specialized in case a reaction type
 * needs to be overridden.
 *
 * @file ReactionFactory.h
 * @brief In this header, the reaction factory is declared.
 * @author clonker
 * @date 21.06.16
 */

#ifndef READDY_MAIN_REACTIONFACTORY_H
#define READDY_MAIN_REACTIONFACTORY_H

#include <string>
#include <unordered_map>
#include <type_traits>
#include <readdy/common/make_unique.h>
#include "Conversion.h"
#include "Enzymatic.h"
#include "Fission.h"
#include "Fusion.h"
#include "Decay.h"

namespace readdy {
namespace model {
namespace reactions {

class ReactionFactory {
public:
    template<typename R, typename... Args>
    std::unique_ptr<R> createReaction(Args &&... args) const {
        return std::unique_ptr<R>(ReactionFactory::get_dispatcher<R, Args...>::impl(
                this, std::forward<Args>(args)...)
        );
    }

protected:
    virtual Conversion *createConversion(const std::string &name, unsigned int from, unsigned int to,
                                         const double rate) const {
        return new Conversion(name, from, to, rate);
    };

    virtual Enzymatic *createEnzymatic(const std::string &name, unsigned int catalyst, unsigned int from,
                                       unsigned int to, const double rate,
                                       const double eductDistance) const {
        return new Enzymatic(name, catalyst, from, to, rate, eductDistance);
    };

    virtual Fission *createFission(const std::string &name, unsigned int from, unsigned int to1,
                                   unsigned int to2, const double rate, const double productDistance,
                                   const double weight1 = 0.5, const double weight2 = 0.5) const {
        return new Fission(name, from, to1, to2, rate, productDistance, weight1, weight2);
    };

    virtual Fusion *createFusion(const std::string &name, unsigned int from1, unsigned int from2,
                                 unsigned int to, const double rate, const double eductDistance,
                                 const double weight1 = 0.5, const double weight2 = 0.5) const {
        return new Fusion(name, from1, from2, to, rate, eductDistance, weight1, weight2);
    };

    template<typename T, typename... Args>
    struct get_dispatcher;

    template<typename T, typename... Args>
    struct get_dispatcher {
        static T *impl(const ReactionFactory *self, Args... args) {
            // this only invokes the normal constructor
            return new T(std::forward<Args>(args)...);
        };
    };
};


template<typename... Args>
struct ReactionFactory::get_dispatcher<Conversion, Args...> {
    static Conversion *impl(const ReactionFactory *self, Args &&... args) {
        return self->createConversion(std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct ReactionFactory::get_dispatcher<Enzymatic, Args...> {
    static Enzymatic *impl(const ReactionFactory *self, Args &&... args) {
        return self->createEnzymatic(std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct ReactionFactory::get_dispatcher<Fission, Args...> {
    static Fission *impl(const ReactionFactory *self, Args &&... args) {
        return self->createFission(std::forward<Args>(args)...);
    }
};

template<typename... Args>
struct ReactionFactory::get_dispatcher<Fusion, Args...> {
    static Fusion *impl(const ReactionFactory *self, Args &&... args) {
        return self->createFusion(std::forward<Args>(args)...);
    }
};

}
}
}

#endif //READDY_MAIN_REACTIONFACTORY_H