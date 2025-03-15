/**
 * << detailed description >>
 *
 * @file Blosc2Filter.cpp
 * @brief << brief description >>
 * @author earkfeld
 * @date 01.05.25
 */


#include "readdy/io/Blosc2Filter.h"
#include "blosc2_filter.h"

namespace readdy::io {

    bool Blosc2Filter::available() const {
        return H5Zfilter_avail(FILTER_BLOSC2) > 0;
    }

    void Blosc2Filter::activate(h5rd::PropertyList &plist) {
        registerFilter();
        unsigned int cd_values[7];
        // compression level 0-9 (0 no compression, 9 highest compression)
        cd_values[4] = compressionLevel;
        // 0: shuffle not active, 1: shuffle active
        cd_values[5] = shuffle ? 1 : 0;
        // the compressor to use
        switch (compressor) {
            case BloscLZ: {
                cd_values[6] = BLOSC_BLOSCLZ;
                break;
            }
            case LZ4: {
                cd_values[6] = BLOSC_LZ4;
                break;
            }
            case LZ4HC: {
                cd_values[6] = BLOSC_LZ4HC;
                break;
            }
            case ZLIB: {
                cd_values[6] = BLOSC_ZLIB;
                break;
            }
            case ZSTD: {
                cd_values[6] = BLOSC_ZSTD;
                break;
            }
        }
        if (H5Pset_filter(plist.id(), FILTER_BLOSC2, H5Z_FLAG_OPTIONAL, 7, cd_values) < 0) {
            H5Eprint(H5Eget_current_stack(), stderr);
            throw h5rd::Exception("Could not set blosc filter!");
        }
    }

    void Blosc2Filter::registerFilter() {
        static std::atomic_bool initialized{false};
        if (!initialized.load()) {
            char *version, *date;
            register_blosc2(&version, &date);
            log::debug("registered blosc with version {} ({})", version, date);
            initialized = true;
        }
    }

    Blosc2Filter::Blosc2Filter(Blosc2Filter::Compressor compressor, unsigned int compressionLevel, bool shuffle) : compressor(
            compressor), compressionLevel(compressionLevel), shuffle(shuffle) {
        if (compressionLevel > 9) {
            throw std::invalid_argument("Blosc only allows compression levels ranging from 0 to 9.");
        }
    }
}