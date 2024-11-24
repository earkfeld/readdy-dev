
#include <pybind11/pybind11.h>
#include <readdy/kernel/cpu/CPUKernel.h>
#include <readdy/common/index_persistent_vector.h>
#include <readdy/model/actions/UserDefinedAction.h>
#include <readdy/model/topologies/reactions/StructuralTopologyReaction.h>
#include <readdy/model/actions/Utils.h>
#include <iostream>

namespace py = pybind11;
namespace rnd = readdy::model::rnd;

class SimpleAction : public readdy::model::actions::UserDefinedAction {
public:
    explicit SimpleAction(readdy::scalar timeStep) : UserDefinedAction(timeStep) {}

    void perform(const readdy::util::PerformanceNode &node) override {
        std::cout << "SimpleAction::perform with timestep: " << timeStep() << std::endl;
    }
};

PYBIND11_MODULE(simple_action, m) {
    py::module::import("readdy");
    py::class_<SimpleAction, readdy::model::actions::UserDefinedAction, std::shared_ptr<SimpleAction>>(m, "SimpleAction")
        .def(py::init<readdy::scalar>())
        .def("perform", &SimpleAction::perform);
}
