SET(SOURCES_DIR "${READDY_GLOBAL_DIR}/readdy2/main/model")

SET(MODEL_INCLUDE_DIRS "${COMMON_INCLUDE_DIRS}" CACHE INTERNAL "Model include dirs")

# libraries
SET(READDY_MODEL_LIBRARIES "${READDY_COMMON_LIBRARIES}")

# includes
SET(MODEL_INCLUDE_DIRS "${COMMON_INCLUDE_DIRS}" CACHE INTERNAL "Model include dirs" FORCE)

# sources
LIST(APPEND READDY_MODEL_SOURCES "${SOURCES_DIR}/KernelContext.cpp")
LIST(APPEND READDY_MODEL_SOURCES "${SOURCES_DIR}/KernelStateModel.cpp")
LIST(APPEND READDY_MODEL_SOURCES "${SOURCES_DIR}/Particle.cpp")
LIST(APPEND READDY_MODEL_SOURCES "${SOURCES_DIR}/RandomProvider.cpp")
LIST(APPEND READDY_MODEL_SOURCES "${SOURCES_DIR}/Vec3.cpp")

# all sources
LIST(APPEND READDY_ALL_SOURCES ${READDY_MODEL_SOURCES})