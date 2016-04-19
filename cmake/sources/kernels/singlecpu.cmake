SET(SOURCES_DIR "${READDY_GLOBAL_DIR}/kernels/singlecpu")

# sources
LIST(APPEND SINGLECPU_SOURCES "${SOURCES_DIR}/SingleCPUKernel.cpp")
LIST(APPEND SINGLECPU_SOURCES "${SOURCES_DIR}/SingleCPUProgramFactory.cpp")

# headers
LIST(APPEND SINGLECPU_HEADERS "${SOURCES_DIR}/SingleCPUKernel.h")
LIST(APPEND SINGLECPU_HEADERS "${SOURCES_DIR}/SingleCPUProgramFactory.h")

# --- programs ---
LIST(APPEND SINGLECPU_SOURCES "${SOURCES_DIR}/programs/SingleCPUTestProgram.cpp")
LIST(APPEND SINGLECPU_HEADERS "${SOURCES_DIR}/programs/SingleCPUTestProgram.h")