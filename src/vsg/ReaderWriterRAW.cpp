
#include "ReaderWriterRAW.hpp"

#include "io/BinaryReader.hpp"
#include "io/IFileSys.hpp"
#include "io/NamingKeyMap.hpp"

#include <vsg/core/Array2D.h>
#include <vsg/nodes/Group.h>

namespace ehb
{
    static constexpr uint32_t RAW_MAGIC = 0x52617069;
    static constexpr uint32_t RAW_FORMAT_8888 = 0x38383838;

    ReaderWriterRAW::ReaderWriterRAW(IFileSys& fileSys) :
        fileSys(fileSys) {}

    vsg::ref_ptr<vsg::Object> ReaderWriterRAW::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (auto fullFilePath = vsg::findFile(filename, options); !fullFilePath.empty())
        {
            if (auto file = fileSys.createInputStream(fullFilePath + ".raw"); file != nullptr) { return read(*file, options); }
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterRAW::read(std::istream& stream, vsg::ref_ptr<const vsg::Options>) const
    {
        // very light interface to read files for easier refactoring
        BinaryReader reader(stream);

        auto magic = reader.read<uint32_t>();
        auto format = reader.read<uint32_t>();
        auto flags = reader.read<uint16_t>();
        auto surfaceCount = reader.read<uint16_t>();
        auto width = reader.read<uint16_t>();
        auto height = reader.read<uint16_t>();

        if (magic != RAW_MAGIC)
        {
            // this isn't a raw file
            return {};
        }

        // TODO: handle the different possible formats available here
        if (format != RAW_FORMAT_8888)
        {
            // this is an unsupported format
            return {};
        }

        // with the 8888 format each pixel is 4 bytes
        const uint32_t size = static_cast<uint32_t>(width) * static_cast<uint32_t>(height) * 4;

        auto image = vsg::ubvec4Array2D::create(width, height, vsg::Data::Layout{VK_FORMAT_B8G8R8A8_UNORM});

        reader.readBytes(static_cast<char*>(image->dataPointer()), size);

        return image;
    }
} // namespace ehb
