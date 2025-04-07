# Roadmap


## Known Bugs/Issues
- [ ] Observables for specific species or saving to specific files
- [ ] Reactive particle with fixed position at origin if reaction can/does occur at t=0

## Packaging
- [x] Update to use Conan2 for cpp deps
- [ ] Use py-build-cmake tools for compiling and building pip wheels
- [ ] Add to pip package registry

## Features
- [ ] Python
  - [ ] Make custom loops not experimental
  - [ ] Move active transport module to C++ backend
- [ ] Reactions
  - [ ] Rename action-reaction
  - [ ] Add optional time-dependence for reaction triggers
  - [ ] Reaction groups (to prevent unwanted reaction conflicts)
  - [ ] Same timestep reactions? (for reactions that should be single-step but are consecutive due to interface)
    - [ ] Use flag(s) for this?
- [ ] Representations
  - [ ] Membrane model
  - [ ] Set up dynamic particle radii
  - [ ] Create data structures to give particles an internal state
- [ ] RL Interface
  - [ ] Scoped observables (state observations)
    - [ ] Per particle/topology observables
    - [ ] Temporal striding (e.g. memory for only last n steps, or observe every n steps)
    - [ ] Spatially defined (i.e. cutoff radius for localizing observables)
  - [ ] Generalized Action Factory (particle/topology actions)
    - [ ] position updates
    - [ ] force updates
    - [ ] radius updates
    - [ ] internal state updates
    - [ ] topology updates (if applicable)
- [ ] Kernels
  - [ ] MPI
  - [ ] GPU (MD-GFRD Integrator)


## Miscellaneous
- [ ] Rename action-reaction
- [ ] Add tests for action-reaction (or whatever it gets renamed to)
- [ ] Optimize using second neighbor list for topologies
- [ ] Consistent param names in python bindings

