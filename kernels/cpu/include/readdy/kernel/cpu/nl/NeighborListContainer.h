/********************************************************************
 * Copyright © 2017 Computational Molecular Biology Group,          * 
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
 * @file NeighborListContainer.h
 * @brief << brief description >>
 * @author clonker
 * @date 15.09.17
 * @copyright GNU Lesser General Public License v3.0
 */

#pragma once

#include <vector>
#include <readdy/common/thread/Config.h>
#include <readdy/kernel/cpu/data/DefaultDataContainer.h>
#include <readdy/common/flat_iterator.h>

namespace readdy {
namespace kernel {
namespace cpu {
namespace nl {

class NeighborList;

class NeighborListContainer {

public:
    using value_type = std::size_t;
    using local_index_vector = std::vector<value_type>;
    using index_vector = std::vector<local_index_vector>;
    using data_container_type = readdy::kernel::cpu::data::DefaultDataContainer;
    using thread_config_type = readdy::util::thread::Config;

    using const_iterator = readdy::util::flat_const_iterator<index_vector::const_iterator>;

    NeighborListContainer(const data_container_type &data, const thread_config_type &threadConfig);

    index_vector &elements() {
        return _elements;
    }

    const index_vector &elements() const {
        return _elements;
    }

    const_iterator begin() const;

    const_iterator end() const;

    virtual void update() = 0;

private:
    index_vector _elements;

    std::reference_wrapper<const data_container_type> _data;
    std::reference_wrapper<const thread_config_type> _config;
};

}
}
}
}