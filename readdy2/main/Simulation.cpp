//
// Created by Moritz Hoffmann on 18/02/16.
//
#include <Simulation.h>

using namespace ReaDDy;

double Simulation::getKBT() const {
    return this->kBT;
}

void Simulation::setKBT(double kBT) {
    this->kBT = kBT;
}

void Simulation::setBoxSize(double dx, double dy, double dz) {
    box_size[0] = dx;
    box_size[1] = dy;
    box_size[2] = dz;
}

void Simulation::setPeriodicBoundary(bool pb_x, bool pb_y, bool pb_z) {
    periodic_boundary[0] = pb_x;
    periodic_boundary[1] = pb_y;
    periodic_boundary[2] = pb_z;
}

#ifdef READDY_WITH_PYTHON
#include <boost/python>

BOOST_PYTHON_MODULE(simulation) {
        using namespace boost::python;
        class_<Simulation>("Simulation")
            .def("getKBT", &Simulation::getKBT)
            .def("setKBT", &Simulation::setKBT);
}
#endif

