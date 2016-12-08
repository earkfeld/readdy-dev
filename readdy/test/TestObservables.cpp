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
 * @file TestObservables.cpp
 * @brief << brief description >>
 * @author clonker
 * @date 02.05.16
 */

#include <readdy/plugin/KernelProvider.h>
#include <readdy/Simulation.h>
#include <readdy/testing/KernelTest.h>
#include <readdy/testing/Utils.h>

namespace m = readdy::model;

namespace {
class TestObservables : public KernelTest {

};

TEST_P(TestObservables, TestParticlePositions) {
    const unsigned int n_particles = 100;
    const double diffusionConstant = 1;
    kernel->getKernelContext().setDiffusionConstant("type", diffusionConstant);
    const double timeStep = 1.0;
    kernel->getKernelContext().setTimeStep(timeStep);
    const auto particleTypeId = kernel->getKernelContext().getParticleTypeID("type");
    const auto particles = std::vector<m::Particle>(n_particles, m::Particle(0, 0, 0, particleTypeId));
    kernel->getKernelStateModel().addParticles(particles);
    auto &&obs = kernel->createObservable<m::observables::ParticlePosition>(3);
    auto &&connection = kernel->connectObservable(obs.get());

    auto &&integrator = kernel->createProgram("EulerBDIntegrator");
    auto &&neighborList = kernel->createProgram<readdy::model::programs::UpdateNeighborList>();
    for (readdy::model::observables::time_step_type t = 0; t < 100; t++) {
        integrator->execute();
        neighborList->execute();
        kernel->evaluateObservables(t);
    }

    const auto &result = obs->getResult();
    const auto &&positions = kernel->getKernelStateModel().getParticlePositions();
    auto it_pos = positions.begin();
    int j = 0;
    for (auto it = result.begin(); it != result.end(); it = std::next(it)) {
        EXPECT_EQ(*it, *it_pos);
        it_pos++;
        ++j;
    }
    EXPECT_TRUE(j == 100);
    connection.disconnect();
}

TEST_P(TestObservables, TestCombinerObservable) {
    auto &&o1 = kernel->createObservable<m::observables::ParticlePosition>(1);
    auto &&o2 = kernel->createObservable<m::observables::ParticlePosition>(1);
    auto &&o3 = kernel->createObservable<m::observables::TestCombiner>(o1.get(), o2.get());
    auto &&connection = kernel->connectObservable(o3.get());
    auto &&integrator = kernel->createProgram("EulerBDIntegrator");
    kernel->getKernelStateModel().updateNeighborList();
    for (readdy::model::observables::time_step_type t = 0; t < 100; t++) {
        integrator->execute();
        kernel->getKernelStateModel().updateNeighborList();
    }

    const auto &result = o3->getResult();
    for (auto &&p : result) {
        // todo
    }

    connection.disconnect();
}

TEST_P(TestObservables, TestForcesObservable) {
    // Setup particles
    kernel->getKernelContext().setDiffusionConstant("A", 42.);
    kernel->getKernelContext().setDiffusionConstant("B", 1337.);
    const auto typeIdA = kernel->getKernelContext().getParticleTypeID("A");
    const auto typeIdB = kernel->getKernelContext().getParticleTypeID("B");
    const unsigned int n_particles = 50; // There will be 55 Bs
    const auto particlesA = std::vector<m::Particle>(n_particles, m::Particle(0, 0, 0, typeIdA));
    const auto particlesB = std::vector<m::Particle>(n_particles + 5, m::Particle(0, 0, 0, typeIdB));
    kernel->getKernelStateModel().addParticles(particlesA);
    kernel->getKernelStateModel().addParticles(particlesB);
    {
        // Check if result has correct size
        // Check that empty particleType argument gives correct object, namely all forces
        auto &&obsA = kernel->createObservable<m::observables::Forces>(1, std::vector<std::string>{"A"});
        auto &&obsB = kernel->createObservable<m::observables::Forces>(1, std::vector<std::string>{"B"});
        auto &&obsBoth = kernel->createObservable<m::observables::Forces>(1);
        auto &&connectionA = kernel->connectObservable(obsA.get());
        auto &&connectionB = kernel->connectObservable(obsB.get());
        auto &&connectionBoth = kernel->connectObservable(obsBoth.get());
        // Evaluate twice to ensure that results do not accumulate
        kernel->evaluateObservables(0);
        kernel->evaluateObservables(1);
        const auto &resA = obsA->getResult();
        const auto &resB = obsB->getResult();
        const auto &resBoth = obsBoth->getResult();
        EXPECT_EQ(resA.size(), 50);
        EXPECT_EQ(resB.size(), 55);
        EXPECT_EQ(resBoth.size(), 105);
        m::Vec3 zero = m::Vec3(0, 0, 0);
        for (auto force : resBoth) {
            EXPECT_TRUE(force == zero);
        }
    }
    // Two particles C and C with radius 1 and harmonic repulsion at distance 1.5 -> force = kappa * (radiiSum - 1.5)
    kernel->getKernelContext().setPeriodicBoundary(false, false, false);
    kernel->getKernelContext().setBoxSize(5, 5, 5);
    kernel->getKernelContext().setDiffusionConstant("C", 1.);
    kernel->getKernelContext().setParticleRadius("C", 1.);
    const auto typeIdC = kernel->getKernelContext().getParticleTypeID("C");
    const auto particlesC = std::vector<m::Particle>{m::Particle(0, 0, 0, typeIdC), m::Particle(0, -1.5, 0, typeIdC)};
    kernel->getKernelStateModel().addParticles(particlesC);

    auto harmonicRepulsion = kernel->createPotentialAs<readdy::model::potentials::HarmonicRepulsion>();
    harmonicRepulsion->setForceConstant(2.);
    kernel->getKernelContext().registerPotential(std::move(harmonicRepulsion), "C", "C");

    auto &&nl = kernel->createProgram<readdy::model::programs::UpdateNeighborList>();
    auto &&forces = kernel->createProgram<readdy::model::programs::CalculateForces>();
    kernel->getKernelContext().configure();
    {
        auto obsC = kernel->createObservable<m::observables::Forces>(1, std::vector<std::string>{"C"});
        auto connectionC = kernel->connectObservable(obsC.get());
        nl->execute();
        forces->execute();
        kernel->evaluateObservables(2);
        const auto &resC = obsC->getResult();
        m::Vec3 force0 = m::Vec3(0., 1., 0.);
        m::Vec3 force1 = m::Vec3(0., -1., 0.);
        EXPECT_EQ(resC.size(), 2);
        EXPECT_TRUE(resC[0] == force0 || resC[1] == force0);
        EXPECT_TRUE(resC[1] == force1 || resC[0] == force1);
    }
}

INSTANTIATE_TEST_CASE_P(TestObservables, TestObservables,
                        ::testing::ValuesIn(readdy::testing::getKernelsToTest()));
}
