SET(SOURCES_DIR "${READDY_GLOBAL_DIR}/wrappers/python/src/cxx")

list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/ReaddyBinding.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/common.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/CommonModule.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/ExportIO.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/ReadableReactionRecord.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/ReadableReactionRecord.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/ReadableParticle.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/SpdlogPythonSink.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/common/Utils.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/ExportObservables.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/PyTopology.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/ExportLoopApi.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/ExportTopologies.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/ExportKernelContext.cpp")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/PyFunction.h")
list(APPEND READDY_BINDING_SOURCES "${SOURCES_DIR}/api/ApiModule.cpp")

list(APPEND READDY_ALL_SOURCES ${READDY_BINDING_SOURCES})