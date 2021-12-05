
#include "ReaderWriterASP.hpp"

#include "io/BinaryReader.hpp"
#include "io/LocalFileSys.hpp"

#include "world/AspectImpl.hpp"

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

        std::shared_ptr<Aspect::Impl> aspectImpl = std::make_shared<Aspect::Impl>();

        // don't initialize this as it will get initialized by the loader when it hits that part of the file
        uint32_t currentSubMeshIndex;

        FourCC chunk;
        while (reader.readFourCC(chunk))
        {
            if (chunk == "BMSH")
            {
                log->info("Reading {}", chunk);

                // version
                reader.skipBytes(4);

                aspectImpl->sizeTextField = reader.read<uint32_t>();
                aspectImpl->boneCount = reader.read<uint32_t>();
                aspectImpl->textureCount = reader.read<uint32_t>();
                aspectImpl->vertexCount = reader.read<uint32_t>();
                aspectImpl->subMeshCount = reader.read<uint32_t>();
                aspectImpl->renderFlags = reader.read<uint32_t>();

                log->debug("asp mesh has {} textures", aspectImpl->textureCount);

                ByteArray rawText(aspectImpl->sizeTextField);
                reader.readBytes(rawText.data(), rawText.size());

                size_t index = 0;
                aspectImpl->textureNames.resize(aspectImpl->textureCount);
                for (uint32_t i = 0; i < aspectImpl->textureCount; ++i)
                {
                    for (; index < rawText.size(); ++index)
                    {
                        const char c = static_cast<char>(rawText[index]);
                        if (c == '\0')
                        {
                            // Skip null padding for the next name:
                            while (index < rawText.size() && rawText[index] == 0)
                            {
                                ++index;
                            }
                            break;
                        }
                        aspectImpl->textureNames[i].push_back(c);
                    }

                    log->debug("textureNames[{}] = {}", i, aspectImpl->textureNames[i]);
                }

                aspectImpl->boneInfos.resize(aspectImpl->boneCount);
                for (uint32_t b = 0; b < aspectImpl->boneCount; ++b)
                {
                    for (; index < rawText.size(); ++index)
                    {
                        const char c = static_cast<char>(rawText[index]);
                        if (c == '\0')
                        {
                            // Skip null padding for the next name:
                            while (index < rawText.size() && rawText[index] == 0)
                            {
                                ++index;
                            }
                            break;
                        }
                        aspectImpl->boneInfos[b].name.push_back(c);
                    }
                }

                aspectImpl->subMeshes.resize(aspectImpl->subMeshCount);

            }
            else if (chunk == "BONH")
            {
                // version
                reader.skipBytes(4);

                for (size_t b = 0; b < aspectImpl->boneInfos.size(); ++b)
                {
                    const auto boneIndex = reader.read<uint32_t>();
                    const auto parentIndex = reader.read<uint32_t>();
                    const auto boneFlags = reader.read<uint32_t>();

                    aspectImpl->boneInfos[boneIndex].parentIndex = parentIndex;
                    aspectImpl->boneInfos[boneIndex].flags = boneFlags;
                }
            }
            else if (chunk == "BSUB")
            {
                const auto version = reader.read<uint32_t>();

                currentSubMeshIndex = reader.read<uint32_t>();

                if (Aspect::Impl::versionOf(version) <= 40)
                {
                    currentSubMeshIndex += 1;
                }

                Aspect::Impl::SubMesh& mesh = aspectImpl->subMeshes[currentSubMeshIndex];

                mesh.textureCount = reader.read<uint32_t>();
                mesh.vertexCount = reader.read<uint32_t>();
                mesh.cornerCount = reader.read<uint32_t>();
                mesh.faceCount = reader.read<uint32_t>();
            }
            else if (chunk == "BSMM")
            {
                reader.skipBytes(4);

                auto& mesh = aspectImpl->subMeshes[currentSubMeshIndex];
                mesh.textureCount = reader.read<uint32_t>();

                mesh.matInfo.resize(mesh.textureCount);
                for (uint32_t t = 0; t < mesh.textureCount; ++t)
                {
                    mesh.matInfo[t].textureIndex = reader.read<uint32_t>();
                    mesh.matInfo[t].faceSpan = reader.read<uint32_t>();
                }
            }
            else if (chunk == "BVTX")
            {
                // version
                reader.skipBytes(4);

                // another instance of vertexCount
                reader.skipBytes(4);

                auto& mesh = aspectImpl->subMeshes[currentSubMeshIndex];
                mesh.positions.resize(mesh.vertexCount);
                for (uint32_t v = 0; v < mesh.vertexCount; ++v)
                {
                    auto vert = reader.read<vsg::vec3>();

                    mesh.positions[v] = vert;
                }
            }
            else if (chunk == "BCRN")
            {
                // version
                reader.skipBytes(4);

                reader.skipBytes(4);

                auto& mesh = aspectImpl->subMeshes[currentSubMeshIndex];

                mesh.corners.resize(mesh.cornerCount);
                for (uint32_t c = 0; c < mesh.cornerCount; ++c)
                {
                    auto& corner = mesh.corners[c];

                    // Vertex position:
                    corner.vtxIndex = reader.read<uint32_t>();
                    if (corner.vtxIndex > mesh.positions.size())
                    {
                        corner.vtxIndex = static_cast<uint32_t>(mesh.positions.size() - 1);
                    }

                    corner.position = mesh.positions[corner.vtxIndex];

                    // Vertex normal, color:
                    corner.normal = reader.read<vsg::vec3>();
                    corner.color[0] = reader.read<uint8_t>();
                    corner.color[1] = reader.read<uint8_t>();
                    corner.color[2] = reader.read<uint8_t>();
                    corner.color[3] = reader.read<uint8_t>();

                    // Why did they leave this unused field here in the middle?
                    /* auto unused = */ reader.skipBytes(4);

                    // Float UVs:
                    corner.texCoord = reader.read<vsg::vec2>();
                }
            }
            else if (chunk == "WCRN")
            {
                reader.skipBytes(4);

                reader.skipBytes(4);
                auto& mesh = aspectImpl->subMeshes[currentSubMeshIndex];

                mesh.wCorners.resize(mesh.cornerCount);
                for (uint32_t c = 0; c < mesh.cornerCount; ++c)
                {
                    auto& wCorner = mesh.wCorners[c];

                    wCorner.pos = reader.read<vsg::vec3>();
                    wCorner.weight = reader.read<vsg::quat>();

                    wCorner.bone[0] = reader.read<uint8_t>();
                    wCorner.bone[1] = reader.read<uint8_t>();
                    wCorner.bone[2] = reader.read<uint8_t>();
                    wCorner.bone[3] = reader.read<uint8_t>();

                    // TODO: handle potential differences for version < 40

                    wCorner.normal = reader.read<vsg::vec3>();

                    wCorner.color[0] = reader.read<uint8_t>();
                    wCorner.color[1] = reader.read<uint8_t>();
                    wCorner.color[2] = reader.read<uint8_t>();
                    wCorner.color[3] = reader.read<uint8_t>();

                    wCorner.texCoord = reader.read<vsg::vec2>();

                    /* TODO
                    // remove null bone/weights
                    // This is a reverse iteration, I guess, from 4 to 1 (or 0)
                    for i = 4 to 1 by -1 do
                    if (w[i] == 0) do
                    (
                        deleteItem w i
                        deleteItem b i
                    )
                    */
                }
            }
            else if (chunk == "BVMP")
            {
            }
            else if (chunk == "BTRI")
            {
                // version
                const auto version = reader.read<uint32_t>();

                // skip faceCount
                reader.skipBytes(4);

                auto& mesh = aspectImpl->subMeshes[currentSubMeshIndex];

                if (Aspect::Impl::versionOf(version) == 22)
                {
                    mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
                    for (uint32_t i = 0; i < mesh.textureCount; ++i)
                    {
                        mesh.faceInfo.cornerSpan[i] = reader.read<uint32_t>();
                    }

                    mesh.faceInfo.cornerStart.resize(mesh.textureCount);
                    mesh.faceInfo.cornerStart[0] = 0;
                    for (uint32_t i = 0; i < mesh.textureCount - 1; ++i)
                    {
                        mesh.faceInfo.cornerStart[i] =
                            mesh.faceInfo.cornerStart[i] + mesh.faceInfo.cornerSpan[i];
                    }
                }
                else if (Aspect::Impl::versionOf(version) > 22)
                {
                    mesh.faceInfo.cornerStart.resize(mesh.textureCount);
                    mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
                    for (uint32_t i = 0; i < mesh.textureCount; ++i)
                    {
                        mesh.faceInfo.cornerStart[i] = reader.read<uint32_t>();
                        mesh.faceInfo.cornerSpan[i] = reader.read<uint32_t>();
                    }
                }
                else
                {
                    mesh.faceInfo.cornerStart.resize(mesh.textureCount);
                    mesh.faceInfo.cornerSpan.resize(mesh.textureCount);
                    for (uint32_t i = 0; i < mesh.textureCount; ++i)
                    {
                        mesh.faceInfo.cornerStart[i] = 0;
                        mesh.faceInfo.cornerSpan[i] = mesh.cornerCount;
                    }
                }

                mesh.faceInfo.cornerIndex.resize(mesh.faceCount);
                for (uint32_t f = 0; f < mesh.faceCount; ++f)
                {
                    auto a = reader.read<uint32_t>();
                    auto b = reader.read<uint32_t>();
                    auto c = reader.read<uint32_t>();

                    mesh.faceInfo.cornerIndex[f].index[0] = a;
                    mesh.faceInfo.cornerIndex[f].index[1] = b;
                    mesh.faceInfo.cornerIndex[f].index[2] = c;
                }
            }
            else if (chunk == "RPOS")
            {
                // version
                reader.skipBytes(4);

                // numBones
                reader.skipBytes(4);

                aspectImpl->rposInfoAbI.resize(aspectImpl->boneInfos.size());
                aspectImpl->rposInfoRel.resize(aspectImpl->boneInfos.size());

                for (uint32_t i = 0; i < aspectImpl->boneInfos.size(); i++)
                {
                    Aspect::Impl::RPosInfo& abI = aspectImpl->rposInfoAbI[i];
                    abI.rotation = reader.read<vsg::dquat>();
                    abI.position = reader.read<vsg::vec3>();

                    Aspect::Impl::RPosInfo& rel = aspectImpl->rposInfoRel[i];
                    rel.rotation = reader.read<vsg::dquat>();
                    rel.position = reader.read<vsg::vec3>();
                }
            }
            else if (chunk == "BEND")
            {
            }
        }

        return Aspect::create(std::move(aspectImpl), options);
    };
} // namespace ehb