# coding=utf-8

# Copyright © 2016 Computational Molecular Biology Group,
#                  Freie Universität Berlin (GER)
#
# This file is part of ReaDDy.
#
# ReaDDy is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General
# Public License along with this program. If not, see
# <http://www.gnu.org/licenses/>.

"""
@author: clonker
"""

import os
import unittest
import tempfile
import shutil
import h5py
import numpy as np
import readdy._internal.readdybinding.common as common
import readdy._internal.readdybinding.common.io as io
from readdy._internal.readdybinding.api import Simulation
from readdy.util.trajectory_utils import TrajectoryReader
from contextlib import closing


class TestSchemeApi(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.dir = tempfile.mkdtemp("test-io")

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.dir, ignore_errors=True)

    def test_write_trajectory(self):
        common.set_logging_level("error")
        traj_fname = os.path.join(self.dir, "traj.h5")
        simulation = Simulation()
        simulation.set_kernel("SingleCPU")
        simulation.box_size = common.Vec(5,5,5)
        simulation.register_particle_type("A", 0.0, 0.0)

        def callback(_):
            simulation.add_particle("A", common.Vec(0, 0, 0))

        simulation.register_observable_n_particles(1, ["A"], callback)
        traj_handle = simulation.register_observable_trajectory(0)
        with closing(io.File(traj_fname, io.FileAction.CREATE, io.FileFlag.OVERWRITE)) as f:
            traj_handle.enable_write_to_file(f, u"", 3)
            simulation.run_scheme_readdy(True).write_config_to_file(f).configure(1).run(20)

        r = TrajectoryReader(traj_fname)
        trajectory_items = r[:]
        for idx, items in enumerate(trajectory_items):
            np.testing.assert_equal(len(items), idx+1)
            for item in items:
                np.testing.assert_equal(item.t, idx)
                np.testing.assert_equal(item.position, np.array([.0, .0, .0]))
        with h5py.File(traj_fname) as f:
            np.testing.assert_equal("A", f["readdy/config/particle_types/0"].value)

        common.set_logging_level("debug")

    def test_write_trajectory_as_observable(self):
        common.set_logging_level("error")
        traj_fname = os.path.join(self.dir, "traj_as_obs.h5")
        simulation = Simulation()
        simulation.set_kernel("SingleCPU")
        simulation.box_size = common.Vec(5,5,5)
        simulation.register_particle_type("A", 0.0, 0.0)

        def callback(_):
            simulation.add_particle("A", common.Vec(0, 0, 0))

        simulation.register_observable_n_particles(1, ["A"], callback)
        traj_handle = simulation.register_observable_trajectory(1)

        with closing(io.File(traj_fname, io.FileAction.CREATE, io.FileFlag.OVERWRITE)) as f:
            traj_handle.enable_write_to_file(f, u"", int(3))
            simulation.run_scheme_readdy(True).configure(1).run(20)

        r = TrajectoryReader(traj_fname)
        trajectory_items = r[:]
        for idx, items in enumerate(trajectory_items):
            np.testing.assert_equal(len(items), idx+1)
            for item in items:
                np.testing.assert_equal(item.t, idx)
                np.testing.assert_equal(item.position, np.array([.0, .0, .0]))

        common.set_logging_level("debug")

    def test_readwrite_double_and_string(self):
        fname = os.path.join(self.dir, "test_readwrite_double_and_string.h5")
        data = np.array([[2.222, 3, 4, 5], [3.3, 3, 3, 3]], dtype=np.float64)
        with closing(io.File(fname, io.FileAction.CREATE)) as f:
            f.write_double("/sowas", data)
            f.write_string("/maeh", u"hierstehtwas")

        with h5py.File(fname, "r") as f2:
            np.testing.assert_equal(f2.get('/sowas'), data)
            np.testing.assert_equal(f2.get("/maeh").value.decode(), u"hierstehtwas")

    def test_groups_readwrite(self):
        fname = os.path.join(self.dir, "test_groups_readwrite.h5")
        data = np.array([[2.222, 3, 4, 5], [3.3, 3, 3, 3]], dtype=np.float64)
        with closing(io.File(fname, io.FileAction.CREATE)) as f:
            g = f.create_group("/my_super_group")
            subg = g.create_group("my_super_subgroup")
            g.write_double("doubleds", data)
            subg.write_string("stringds", u"jap")

        with h5py.File(fname, "r") as f2:
            np.testing.assert_equal(f2.get("/my_super_group")["doubleds"], data)
            np.testing.assert_equal(f2.get("/my_super_group").get("my_super_subgroup")["stringds"].value.decode(), u"jap")
            np.testing.assert_equal(f2.get("/my_super_group/my_super_subgroup")["stringds"].value.decode(), u"jap")

    def test_append(self):
        fname = os.path.join(self.dir, "test_append.h5")
        with closing(io.File(fname, io.FileAction.CREATE)) as f:
            g = f.create_group("/append_group")

            full_data = [[3.3, 2.2], [1, 1], [3.4, 2.4], [14, 14], [5.5, 5.5]]

            ds_double = io.DataSet_double("doubleds", g, [2, 2], [io.unlimited_dims(), 2])
            ds_double.append(np.array([[3.3, 2.2], [1, 1]], dtype=np.float64))
            ds_double.append(np.array([[3.4, 2.4], [14, 14], [5.5, 5.5]], dtype=np.float64))

        with h5py.File(fname, "r") as f2:
            np.testing.assert_equal(f2.get("append_group")["doubleds"][:], full_data)


if __name__ == '__main__':
    common.set_logging_level("debug")
    unittest.main()
