/**
 * << detailed description >>
 *
 * @file NeighborList.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 08.09.16
 */

#include <set>
#include <sstream>

#include <readdy/kernel/cpu/model/NeighborList.h>
#include <readdy/kernel/cpu/util/hilbert.h>
#include <readdy/common/numeric.h>
#include <future>
#include <readdy/kernel/cpu/util/scoped_thread.h>
#include <readdy/kernel/cpu/util/barrier.h>

template<typename T, typename Predicate>
typename std::vector<T>::iterator insert_sorted(std::vector<T> &vec, T const &item, Predicate pred) {
    return vec.insert(std::upper_bound(vec.begin(), vec.end(), item, pred), item);
}

namespace readdy {
namespace kernel {
namespace cpu {
namespace model {

template<typename T, typename Dims>
T get_contiguous_index(T i, T j, T k, Dims I, Dims J) {
    return k + j * J + i * I * J;
};

const static std::vector<NeighborList::neighbor_t> no_neighbors{};

NeighborList::NeighborList(const ctx_t *const context, data_t &data, util::Config const *const config,
                           skin_size_t skin)
        : ctx(context), config(config), cells(std::vector<Cell>()),
          simBoxSize(ctx->getBoxSize()), skin_size(skin), data(data) {}

void NeighborList::setupCells() {
    if (cells.empty()) {
        double maxCutoff = 0;
        for (auto &&e : ctx->getAllOrder2RegisteredPotentialTypes()) {
            for (auto &&p : ctx->getOrder2Potentials(std::get<0>(e), std::get<1>(e))) {
                maxCutoff = maxCutoff < p->getCutoffRadius() ? p->getCutoffRadius() : maxCutoff;
            }
        }
        for (auto &&e : ctx->getAllOrder2Reactions()) {
            maxCutoff = maxCutoff < e->getEductDistance() ? e->getEductDistance() : maxCutoff;
        }
        NeighborList::maxCutoff = maxCutoff + skin_size;
        if (maxCutoff > 0) {
            const auto desiredCellWidth = .5 * maxCutoff;

            for (unsigned short i = 0; i < 3; ++i) {
                nCells[i] = static_cast<cell_index>(floor(simBoxSize[i] / desiredCellWidth));
                if (nCells[i] == 0) nCells[i] = 1;
                cellSize[i] = simBoxSize[i] / static_cast<double>(nCells[i]);
            }

            for (cell_index i = 0; i < nCells[0]; ++i) {
                for (cell_index j = 0; j < nCells[1]; ++j) {
                    for (cell_index k = 0; k < nCells[2]; ++k) {
                        cells.push_back({i, j, k, nCells});
                    }
                }
            }

            for (cell_index i = 0; i < nCells[0]; ++i) {
                for (cell_index j = 0; j < nCells[1]; ++j) {
                    for (cell_index k = 0; k < nCells[2]; ++k) {
                        setupNeighboringCells(i, j, k);
                    }
                }
            }
        } else {
            nCells = {{1, 1, 1}};
            cells.push_back({0, 0, 0, nCells});
            setupNeighboringCells(0, 0, 0);
        }
    }
    if (cells.size() > 1) {
        // todo group particles by cells
        // todo sort particles wrt hilbert curve within cells
    }
}

void
NeighborList::setupNeighboringCells(const signed_cell_index i, const signed_cell_index j, const signed_cell_index k) {
    auto me = getCell(i, j, k);
    for (signed_cell_index _i = -2; _i < 3; ++_i) {
        for (signed_cell_index _j = -2; _j < 3; ++_j) {
            for (signed_cell_index _k = -2; _k < 3; ++_k) {
                // don't add me as neighbor to myself
                if (!(_i == 0 && _j == 0 && _k == 0)) {
                    me->addNeighbor(getCell(i + _i, j + _j, k + _k));
                }
            }
        }
    }
}

void NeighborList::clear() {
    cells.clear();
    for (auto &list : data.neighbors) {
        list.clear();
    }
    initialSetup = true;
}

void checkCell(NeighborList::Cell &cell, const NeighborList::data_t &data, const NeighborList::skin_size_t skin) {
    if (skin > 0) {
        for (const auto idx : cell.particleIndices) {
            const auto &entry = data.entry_at(idx);
            if (!entry.is_deactivated()) {
                const double disp = entry.displacement;
                if (disp > cell.maximal_displacements[0]) {
                    cell.maximal_displacements[0] = disp;
                } else if (disp > cell.maximal_displacements[1]) {
                    cell.maximal_displacements[1] = disp;
                }
            }
        }
        cell.checkDirty(skin);
    } else {
        cell.dirty = true;
    }
}

bool cellOrNeighborDirty(const NeighborList::Cell &cell) {
    return cell.dirty ||
           (std::find_if(cell.neighbors.begin(), cell.neighbors.end(), [](const NeighborList::Cell *const neighbor) {
               return neighbor->dirty;
           }) != cell.neighbors.end());
}

void NeighborList::fillCells() {
    if (maxCutoff > 0) {
        const auto c2 = maxCutoff * maxCutoff;
        auto d2 = ctx->getDistSquaredFun();

        if (initialSetup) {

            for (auto &cell : cells) {
                cell.particleIndices.clear();
            }
            data.neighbors.clear();
            data.neighbors.resize(data.size() + data.getNDeactivated());
            data_t::index_t idx = 0;

            for (const auto &entry : data) {
                if (!entry.is_deactivated()) {
                    auto cell = getCell(entry.pos);
                    if (cell) {
                        cell->particleIndices.push_back(idx);
                    }
                }
                ++idx;
            }

            using cell_it_t = decltype(cells.begin());
            const auto size = cells.size();
            const std::size_t grainSize = size / config->nThreads;

            auto worker = [this, c2, d2](cell_it_t cellsBegin, cell_it_t cellsEnd) -> void {
                for (auto _b = cellsBegin; _b != cellsEnd; ++_b) {
                    auto &cell = *_b;
                    setUpCell(cell, c2, d2);
                }
            };

            std::vector<util::scoped_thread> threads;
            threads.reserve(config->nThreads);

            auto it_cells = cells.begin();
            for (int i = 0; i < config->nThreads - 1; ++i) {
                threads.push_back(util::scoped_thread(std::thread(worker, it_cells, it_cells + grainSize)));
                it_cells += grainSize;
            }
            threads.push_back(util::scoped_thread(std::thread(worker, it_cells, cells.end())));
        } else {
            auto dirtyCells = findDirtyCells();
            const std::size_t grainSize = dirtyCells.size() / config->nThreads;

            using cell_it_t = decltype(dirtyCells.begin());

            auto worker = [this, c2, d2, grainSize](cell_it_t begin, cell_it_t end, const util::barrier &barrier) {
                // first gather updates
                std::vector<std::vector<NeighborList::particle_index>> cellUpdates;
                cellUpdates.reserve(grainSize);
                {
                    for (auto _b = begin; _b != end; ++_b) {
                        auto cell = *_b;
                        cellUpdates.push_back({});
                        auto &updatedIndices = cellUpdates.back();
                        updatedIndices.reserve(cell->particleIndices.size());
                        for (const auto idx : cell->particleIndices) {
                            const auto &entry = data.entry_at(idx);
                            if (!entry.is_deactivated() && cell->contiguous_index == getCellIndex(entry.position())) {
                                updatedIndices.push_back(idx);
                            }
                        }
                        for (const auto neigh : cell->neighbors) {
                            for (const auto idx : neigh->particleIndices) {
                                const auto &entry = data.entry_at(idx);
                                if (!entry.is_deactivated()
                                    && cell->contiguous_index == getCellIndex(entry.position())) {
                                    updatedIndices.push_back(idx);
                                }
                            }
                        }
                    }
                }
                // wait until all threads have their updates gathered
                barrier.wait();
                // apply updates
                {
                    auto it = begin;
                    for (auto &&update : cellUpdates) {
                        auto cell = *it;
                        cell->particleIndices = std::move(update);
                        ++it;
                    }
                }
                // wait until all threads have applied their updates
                barrier.wait();
                // update neighbor list in respective cells
                {
                    for (auto _b = begin; _b != end; ++_b) {
                        auto cell = *_b;
                        setUpCell(*cell, c2, d2);
                    }
                }
            };

            util::barrier b(config->nThreads);

            std::vector<util::scoped_thread> threads;
            threads.reserve(config->nThreads);

            log::console()->debug("got dirty cells {} vs total cells {}", dirtyCells.size(), cells.size());

            auto it_cells = dirtyCells.begin();
            for (int i = 0; i < config->nThreads - 1; ++i) {
                auto advanced = std::next(it_cells, grainSize);
                log::console()->debug("dist={}", std::distance(it_cells, advanced));
                threads.push_back(util::scoped_thread(std::thread(worker, it_cells, advanced, std::cref(b))));
                it_cells = advanced;
            }
            threads.push_back(util::scoped_thread(std::thread(worker, it_cells, dirtyCells.end(), std::cref(b))));
        }
    }
}

void NeighborList::create() {
    simBoxSize = ctx->getBoxSize();
    data.setFixPosFun(ctx->getFixPositionFun());
    setupCells();
    fillCells();
    initialSetup = false;
}

NeighborList::Cell *NeighborList::getCell(signed_cell_index i, signed_cell_index j, signed_cell_index k) {
    const auto &periodic = ctx->getPeriodicBoundary();
    if (periodic[0]) i = readdy::util::numeric::positive_modulo(i, nCells[0]);
    else if (i < 0 || i >= nCells[0]) return nullptr;
    if (periodic[1]) j = readdy::util::numeric::positive_modulo(j, nCells[1]);
    else if (j < 0 || j >= nCells[1]) return nullptr;
    if (periodic[2]) k = readdy::util::numeric::positive_modulo(k, nCells[2]);
    else if (k < 0 || k >= nCells[2]) return nullptr;
    const auto cix = get_contiguous_index(i, j, k, nCells[1], nCells[2]);
    if (cix < cells.size()) {
        return &cells.at(static_cast<cell_index>(cix));
    } else {
        log::console()->critical("NeighborList::getCell(nonconst): Requested cell ({},{},{})={}, but there are "
                                         "only {} cells.", i, j, k, cix, cells.size());
        throw std::runtime_error("tried to get cell index that was too large");
    }
}

void NeighborList::remove(const particle_index idx) {
    auto cell = getCell(data.pos(idx));
    if (cell != nullptr) {
        auto remove_predicate = [idx](const ParticleData::Neighbor &n) {
            return n.idx == idx;
        };
        try {
            for (auto &neighbor : neighbors(idx)) {
                auto &neighbors_2nd = data.neighbors.at(neighbor.idx);
                neighbors_2nd.erase(std::find_if(neighbors_2nd.begin(), neighbors_2nd.end(), remove_predicate));
            }
            auto find_it = std::find(cell->particleIndices.begin(), cell->particleIndices.end(), idx);
            if (find_it != cell->particleIndices.end()) {
                cell->particleIndices.erase(find_it);
            }
        } catch (const std::out_of_range &) {
            log::console()->error("tried to remove particle with id {} but it was not in the neighbor list", idx);
        }
    }
}

void NeighborList::insert(const particle_index idx) {
    const auto &d2 = ctx->getDistSquaredFun();
    const auto &pos = data.pos(idx);
    const auto cutoffSquared = maxCutoff * maxCutoff;
    auto cell = getCell(pos);
    if (cell) {
        cell->particleIndices.push_back(idx);
        try {
            auto &myNeighbors = data.neighbors.at(idx);
            for (const auto pJ : cell->particleIndices) {
                if (idx != pJ) {
                    const auto distSquared = d2(pos, data.pos(pJ));
                    if (distSquared < cutoffSquared) {
                        myNeighbors.push_back({pJ, distSquared});
                        data.neighbors.at(pJ).push_back({idx, distSquared});
                    }
                }
            }
            for (auto &neighboringCell : cell->neighbors) {
                for (const auto &pJ : neighboringCell->particleIndices) {
                    const auto distSquared = d2(pos, data.pos(pJ));
                    if (distSquared < cutoffSquared) {
                        myNeighbors.push_back({pJ, distSquared});
                        data.neighbors.at(pJ).push_back({idx, distSquared});
                    }
                }
            }
        } catch (const std::out_of_range &) {
            log::console()->error("this should not happen123");
        }
    } else {
        //log::console()->error("could not assign particle (index={}) to any cell!", data.getEntryIndex(idx));
    }
}

NeighborList::Cell *NeighborList::getCell(const readdy::model::Particle::pos_type &pos) {
    const cell_index i = static_cast<const cell_index>(floor((pos[0] + .5 * simBoxSize[0]) / cellSize[0]));
    const cell_index j = static_cast<const cell_index>(floor((pos[1] + .5 * simBoxSize[1]) / cellSize[1]));
    const cell_index k = static_cast<const cell_index>(floor((pos[2] + .5 * simBoxSize[2]) / cellSize[2]));
    return getCell(i, j, k);
}

void NeighborList::updateData(ParticleData::update_t update) {
    if (maxCutoff > 0) {
        for (const auto &p : std::get<1>(update)) {
            remove(p);
        }
    }

    auto newEntries = data.update(std::move(update));
    if (maxCutoff > 0) {
        for (const auto p : newEntries) {
            insert(p);
        }
    }
}

const std::vector<NeighborList::neighbor_t> &NeighborList::neighbors(NeighborList::particle_index const entry) const {
    if (maxCutoff > 0) {
        try {
            return data.neighbors.at(entry);
        } catch (const std::out_of_range &) {
            log::console()->error("this is garbage");
        }
    }
    return no_neighbors;
}

const NeighborList::Cell *const NeighborList::getCell(const readdy::model::Particle::pos_type &pos) const {
    const cell_index i = static_cast<const cell_index>(floor((pos[0] + .5 * simBoxSize[0]) / cellSize[0]));
    const cell_index j = static_cast<const cell_index>(floor((pos[1] + .5 * simBoxSize[1]) / cellSize[1]));
    const cell_index k = static_cast<const cell_index>(floor((pos[2] + .5 * simBoxSize[2]) / cellSize[2]));
    return getCell(i, j, k);
}

const NeighborList::Cell *const NeighborList::getCell(NeighborList::signed_cell_index i,
                                                      NeighborList::signed_cell_index j,
                                                      NeighborList::signed_cell_index k) const {

    const auto &periodic = ctx->getPeriodicBoundary();
    if (periodic[0]) i = readdy::util::numeric::positive_modulo(i, nCells[0]);
    else if (i < 0 || i >= nCells[0]) return nullptr;
    if (periodic[1]) j = readdy::util::numeric::positive_modulo(j, nCells[1]);
    else if (j < 0 || j >= nCells[1]) return nullptr;
    if (periodic[2]) k = readdy::util::numeric::positive_modulo(k, nCells[2]);
    else if (k < 0 || k >= nCells[2]) return nullptr;
    const auto cix = get_contiguous_index(i, j, k, nCells[1], nCells[2]);
    if (cix < cells.size()) {
        return &cells.at(static_cast<cell_index>(cix));
    } else {
        log::console()->critical("NeighborList::getCell(const): Requested cell ({},{},{})={}, but there are "
                                         "only {} cells.", i, j, k, cix, cells.size());
        throw std::out_of_range("tried to access an invalid cell");
    }
}

const std::vector<NeighborList::neighbor_t> &NeighborList::find_neighbors(particle_index const entry) const {
    if (maxCutoff > 0 && entry < data.neighbors.size()) {
        try {
            return data.neighbors.at(entry);
        } catch (const std::out_of_range &) {
            log::console()->error("this def should not happen!§§");
        }
    }
    return no_neighbors;
}

NeighborList::~NeighborList() = default;

void NeighborList::Cell::addNeighbor(NeighborList::Cell *cell) {
    if (cell && cell->contiguous_index != contiguous_index
        && (enoughCells || std::find(neighbors.begin(), neighbors.end(), cell) == neighbors.end())) {
        neighbors.push_back(cell);
    }
}

NeighborList::Cell::Cell(cell_index i, cell_index j, cell_index k,
                         const std::array<NeighborList::cell_index, 3> &nCells)
        : contiguous_index(get_contiguous_index(i, j, k, nCells[1], nCells[2])),
          enoughCells(nCells[0] >= 5 && nCells[1] >= 5 && nCells[2] >= 5), maximal_displacements{0, 0} {
}

bool operator==(const NeighborList::Cell &lhs, const NeighborList::Cell &rhs) {
    return lhs.contiguous_index == rhs.contiguous_index;
}

bool operator!=(const NeighborList::Cell &lhs, const NeighborList::Cell &rhs) {
    return !(lhs == rhs);
}

void NeighborList::Cell::checkDirty(skin_size_t skin) {
    dirty = maximal_displacements[0] + maximal_displacements[1] > skin;
}

NeighborList::hilbert_index_t NeighborList::getHilbertIndex(std::size_t i, std::size_t j, std::size_t k) const {
    bitmask_t coords[3]{i, j, k};
    return static_cast<unsigned int>(hilbert_c2i(3, CHAR_BIT, coords));
}

void NeighborList::displace(data_t::Entry &entry, const data_t::particle_type::pos_type &delta) {
    data.displace(entry, delta);
}

void NeighborList::displace(data_iter_t iter, const readdy::model::Vec3 &vec) {
    auto &entry = *iter;
    displace(entry, vec);
}

void NeighborList::displace(data_t::index_t idx, const data_t::particle_type::pos_type &delta) {
    auto &entry = data.entry_at(idx);
    displace(entry, delta);
}

void NeighborList::setPosition(data_t::index_t idx, data_t::particle_type::pos_type &&newPosition) {
    auto &entry = data.entry_at(idx);
    const auto delta = newPosition - entry.position();
    displace(entry, delta);
}

std::unordered_set<NeighborList::Cell *> NeighborList::findDirtyCells() {
    using it_type = decltype(cells.begin());

    using future_t = std::future<std::vector<Cell *>>;
    using promise_t = std::promise<std::vector<Cell *>>;

    std::unordered_set<NeighborList::Cell *> result;


    auto worker = [this](it_type begin, it_type end, promise_t update, const util::barrier& barrier) {

        for(it_type it = begin; it != end; ++it) {
            checkCell(*it, data, skin_size);
        }
        barrier.wait();

        std::vector<Cell *> foundDirtyCells;
        for (it_type it = begin; it != end; ++it) {
            if (cellOrNeighborDirty(*it)) {
                foundDirtyCells.push_back(&*it);
            }
        }
        update.set_value(std::move(foundDirtyCells));
    };
    util::barrier b(config->nThreads);

    std::vector<future_t> dirtyCells;
    dirtyCells.reserve(config->nThreads);
    {
        auto it_cells = cells.begin();
        std::vector<util::scoped_thread> threads;
        const std::size_t grainSize = cells.size() / config->nThreads;
        for (int i = 0; i < config->nThreads - 1; ++i) {
            promise_t promise;
            dirtyCells.push_back(promise.get_future());
            threads.push_back(
                    util::scoped_thread(std::thread(worker, it_cells, it_cells + grainSize, std::move(promise), std::cref(b)))
            );
            it_cells += grainSize;
        }
        promise_t promise;
        dirtyCells.push_back(promise.get_future());
        threads.push_back(util::scoped_thread(std::thread(worker, it_cells, it_cells + grainSize, std::move(promise), std::cref(b))));
    }
    for (auto &&dirties : dirtyCells) {
        auto v = std::move(dirties.get());
        result.insert(v.begin(), v.end());
    }
    // no move needed, is inlined
    return result;
}

void NeighborList::setUpCell(NeighborList::Cell &cell, const double cutoffSquared, const ctx_t::dist_squared_fun &d2) {
    cell.dirty = false;
    cell.maximal_displacements[0] = 0;
    cell.maximal_displacements[1] = 0;
    for (const auto &pI : cell.particleIndices) {
        auto &entry_i = data.entry_at(pI);
        entry_i.displacement = 0;
        auto &neighbors_i = data.neighbors_at(pI);
        neighbors_i.clear();
        for (const auto &pJ : cell.particleIndices) {
            if (pI != pJ) {
                const auto distSquared = d2(entry_i.position(), data.pos(pJ));
                if (distSquared < cutoffSquared) {
                    neighbors_i.push_back({pJ, distSquared});
                }
            }
        }
        for (const auto &neighboringCell : cell.neighbors) {
            for (const auto pJ : neighboringCell->particleIndices) {
                const auto distSquared = d2(entry_i.position(), data.pos(pJ));
                if (distSquared < cutoffSquared) {
                    neighbors_i.push_back({pJ, distSquared});
                }
            }
        }

    }
}

int NeighborList::getCellIndex(const readdy::model::Particle::pos_type &pos) const {
    cell_index i = static_cast<const cell_index>(floor((pos[0] + .5 * simBoxSize[0]) / cellSize[0]));
    cell_index j = static_cast<const cell_index>(floor((pos[1] + .5 * simBoxSize[1]) / cellSize[1]));
    cell_index k = static_cast<const cell_index>(floor((pos[2] + .5 * simBoxSize[2]) / cellSize[2]));
    const auto &periodic = ctx->getPeriodicBoundary();
    if (periodic[0]) i = readdy::util::numeric::positive_modulo(i, nCells[0]);
    else if (i < 0 || i >= nCells[0]) return -1;
    if (periodic[1]) j = readdy::util::numeric::positive_modulo(j, nCells[1]);
    else if (j < 0 || j >= nCells[1]) return -1;
    if (periodic[2]) k = readdy::util::numeric::positive_modulo(k, nCells[2]);
    else if (k < 0 || k >= nCells[2]) return -1;
    return get_contiguous_index(i, j, k, nCells[1], nCells[2]);
}

}
}
}
}
