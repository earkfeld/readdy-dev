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
 * @file KernelMock.h
 * @brief << brief description >>
 * @author clonker
 * @date 13.07.16
 */

#ifndef READDY_MAIN_KERNELMOCK_H
#define READDY_MAIN_KERNELMOCK_H

#include <readdy/model/Kernel.h>
#include <gmock/gmock.h>

namespace readdy {
namespace testing {
class KernelMock : public readdy::model::Kernel {

public:
    KernelMock(const std::string &name) : Kernel(name) {}

    MOCK_CONST_METHOD0(getProgramFactory, readdy::model::programs::ProgramFactory & (void));

    MOCK_CONST_METHOD0(getKernelStateModel, readdy::model::KernelStateModel & (void));

    MOCK_CONST_METHOD0(getKernelContext, readdy::model::KernelContext & (void));

    MOCK_CONST_METHOD0(getPotentialFactory, readdy::model::potentials::PotentialFactory & (void));

    MOCK_CONST_METHOD0(getReactionFactory, readdy::model::reactions::ReactionFactory & (void));

};
}
}
#endif //READDY_MAIN_KERNELMOCK_H