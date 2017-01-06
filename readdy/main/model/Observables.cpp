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
 * Core library implementation of observables. Since every kernel usually has its own implementation, there are mostly constructors here.
 *
 * @file Observables.cpp
 * @brief Implementation of observables
 * @author clonker
 * @author chrisfroe
 * @date 26.04.16
 */

#include <readdy/model/observables/Observables.h>
#include <readdy/model/observables/io/AccumulativeWriter.h>
#include <readdy/model/Kernel.h>
#include <readdy/model/_internal/Util.h>
#include <readdy/common/numeric.h>
#include <readdy/io/DataSet.h>

const std::string OBSERVABLES_GROUP_PATH = "/readdy/observables";

namespace readdy {
namespace model {
namespace observables {

class Vec3MemoryType : public readdy::io::DataSetType {
public:
    Vec3MemoryType() {
        using entry_t = readdy::model::Vec3;
        tid = []() -> hid_t {
            hid_t entryTypeMemory = H5Tcreate(H5T_COMPOUND, sizeof(entry_t));
            // init vec pod
            io::NativeDataSetType<entry_t::entry_t> posType{};
            H5Tinsert(entryTypeMemory, "x", 0, posType.tid);
            H5Tinsert(entryTypeMemory, "y", sizeof(entry_t::entry_t), posType.tid);
            H5Tinsert(entryTypeMemory, "z", 2*sizeof(entry_t::entry_t), posType.tid);
            return entryTypeMemory;
        }();
    }
};

class Vec3FileType : public readdy::io::DataSetType {
public:
    Vec3FileType() {
        using entry_t = readdy::model::Vec3;
        tid = []() -> hid_t {
            std::size_t size = 3 * sizeof(entry_t::entry_t);
            hid_t entryTypeFile = H5Tcreate(H5T_COMPOUND, size);
            // data types
            io::STDDataSetType<entry_t::entry_t> posType{};
            // init trajectory pod
            std::size_t cumsize = 0;
            H5Tinsert(entryTypeFile, "px", cumsize, posType.tid);
            cumsize += sizeof(entry_t::entry_t);
            H5Tinsert(entryTypeFile, "py", cumsize, posType.tid);
            cumsize += sizeof(entry_t::entry_t);
            H5Tinsert(entryTypeFile, "pz", cumsize, posType.tid);
            return entryTypeFile;
        }();
    }
};


struct Positions::Impl {
    using writer_t = AccumulativeWriter<io::DataSet<readdy::model::Vec3, true>, Positions::result_t>;
    std::unique_ptr<writer_t> dataSet;
};

Positions::Positions(Kernel *const kernel, unsigned int stride,
                     std::vector<std::string> typesToCount) :
        Positions(kernel, stride,
                  _internal::util::transformTypes2(typesToCount, kernel->getKernelContext())) {}

Positions::Positions(Kernel *const kernel, unsigned int stride,
                     std::vector<unsigned int> typesToCount) :
        Observable(kernel, stride), typesToCount(typesToCount), pimpl(std::make_unique<Impl>()) {}

void Positions::append() {
    pimpl->dataSet->append(result);
}

Positions::Positions(Kernel *const kernel, unsigned int stride) : Observable(kernel, stride) { }

void Positions::initializeDataSet(io::File &file, const std::string &dataSetName, unsigned int flushStride) {
    if(!pimpl->dataSet) {
        std::vector<readdy::io::h5::dims_t> fs = {flushStride};
        std::vector<readdy::io::h5::dims_t> dims = {readdy::io::h5::UNLIMITED_DIMS};
        auto dataSet = std::make_unique<io::DataSet<readdy::model::Vec3, true>>(
                dataSetName, file.createGroup(OBSERVABLES_GROUP_PATH), fs, dims,
                Vec3MemoryType(), Vec3FileType()
        );
        pimpl->dataSet = std::make_unique<Impl::writer_t>(flushStride, std::move(dataSet));
    }
}

Positions::~Positions() = default;

void TestCombiner::evaluate() {
    std::vector<double> result;
    const auto &r1 = std::get<0>(parentObservables)->getResult();
    const auto &r2 = std::get<1>(parentObservables)->getResult();

    auto b1 = r1.begin();
    auto b2 = r2.begin();

    for (; b1 != r1.end();) {
        result.push_back((*b1) * (*b2));
        ++b1;
        ++b2;
    }

    TestCombiner::result = result;
}

RadialDistribution::RadialDistribution(Kernel *const kernel, unsigned int stride,
                                       std::vector<double> binBorders, std::vector<unsigned int> typeCountFrom,
                                       std::vector<unsigned int> typeCountTo, double particleToDensity)
        : Observable(kernel, stride), typeCountFrom(typeCountFrom), typeCountTo(typeCountTo),
          particleToDensity(particleToDensity) {
    setBinBorders(binBorders);
}

void RadialDistribution::evaluate() {
    if (binBorders.size() > 1) {
        std::fill(counts.begin(), counts.end(), 0);
        const auto particles = kernel->getKernelStateModel().getParticles();
        auto isInCollection = [](const readdy::model::Particle &p, const std::vector<unsigned int> &collection) {
            return std::find(collection.begin(), collection.end(), p.getType()) != collection.end();
        };
        const auto nFromParticles = std::count_if(particles.begin(), particles.end(),
                                                  [this, isInCollection](const readdy::model::Particle &p) {
                                                      return isInCollection(p, typeCountFrom);
                                                  });
        {
            const auto &distSquared = kernel->getKernelContext().getDistSquaredFun();
            for (auto &&pFrom : particles) {
                if (isInCollection(pFrom, typeCountFrom)) {
                    for (auto &&pTo : particles) {
                        if (isInCollection(pTo, typeCountTo) && pFrom.getId() != pTo.getId()) {
                            const auto dist = sqrt(distSquared(pFrom.getPos(), pTo.getPos()));
                            auto upperBound = std::upper_bound(binBorders.begin(), binBorders.end(), dist);
                            if (upperBound != binBorders.end()) {
                                const auto binBordersIdx = upperBound - binBorders.begin();
                                counts[binBordersIdx - 1]++;
                            }
                        }
                    }
                }
            }
        }

        auto &radialDistribution = std::get<1>(result);
        {
            const auto &binCenters = std::get<0>(result);
            auto &&it_centers = binCenters.begin();
            auto &&it_distribution = radialDistribution.begin();
            for (auto &&it_counts = counts.begin(); it_counts != counts.end(); ++it_counts) {
                const auto idx = it_centers - binCenters.begin();
                const auto lowerRadius = binBorders[idx];
                const auto upperRadius = binBorders[idx + 1];
                *it_distribution =
                        (*it_counts) /
                        (4 / 3 * util::numeric::pi() * (std::pow(upperRadius, 3) - std::pow(lowerRadius, 3)) * nFromParticles * particleToDensity);
                ++it_distribution;
                ++it_centers;
            }
        }
    }
}

const std::vector<double> &RadialDistribution::getBinBorders() const {
    return binBorders;
}

void RadialDistribution::setBinBorders(const std::vector<double> &binBorders) {
    if (binBorders.size() > 1) {
        RadialDistribution::binBorders = binBorders;
        auto nCenters = binBorders.size() - 1;
        result = std::make_pair(std::vector<double>(nCenters), std::vector<double>(nCenters));
        counts = std::vector<double>(nCenters);
        auto &binCenters = std::get<0>(result);
        auto it_begin = binBorders.begin();
        auto it_begin_next = it_begin + 1;
        size_t idx = 0;
        while (it_begin_next != binBorders.end()) {
            binCenters[idx++] = (*it_begin + *it_begin_next) / 2;
            ++it_begin;
            ++it_begin_next;
        }
    } else {
        log::console()->warn("Argument bin borders' size should be at least two to make sense.");
    }

}

RadialDistribution::RadialDistribution(Kernel *const kernel, unsigned int stride,
                                       std::vector<double> binBorders,
                                       const std::vector<std::string> &typeCountFrom,
                                       const std::vector<std::string> &typeCountTo, double particleToDensity)
        : RadialDistribution(kernel, stride, binBorders,
                             _internal::util::transformTypes2(typeCountFrom, kernel->getKernelContext()),
                             _internal::util::transformTypes2(typeCountTo, kernel->getKernelContext()),
                             particleToDensity
) {}

CenterOfMass::CenterOfMass(readdy::model::Kernel *const kernel, unsigned int stride,
                           unsigned int particleType)
        : Observable(kernel, stride), particleTypes({particleType}) {
}

CenterOfMass::CenterOfMass(Kernel *const kernel, unsigned int stride,
                           const std::string &particleType)
        : CenterOfMass(kernel, stride, kernel->getKernelContext().getParticleTypeID(particleType)) {}

void CenterOfMass::evaluate() {
    readdy::model::Vec3 com{0, 0, 0};
    unsigned long n_particles = 0;
    for (auto &&p : kernel->getKernelStateModel().getParticles()) {
        if (particleTypes.find(p.getType()) != particleTypes.end()) {
            ++n_particles;
            com += p.getPos();
        }
    }
    com /= n_particles;
    result = com;
}

CenterOfMass::CenterOfMass(Kernel *const kernel, unsigned int stride,
                           const std::vector<unsigned int> &particleTypes)
        : Observable(kernel, stride), particleTypes(particleTypes.begin(), particleTypes.end()) {
}

CenterOfMass::CenterOfMass(Kernel *const kernel, unsigned int stride,
                           const std::vector<std::string> &particleType) : Observable(kernel,
                                                                                      stride),
                                                                           particleTypes() {
    for (auto &&pt : particleType) {
        particleTypes.emplace(kernel->getKernelContext().getParticleTypeID(pt));
    }

}

NParticles::NParticles(Kernel *const kernel, unsigned int stride,
                       std::vector<std::string> typesToCount)
        : NParticles(kernel, stride,
                     _internal::util::transformTypes2(typesToCount, kernel->getKernelContext())) {

}

NParticles::NParticles(Kernel *const kernel, unsigned int stride,
                       std::vector<unsigned int> typesToCount)
        : Observable(kernel, stride), typesToCount(typesToCount) {
}

Forces::Forces(Kernel *const kernel, unsigned int stride, std::vector<std::string> typesToCount)
        : Forces(kernel, stride,
                 _internal::util::transformTypes2(typesToCount, kernel->getKernelContext())) {}

Forces::Forces(Kernel *const kernel, unsigned int stride, std::vector<unsigned int> typesToCount)
        : Observable(kernel, stride), typesToCount(typesToCount) {}

HistogramAlongAxis::HistogramAlongAxis(readdy::model::Kernel *const kernel, unsigned int stride,
                                       std::vector<double> binBorders,
                                       std::set<unsigned int> typesToCount, unsigned int axis)
        : Observable(kernel, stride), binBorders(binBorders), typesToCount(typesToCount), axis(axis) {
    auto nCenters = binBorders.size() - 1;
    result = std::vector<double>(nCenters);
}


HistogramAlongAxis::HistogramAlongAxis(Kernel *const kernel, unsigned int stride,
                                       std::vector<double> binBorders,
                                       std::vector<std::string> typesToCount,
                                       unsigned int axis)
        : HistogramAlongAxis(kernel, stride, binBorders,
                             _internal::util::transformTypes(typesToCount, kernel->getKernelContext()),
                             axis) {

}


}
}
}
