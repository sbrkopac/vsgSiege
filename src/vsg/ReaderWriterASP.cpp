
#include "ReaderWriterASP.hpp"

#include "io/BinaryReader.hpp"
#include "io/LocalFileSys.hpp"

namespace ehb
{
    ReaderWriterASP::ReaderWriterASP(IFileSys& fileSys) :
        fileSys(fileSys) { log = spdlog::get("log"); }

    vsg::ref_ptr<vsg::Object> ReaderWriterASP::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (auto fullFilePath = vsg::findFile(filename, options); !fullFilePath.empty())
        {
            if (auto file = fileSys.createInputStream(fullFilePath + ".asp"); file != nullptr) { return read(*file, options); }
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterASP::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        BinaryReader reader(stream);

        FourCC chunk;
        while (reader.readFourCC(chunk))
        {
            if (chunk == "BMSH")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BONH")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BSUB")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BSMM")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BSMM")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BVTX")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BCRN")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "WCRN")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BVMP")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BTRI")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "RPOS")
            {
                log->info("Reading {}", chunk);
            }
            else if (chunk == "BEND")
            {
                log->info("Reading {}", chunk);
            }
        }

        return {};
    };
} // namespace ehb