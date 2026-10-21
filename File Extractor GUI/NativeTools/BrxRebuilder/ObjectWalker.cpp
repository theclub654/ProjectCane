#include "BrxWalker.h"
#include "TriangleList.h"

#include <algorithm>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace {
void AppendF32(std::vector<std::uint8_t>& bytes, float value) {
    std::uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value), "unexpected float size");
    std::memcpy(&bits, &value, sizeof(bits));
    bytes.push_back(static_cast<std::uint8_t>(bits & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((bits >> 8) & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((bits >> 16) & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((bits >> 24) & 0xffu));
}

void AppendU16(std::vector<std::uint8_t>& bytes, std::uint16_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>(value >> 8));
}

void AppendU32(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
    bytes.push_back(static_cast<std::uint8_t>(value & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((value >> 8) & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((value >> 16) & 0xffu));
    bytes.push_back(static_cast<std::uint8_t>((value >> 24) & 0xffu));
}

void AppendVec2(std::vector<std::uint8_t>& bytes, const Vec2& value) {
    AppendF32(bytes, value.x); AppendF32(bytes, value.y);
}

void AppendVec3(std::vector<std::uint8_t>& bytes, const Vec3& value) {
    AppendF32(bytes, value.x); AppendF32(bytes, value.y); AppendF32(bytes, value.z);
}

void AppendVec4(std::vector<std::uint8_t>& bytes, const std::array<float, 4>& value) {
    for (float component : value) AppendF32(bytes, component);
}

struct CidLoader { int cid; const char* cidName; const char* loader; };
constexpr CidLoader kCidLoaders[] = {
#define BRX_CID(value, cid, loader, vtable) {value, #cid, #loader},
#include "CidLoaders.generated.inc"
#undef BRX_CID
};
const CidLoader* FindCid(int cid) {
    for (const auto& entry : kCidLoaders) if (entry.cid == cid) return &entry;
    return nullptr;
}
void ReadSaaRecord(CBinaryInputStream& s, std::uint16_t globIndex,
    std::uint16_t globCount, std::uint16_t flags, std::size_t globStart) {
    const auto saaStart = s.GetOffset();
    const auto kind = s.U16Read();
    if (kind == 0 || kind == 0xffff) return;
    if (kind > 8) {
        std::ostringstream out;
        out << "unknown SAAK " << kind << " at 0x" << std::hex << saaStart
            << "; glob " << std::dec << globIndex << '/' << globCount
            << " flags=0x" << std::hex << flags << " began at 0x" << globStart;
        throw std::runtime_error(out.str());
    }
    s.S16Read(); s.U16Read(); s.Skip(24);
}
void ReadTbsp(CBinaryInputStream& s) {
    const auto surfaces=s.U16Read();
    for (std::uint16_t i=0;i<surfaces;++i) { s.F32Read(); s.ReadVector(); }
    const auto nodes=s.U16Read();
    for (std::uint16_t i=0;i<nodes;++i) { s.U16Read(); s.S16Read(); s.S16Read(); }
}
void ReadCurve(CBinaryInputStream& s, int kind) {
    if (kind != 0 && kind != 1) throw std::runtime_error("unknown CRVK value");
    s.U8Read();
    const auto controls=s.U8Read();
    const std::size_t bytesPerControl = kind == 0 ? 16u : 40u;
    s.Skip(std::size_t(controls)*bytesPerControl);
}
void ReadScalarAnimation(CBinaryInputStream& s, int kind) {
    if (kind==0) { const auto n=s.U16Read(); s.Skip(std::size_t(n)*16); return; }
    if (kind==1) { const auto n=s.U16Read(); s.Skip(std::size_t(n)*24); return; }
    if (kind==2) { const auto n=s.U16Read(); s.Skip(std::size_t(n)*6); return; }
    if (kind==3 || kind==4) return;
    throw std::runtime_error("unknown ACGK value");
}
void ReadComponentAnimation(CBinaryInputStream& s) {
    auto flags=s.U8Read(); s.ReadVector();
    for (int axis=0;axis<3;++axis,flags>>=1)
        if (flags&1) { const auto kind=s.U8Read(); ReadScalarAnimation(s,kind); }
}
void ReadVectorAnimation(CBinaryInputStream& s, int kind) {
    if (kind==0) { ReadComponentAnimation(s); return; }
    if (kind==1) { const auto n=s.U16Read(); s.Skip(std::size_t(n)*38); return; }
    if (kind==2) return;
    throw std::runtime_error("unknown ACVK value");
}
void ReadAsegEvents(CBinaryInputStream& s, BrxWalker& walker, bool framed) {
    const auto signedCount=s.S8Read();
    if (signedCount<0) throw std::runtime_error("negative ASEG event count");
    bool havePrevious = false;
    int previousType = 0;
    std::size_t previousStart = 0, previousEnd = 0;
    for (int i=0;i<signedCount;++i) {
        const auto eventStart = s.GetOffset();
        const int type=s.S8Read(); if (framed) s.S16Read();
        switch(type) {
        case -1: break;
        // Two S16 values, two 32-bit encoded values, and three F32s.
        case 0: s.Skip(24); break;
        case 4: case 7: s.S16Read(); s.F32Read(); break;
        case 5: case 33: case 57: case 58: s.F32Read(); break;
        case 6: case 12: case 13: case 17: case 18: case 22: case 24:
        case 29: case 30: case 31: case 32: case 35: case 36: case 41: case 59:
        case 39: s.S16Read(); break;
        case 9: case 10: s.S16Read(); s.F32Read(); break;
        case 16: s.S16Read(); s.S8Read(); s.S8Read(); break;
        case 19: case 20: s.S16Read(); s.U8Read(); break;
        case 21: case 23: s.S16Read(); s.F32Read(); s.F32Read(); break;
        case 25: case 26:
            s.S16Read(); s.S16Read(); walker.ConvertLevelDescriptor(s); break;
        case 27: s.U8Read(); s.S16Read(); break;
        case 28: s.S16Read(); s.S16Read(); s.F32Read(); s.F32Read(); break;
        case 37: case 38: case 45: case 47: s.S16Read(); s.S16Read(); break;
        case 40: s.U16Read(); break;
        case 44: walker.ConvertAudioDescriptor(s); break;
        case 51: walker.ConvertCutsceneDescriptor(s); break;
        case 46: case 52: s.S16Read(); s.S16Read(); s.S16Read(); break;
        case 48: s.S16Read(); s.S16Read(); s.S16Read(); s.F32Read(); break;
        // EATYP_SetCredit stores line, visible duration, then subtitle text.
        // NTSC embeds the text directly. PAL stores an index into the leading
        // per-level localization table, which PCBRX expands to inline UTF-8.
        case 54:
            s.S16Read();
            s.S16Read();
            if (walker.IsPal())
                walker.ConvertLocalizationReference(s);
            else
                walker.ReadAndConvertString(s);
            break;
        case 1: case 2: case 3: case 8: case 11: case 14: case 15: case 34:
        case 42: case 43: case 49: case 50: case 53: case 55: case 56: break;
        default: {
            std::ostringstream out;
            out << "unknown EATYP " << type << " at event " << std::dec << i << '/'
                << static_cast<int>(signedCount) << " offset 0x" << std::hex << eventStart
                << " framed=" << std::dec << framed;
            if (havePrevious)
                out << "; previous EATYP=" << previousType << " covered 0x" << std::hex
                    << previousStart << "..0x" << previousEnd;
            throw std::runtime_error(out.str());
        }
        }
        havePrevious = true;
        previousType = type;
        previousStart = eventStart;
        previousEnd = s.GetOffset();
    }
}
void ReadAseg(CBinaryInputStream& s, BrxWalker& walker) {
    s.S16Read(); s.S16Read(); const auto roots=s.U16Read(); s.Skip(std::size_t(roots)*2);
    const auto signedChannels=s.S8Read();
    if (signedChannels<0) throw std::runtime_error("negative ASEG channel count");
    for (int i=0;i<signedChannels;++i) {
        s.S16Read(); const auto flags=s.S8Read();
        if (flags&1) { const auto kind=s.S8Read(); ReadVectorAnimation(s,kind); }
        if (flags&2) { const auto kind=s.S8Read(); ReadVectorAnimation(s,kind); }
        if (flags&4) { const auto kind=s.S8Read(); ReadVectorAnimation(s,kind); }
        if (flags&8) { const auto kind=s.S8Read(); ReadScalarAnimation(s,kind); }
        if (flags&0x10) {
            const auto poses=s.U8Read();
            for (std::uint8_t p=0;p<poses;++p) {
                const auto kind=s.S8Read(); if (kind!=-1) ReadScalarAnimation(s,kind);
            }
        }
    }
    ReadAsegEvents(s,walker,false); ReadAsegEvents(s,walker,true);
    ReadAsegEvents(s,walker,false);
    walker.LoadOptionsFromBrx(s);
}
void ReadEmitMesh(CBinaryInputStream& s) {
    const auto points=s.U16Read(); s.Skip(std::size_t(points)*12);
    const auto triangles=s.U16Read(); s.Skip(std::size_t(triangles)*10); s.ReadVector();
}
}

void BrxWalker::ReadEmitColors(CBinaryInputStream& s, std::uint16_t count) {
    // fColorRanges remains a byte. Only the packed RGBA8 array changes in v3.
    s.U8Read();
    const auto colorsBegin = s.GetOffset();
    std::vector<std::uint8_t> floatColors;
    floatColors.reserve(std::size_t(count) * 16);
    for (std::uint16_t i = 0; i < count; ++i) {
        const auto rgba = s.U32Read();
        AppendF32(floatColors, static_cast<float>((rgba >> 0) & 0xff) / 255.0f);
        AppendF32(floatColors, static_cast<float>((rgba >> 8) & 0xff) / 255.0f);
        AppendF32(floatColors, static_cast<float>((rgba >> 16) & 0xff) / 255.0f);
        AppendF32(floatColors, static_cast<float>((rgba >> 24) & 0xff) / 255.0f);
    }
    RecordReplacement(colorsBegin, s.GetOffset(), std::move(floatColors));
}

void BrxWalker::LoadSwObjectsFromBrx(CBinaryInputStream& s, unsigned depth) {
    if (depth > 256) throw std::runtime_error("object hierarchy exceeds 256 levels");
    constexpr std::int16_t kCidUnk11 = 107;
    const auto countStart = s.GetOffset();
    const auto count = s.U16Read();
    std::uint16_t removedCount = 0;
    bool havePrevious = false;
    int previousCid = 0, previousOid = 0, previousSplice = 0;
    std::size_t previousStart = 0, previousEnd = 0;
    for (std::uint16_t i = 0; i < count; ++i) {
        const auto objectStart = s.GetOffset();
        const auto cid = s.S16Read(), oid = s.S16Read(), splice = s.S16Read();
        ++stats_.objects;
        try { DispatchObject(s, cid, oid, splice, depth); }
        catch (const std::exception& e) {
            std::ostringstream out;
            out << "object " << i << '/' << count << " CID=" << cid << " OID=" << oid
                << " splice=" << splice << " at 0x" << std::hex << s.GetOffset()
                << ": " << e.what();
            if (havePrevious) {
                out << "; previous object CID=" << std::dec << previousCid
                    << " OID=" << previousOid << " splice=" << previousSplice
                    << " covered 0x" << std::hex << previousStart << "..0x" << previousEnd;
            }
            throw std::runtime_error(out.str());
        }
        havePrevious = true;
        previousCid = cid;
        previousOid = oid;
        previousSplice = splice;
        previousStart = objectStart;
        previousEnd = s.GetOffset();

        if (cid == kCidUnk11) {
            const auto objectEnd = s.GetOffset();

            // CID_UNK11 is the retail executable-integrity checker. Parse it
            // normally so malformed source data still fails validation, then
            // discard both the complete serialized object and any conversions
            // that were discovered while walking its ALO hierarchy.
            rebuildOperations_.erase(std::remove_if(rebuildOperations_.begin(),
                rebuildOperations_.end(), [objectStart, objectEnd](const RebuildOperation& operation) {
                    const bool overlaps = operation.begin < objectEnd && operation.end > objectStart;
                    if (!overlaps)
                        return false;
                    if (operation.begin < objectStart || operation.end > objectEnd)
                        throw std::runtime_error("CID_UNK11 contains a partially overlapping rebuild operation");
                    return true;
                }), rebuildOperations_.end());
            RecordReplacement(objectStart, objectEnd, {});
            ++removedCount;
        }
    }

    if (removedCount != 0) {
        std::vector<std::uint8_t> replacement;
        AppendU16(replacement, static_cast<std::uint16_t>(count - removedCount));
        RecordReplacement(countStart, countStart + sizeof(std::uint16_t),
            std::move(replacement));
    }
}

void BrxWalker::DispatchObject(CBinaryInputStream& s, std::int16_t cid,
    std::int16_t oid, std::int16_t splice, unsigned depth) {
    const auto loaderStart = s.GetOffset();
    const auto* entry = FindCid(cid);
    if (!entry) throw std::runtime_error("CID is outside g_mpcidpvt");
    const std::string_view loader(entry->loader);
    try {
    if (loader == "LoadLoFromBrx") return LoadLo(s);
    if (loader == "LoadSwFromBrx") return LoadSwFromBrx(s);
    if (loader == "LoadAloFromBrx") return LoadAlo(s, depth);
    if (loader == "LoadSoFromBrx") return LoadSo(s, depth);
    // These retail overrides add no serialized fields; they only select runtime behavior.
    if (loader == "LoadBombFromBrx" || loader == "LoadBrkFromBrx" ||
        loader == "LoadButtonFromBrx" || loader == "LoadChkpntFromBrx" ||
        loader == "LoadDartFromBrx" || loader == "LoadFlyFromBrx" ||
        loader == "LoadHbskFromBrx" || loader == "LoadJloFromBrx" ||
        loader == "LoadJtFromBrx" || loader == "LoadLasenFromBrx" ||
        loader == "LoadMgvFromBrx" || loader == "LoadPufferFromBrx" ||
        loader == "LoadRatFromBrx" || loader == "LoadRocFromBrx" ||
        loader == "LoadRohFromBrx" || loader == "LoadRopFromBrx" ||
        loader == "LoadRostFromBrx" || loader == "LoadRovFromBrx" ||
        loader == "LoadRythmFromBrx" || loader == "LoadSprizeFromBrx" ||
        loader == "LoadStepGuardFromBrx" || loader == "LoadVolbtnFromBrx" ||
        loader == "LoadMissileFromBrx" || loader == "LoadClueFromBrx" ||
        loader == "LoadMbgFromBrx" || loader == "LoadRubyFromBrx") return LoadSo(s, depth);
    if (loader == "LoadDprizeFromBrx" || loader == "LoadJlocFromBrx" ||
        loader == "LoadLockFromBrx" || loader == "LoadLockgFromBrx")
        return LoadAlo(s, depth);
    if (loader == "LoadCnvoFromBrx") {
        LoadSo(s, depth); s.F32Read(); s.F32Read(); return;
    }
    if (loader == "LoadPntFromBrx") {
        s.ReadVector(); LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadScanFromBrx") {
        s.ReadVector(); LoadOptionsFromBrx(s); ReadAndConvertString(s); return;
    }
    if (loader == "LoadXfmFromBrx" || loader == "LoadHndFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadVolFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); ReadTbsp(s); LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadFlashFromBrx") {
        LoadAlo(s, depth); ReadTbsp(s); return;
    }
    if (loader == "LoadKeyholeFromBrx") {
        LoadOptionsFromBrx(s);
        const auto points=s.U16Read(); s.Skip(std::size_t(points)*8);
        s.Skip(16);
        for (int set=0;set<5;++set) {
            const auto triangles=s.U16Read(); s.Skip(std::size_t(triangles)*6);
        }
        return;
    }
    if (loader == "LoadPathZoneFromBrx") {
        LoadLo(s);
        const auto vertices=s.U16Read();
        const auto boundaryEdges=s.U16Read();
        const auto edges=std::uint32_t(boundaryEdges)+s.U16Read();
        const auto triangles=s.U16Read();
        for (std::uint16_t i=0;i<vertices;++i) {
            s.ReadVector(); const auto adjacent=s.U8Read();
            for (std::uint8_t j=0;j<adjacent;++j) s.U16Read();
        }
        s.Skip(std::size_t(edges)*4); s.Skip(std::size_t(triangles)*6);
        const auto bsp=s.U16Read();
        for (std::uint16_t i=0;i<bsp;++i) { s.ReadVector(); s.F32Read(); s.S16Read(); s.S16Read(); }
        return;
    }
    if (loader == "LoadRchmFromBrx") {
        for (int i=0;i<24;++i) s.S16Read();
        s.S16Read(); const auto geom=ReadGeom(s);
        for (std::uint16_t i=0;i<geom.positions;++i) s.S8Read();
        const auto towers=s.S16Read();
        if (towers<0) throw std::runtime_error("negative RCHM tower count");
        s.Skip(std::size_t(towers)*8);
        const auto nodes=s.S16Read();
        if (nodes<0) throw std::runtime_error("negative RCHM BSP count");
        s.Skip(std::size_t(nodes)*6); s.F32Read(); LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadVismapFromBrx") {
        const auto vismapBegin = s.GetOffset();
        const auto nodes = s.U16Read();
        std::vector<std::uint8_t> replacement;
        replacement.reserve(2u + std::size_t(nodes) * 32u);
        AppendU16(replacement, nodes);

        for (std::uint16_t i = 0; i < nodes; ++i) {
            const Vec3 normal = s.ReadVector();
            const float planeDistance = s.F32Read();
            const std::uint32_t oldNegative = s.U32Read();
            const std::uint32_t oldPositive = s.U32Read();

            const auto appendChild = [&](std::uint32_t oldChild) {
                if ((oldChild & 0x80000000u) != 0) {
                    AppendU32(replacement, 0xffffffffu); // -1: leaf
                    AppendU32(replacement, oldChild & 0x7fffffffu);
                } else {
                    if (oldChild >= nodes)
                        throw std::runtime_error("vismap child index is out of range");
                    AppendU32(replacement, oldChild);
                    AppendU32(replacement, 0u);
                }
            };

            AppendVec3(replacement, normal);
            AppendF32(replacement, planeDistance);
            appendChild(oldNegative);
            appendChild(oldPositive);
        }

        const auto zones = s.U16Read();
        if (zones > 32)
            throw std::runtime_error("vismap one-hop zone count exceeds 32-bit masks");
        AppendU16(replacement, zones);
        for (std::uint16_t i = 0; i < zones; ++i)
            AppendU32(replacement, s.U32Read());

        RecordReplacement(vismapBegin, s.GetOffset(), std::move(replacement));
        LoadOptionsFromBrx(s);
        return;
    }
    if (loader == "LoadShapeFromBrx") {
        const auto kind=s.U8Read(); ReadCurve(s,kind); LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadTnFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); ReadTbsp(s);
        const auto kind=s.S8Read();
        if (kind == -1) { s.ReadMatrix3(); s.ReadVector(); }
        else ReadCurve(s,kind);
        LoadOptionsFromBrx(s); return;
    }
    if (loader == "LoadProxyFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); const auto sources=s.U8Read();
        for (std::uint8_t i=0;i<sources;++i) {
            const auto sourceCid=s.S16Read();
            if (sourceCid == -1) s.S16Read();
            else {
                const auto sourceOid=s.S16Read(), sourceSplice=s.S16Read();
                DispatchObject(s,sourceCid,sourceOid,sourceSplice,depth+1); s.S16Read();
            }
        }
        const auto overrides=s.U8Read();
        for (std::uint8_t i=0;i<overrides;++i) {
            s.S16Read(); s.S16Read(); s.U32Read();
            if (s.S8Read()==2) s.F32Read();
            LoadOptionsFromBrx(s);
        }
        if (sources==1) LoadSwObjectsFromBrx(s,depth+1);
        return;
    }
    if (loader == "LoadExplgFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); LoadOptionsFromBrx(s);
        const auto count=s.S16Read();
        if (count<0) throw std::runtime_error("negative EXPLG object count");
        for (int i=0;i<count;++i) {
            const auto childCid=s.S16Read(), childOid=s.S16Read(), childContext=s.S16Read();
            DispatchObject(s,childCid,childOid,childContext,depth+1);
        }
        return;
    }
    if (loader == "LoadSmFromBrx") {
        const auto states=s.U8Read(); s.Skip(std::size_t(states)*4);
        const auto transitions=s.U8Read();
        std::vector<bool> hasAnimation(transitions);
        for (std::uint8_t i=0;i<transitions;++i) {
            hasAnimation[i]=s.S8Read()!=0; s.S16Read(); s.S16Read(); s.S32Read(); s.F32Read();
        }
        LoadOptionsFromBrx(s); s.S16Read();
        for (bool animation:hasAnimation) if (animation) {
            const auto childCid=s.S16Read(), childOid=s.S16Read(), childContext=s.S16Read();
            DispatchObject(s,childCid,childOid,childContext,depth+1);
        }
        return;
    }
    if (loader == "LoadAsegFromBrx") { ReadAseg(s,*this); return; }
    if (loader == "LoadDialogFromBrx") {
        LoadAlo(s,depth); const auto count=s.S8Read();
        if (count<0) throw std::runtime_error("negative dialog event count");
        for (int i=0;i<count;++i) {
            switch(s.S8Read()) {
            case 0: ConvertAudioDescriptor(s); break;
            case 7: ConvertCutsceneDescriptor(s); break;
            case 1: case 2: s.S16Read(); s.S16Read(); break;
            case 3: s.F32Read(); s.F32Read(); s.U8Read(); s.U8Read();
                s.S16Read(); s.S16Read(); s.S16Read(); s.S16Read();
                if (IsJapanese()) ReadAndConvertUtf16String(s);
                else ReadAndConvertString(s);
                break;
            case 4: s.S16Read(); break;
            case 5: case 6: break;
            default: throw std::runtime_error("unknown DEK value");
            }
        }
        return;
    }
    if (loader == "LoadWarpFromBrx") {
        s.ReadMatrix3(); s.ReadVector(); LoadOptionsFromBrx(s);
        const auto count=s.S16Read(); if(count<0) throw std::runtime_error("negative WARP ASEG count");
        for(int i=0;i<count;++i) { s.S16Read(); s.S16Read(); s.S16Read(); ReadAseg(s,*this); }
        ReadAndConvertString(s); const auto hidden=s.S16Read();
        if(hidden<0) throw std::runtime_error("negative WARP hidden-object count");
        s.Skip(std::size_t(hidden)*2); return;
    }
    if (loader == "LoadExitFromBrx") {
        ConvertLevelDescriptor(s); ConvertLevelDescriptor(s);
        s.ReadMatrix3(); s.ReadVector(); ReadTbsp(s); LoadOptionsFromBrx(s);
        const auto count=s.S16Read(); if(count<0) throw std::runtime_error("negative EXIT ASEG count");
        for(int i=0;i<count;++i) { s.S16Read(); s.S16Read(); s.U16Read(); ReadAseg(s,*this); }
        return;
    }
    if (loader == "LoadEmitterFromBrx") {
        captureEmitOrigin_=true; emitOriginCaptured_=false; emitOriginKind_=0;
        LoadAlo(s,depth); captureEmitOrigin_=false;
        const auto curve=s.S8Read(); if(curve!=-1) ReadCurve(s,curve);
        if(emitOriginKind_==4) ReadEmitMesh(s);
        const auto colors=s.U16Read(); if(colors) ReadEmitColors(s,colors);
        return;
    }
    if (loader == "LoadExploFromBrx") {
        captureEmitOrigin_=true; emitOriginCaptured_=false; emitOriginKind_=0;
        s.ReadMatrix3(); s.ReadVector(); LoadOptionsFromBrx(s); captureEmitOrigin_=false;
        const auto curve=s.S8Read(); if(curve!=-1) ReadCurve(s,curve);
        if(emitOriginKind_==4) ReadEmitMesh(s);
        const auto colors=s.U16Read(); if(colors) ReadEmitColors(s,colors);
        return;
    }
    if (loader == "NONE")
        throw std::runtime_error(std::string("CID has no LO binary loader: ") + entry->cidName);
    throw std::runtime_error(std::string("internal CID dispatch mismatch: ") + entry->loader +
        " (" + entry->cidName + ")");
    } catch (const std::exception& exception) {
        std::ostringstream out;
        out << loader << " for CID=" << cid << " OID=" << oid << " splice=" << splice
            << " entered at 0x" << std::hex << loaderStart << " failed at 0x"
            << s.GetOffset() << ": " << exception.what();
        throw std::runtime_error(out.str());
    }
}

void BrxWalker::LoadLo(CBinaryInputStream& s) { LoadOptionsFromBrx(s); }

void BrxWalker::LoadAlox(CBinaryInputStream& s) {
    const auto flags = s.U32Read();
    if (flags & 0x01) s.ReadMatrix3();
    if (flags & 0x02) s.ReadMatrix3();
    if (flags & 0x0c) s.S16Read();
    if (flags & 0x10) s.S16Read();
    if (flags & 0x20) { s.S16Read(); s.ReadVector(); s.ReadVector(); s.F32Read(); }
    if (flags & 0x40) { s.S16Read(); s.S16Read(); }
    if (flags & 0x80) s.U8Read();
}

void BrxWalker::LoadAlo(CBinaryInputStream& s, unsigned depth) {
    s.ReadMatrix3(); s.ReadVector();
    s.U8Read(); s.U8Read(); s.U8Read(); s.U32Read();
    s.F32Read(); s.F32Read(); s.F32Read(); s.F32Read();
    LoadOptionsFromBrx(s);
    // Emitter origin options belong to this ALO itself, not to descendants.
    if (captureEmitOrigin_) captureEmitOrigin_=false;
    const auto poseCount = LoadGlobset(s);
    LoadAlox(s);
    const auto poseCopies = s.U8Read();
    for (std::uint8_t i = 0; i < poseCopies; ++i) {
        s.S16Read();
        for (std::uint8_t pose = 0; pose < poseCount; ++pose) s.F32Read();
    }
    LoadSwObjectsFromBrx(s, depth + 1);
}

void BrxWalker::LoadSo(CBinaryInputStream& s, unsigned depth) {
    s.U8Read();
    const auto geometry = ReadGeom(s); ReadBspc(s, geometry);
    s.F32Read(); s.ReadMatrix3(); s.ReadVector();
    const auto normalGroups = s.U16Read();
    for (std::uint16_t i = 0; i < normalGroups; ++i) { s.S16Read(); s.U16Read(); }
    const auto bspNodeMap = s.U32Read();
    for (std::uint32_t i = 0; i < bspNodeMap; ++i) s.S16Read();
    const auto hitSurfaceGroups = s.U32Read();
    for (std::uint32_t i = 0; i < hitSurfaceGroups; ++i) { s.S16Read(); s.S16Read(); }
    if (hitSurfaceGroups)
        for (std::uint32_t i = 0; i <= geometry.surfaces; ++i) s.S16Read();
    const auto cameraGeometry = ReadGeom(s); ReadBspc(s, cameraGeometry);
    LoadAlo(s, depth);
}

std::uint8_t BrxWalker::LoadGlobset(CBinaryInputStream& s) {
    const auto globsetStart = s.GetOffset();
    const auto vertexKind = s.U8Read();
    const auto normalKind = s.U8Read();
    const auto boneCount = s.U8Read();
    for (std::uint8_t i = 0; i < boneCount; ++i) s.S16Read();
    const auto poseCount = s.U8Read();
    for (std::uint8_t i = 0; i < poseCount; ++i) s.F32Read();
    const auto globCount = s.U16Read();
    // Every serialized glob contains at least its flags and the common
    // position/radius/material fields (23 bytes).  Retail levels can contain
    // well over 2048 globs, so validate against the bytes that remain instead
    // of imposing an invented count ceiling.
    constexpr std::size_t minimumSerializedGlobSize = 23;
    if (globCount > s.Remaining() / minimumSerializedGlobSize) {
        std::ostringstream out;
        out << "glob count " << globCount << " cannot fit in the remaining stream at 0x" << std::hex
            << (s.GetOffset() - 2) << "; globset began at 0x" << globsetStart
            << " vertexKind=" << std::dec << static_cast<unsigned>(vertexKind)
            << " normalKind=" << static_cast<unsigned>(normalKind)
            << " bones=" << static_cast<unsigned>(boneCount)
            << " poses=" << static_cast<unsigned>(poseCount)
            << "; preceding option list 0x" << std::hex << lastOptionListStart_
            << "..0x" << lastOptionListEnd_
            << " terminator at 0x" << lastOptionTerminator_;
        if (haveLastOption_) {
            out << " last EOPID=" << std::dec << lastOptionId_
                << " covered 0x" << std::hex << lastOptionStart_
                << "..0x" << lastOptionEnd_;
        }
        throw std::runtime_error(out.str());
    }
    for (std::uint16_t g = 0; g < globCount; ++g) {
        const auto globStart = s.GetOffset();
        const auto flags = s.U16Read(); const bool instance = (flags & 1) != 0;
        if (instance) { s.S16Read(); s.ReadMatrix4(); }
        if (flags & 0x002) s.U32Read(); if (flags & 0x200) s.F32Read();
        if (flags & 0x004) s.F32Read(); if (flags & 0x008) s.F32Read();
        if (flags & 0x010) s.F32Read(); if (flags & 0x020) s.F32Read();
        if (flags & 0x040) ReadSaaRecord(s, g, globCount, flags, globStart);
        if (flags & 0x080) { s.ReadVector(); s.Skip(16); }
        if (flags & 0x100) {
            s.S16Read(); const auto warpKind = s.S8Read();
            if (warpKind != -1) { s.Skip(16); s.ReadMatrix4(); }
            s.U8Read(); s.U8Read(); s.U8Read();
        }
        s.ReadVector(); s.F32Read(); s.S16Read(); s.U8Read(); s.U8Read(); s.U8Read();
        if (instance) continue;
        const auto subglobCount = s.U16Read();
        for (std::uint16_t sub = 0; sub < subglobCount; ++sub) {
            s.ReadVector(); s.F32Read();
            const auto pcBlockBegin = s.GetOffset();
            const auto vc=s.U8Read(), nc=s.U8Read(), cc=s.U8Read(), tc=s.U8Read(), ic=s.U8Read();
            s.Align(4);
            std::vector<Vec3> positions(vc);
            std::vector<Vec3> normals(nc);
            std::vector<std::array<float, 4>> colors(cc);
            std::vector<Vec2> texcoords(tc);
            for (Vec3& value : positions) value = s.ReadVector();
            for (Vec3& value : normals) value = s.ReadVector();
            for (auto& value : colors)
                for (float& component : value)
                    component = static_cast<float>(s.U8Read()) / 255.0f;
            for (Vec2& value : texcoords) value = s.ReadVector2();

            std::vector<StripVertex> strip(ic);
            for (StripVertex& vertex : strip) {
                vertex.position = s.U8Read();
                vertex.normal = s.U8Read();
                vertex.texcoord = s.U8Read();
                vertex.misc = s.U8Read();
            }

            const std::vector<Triangle> triangles = ConvertTriangleStrip(strip);
            stats_.stripVertices += strip.size();
            stats_.triangles += triangles.size();

            const auto shaderId = s.U16Read();
            const auto selfIllumByte = s.U8Read();
            const auto bi=s.U8Read();
            std::vector<std::uint8_t> boneMap(bi);
            for (auto& bone : boneMap) bone = s.U8Read();
            std::vector<float> weights(static_cast<std::size_t>(vc) * bi);
            for (float& weight : weights) weight = s.F32Read();

            std::vector<Vec3> posePositions;
            std::vector<Vec3> poseNormals;
            std::vector<std::uint16_t> posePositionIndices;
            std::vector<std::uint16_t> poseNormalIndices;
            if (poseCount) {
                const auto pp=s.U16Read(); posePositions.resize(pp);
                for (Vec3& value : posePositions) value = s.ReadVector();
                const auto pn=s.U16Read(); poseNormals.resize(pn);
                for (Vec3& value : poseNormals) value = s.ReadVector();
                posePositionIndices.resize(static_cast<std::size_t>(poseCount) * ic);
                poseNormalIndices.resize(static_cast<std::size_t>(poseCount) * ic);
                for (std::uint8_t pose = 0; pose < poseCount; ++pose) {
                    for (std::uint8_t vertex = 0; vertex < ic; ++vertex)
                        posePositionIndices[static_cast<std::size_t>(pose) * ic + vertex] = s.U16Read();
                    for (std::uint8_t vertex = 0; vertex < ic; ++vertex)
                        poseNormalIndices[static_cast<std::size_t>(pose) * ic + vertex] = s.U16Read();
                }
            }

            if (shaderId >= shaderColors_.size())
                throw std::runtime_error("subglob shader index is out of range");

            std::vector<std::uint8_t> replacement;
            if (triangles.size() > UINT16_MAX)
                throw std::runtime_error("subglob triangle count exceeds PCBRX uint16 range");
            replacement.reserve(12u + static_cast<std::size_t>(ic) * 72u +
                triangles.size() * 6u + static_cast<std::size_t>(ic) * poseCount * 32u);
            AppendU16(replacement, static_cast<std::uint16_t>(ic));
            AppendU16(replacement, static_cast<std::uint16_t>(triangles.size()));
            AppendU16(replacement, shaderId);
            AppendU16(replacement, static_cast<std::uint16_t>(std::min<std::uint8_t>(bi, 4)));
            AppendF32(replacement, static_cast<float>((selfIllumByte * 0x7FA6u) / 0xFFu));

            const auto& shaderColor = shaderColors_[shaderId];
            for (const StripVertex& vertex : strip) {
                if (vertex.position >= positions.size())
                    throw std::runtime_error("subglob position index is out of range");
                AppendVec3(replacement, positions[vertex.position]);

                if (vertex.normal == 0xffu) AppendVec3(replacement, {});
                else {
                    if (vertex.normal >= normals.size())
                        throw std::runtime_error("subglob normal index is out of range");
                    AppendVec3(replacement, normals[vertex.normal]);
                }

                std::array<float, 4> finalColor = shaderColor;
                const auto colorIndex = static_cast<std::uint8_t>(vertex.misc & 0x7fu);
                if (colorIndex != 0x7fu) {
                    if (colorIndex >= colors.size())
                        throw std::runtime_error("subglob color index is out of range");
                    for (int component = 0; component < 4; ++component)
                        finalColor[component] *= colors[colorIndex][component];
                }
                AppendVec4(replacement, finalColor);

                if (vertex.texcoord == 0xffu) AppendVec2(replacement, {});
                else {
                    if (vertex.texcoord >= texcoords.size())
                        throw std::runtime_error("subglob texcoord index is out of range");
                    AppendVec2(replacement, texcoords[vertex.texcoord]);
                }

                for (std::uint32_t influence = 0; influence < 4; ++influence)
                    AppendU16(replacement, static_cast<std::uint16_t>(
                        influence < boneMap.size() ? boneMap[influence] : 0u));
                for (std::uint32_t influence = 0; influence < 4; ++influence) {
                    const auto weightIndex = static_cast<std::size_t>(vertex.position) * bi + influence;
                    AppendF32(replacement, influence < bi && weightIndex < weights.size()
                        ? weights[weightIndex] : 0.0f);
                }
            }

            for (const Triangle& triangle : triangles) {
                if (triangle.v0 > UINT16_MAX || triangle.v1 > UINT16_MAX ||
                    triangle.v2 > UINT16_MAX)
                    throw std::runtime_error("subglob triangle index exceeds PCBRX uint16 range");
                AppendU16(replacement, static_cast<std::uint16_t>(triangle.v0));
                AppendU16(replacement, static_cast<std::uint16_t>(triangle.v1));
                AppendU16(replacement, static_cast<std::uint16_t>(triangle.v2));
            }

            for (std::size_t vertexIndex = 0; vertexIndex < strip.size(); ++vertexIndex) {
                const StripVertex& vertex = strip[vertexIndex];
                for (std::uint8_t pose = 0; pose < poseCount; ++pose) {
                    const auto lookup = static_cast<std::size_t>(pose) * ic + vertexIndex;
                    const auto posePosition = posePositionIndices[lookup];
                    const Vec3& basePosition = positions[vertex.position];
                    const Vec3 posedPosition = posePosition < posePositions.size()
                        ? posePositions[posePosition] : basePosition;
                    AppendF32(replacement, posedPosition.x - basePosition.x);
                    AppendF32(replacement, posedPosition.y - basePosition.y);
                    AppendF32(replacement, posedPosition.z - basePosition.z);
                    AppendF32(replacement, 0.0f);

                    if (vertex.normal == 0xffu) {
                        AppendVec4(replacement, {0.0f, 0.0f, 0.0f, 0.0f});
                    } else {
                        const auto poseNormal = poseNormalIndices[lookup];
                        const Vec3& baseNormal = normals[vertex.normal];
                        const Vec3 posedNormal = poseNormal < poseNormals.size()
                            ? poseNormals[poseNormal] : baseNormal;
                        AppendF32(replacement, posedNormal.x - baseNormal.x);
                        AppendF32(replacement, posedNormal.y - baseNormal.y);
                        AppendF32(replacement, posedNormal.z - baseNormal.z);
                        AppendF32(replacement, 0.0f);
                    }
                }
            }
            RecordReplacement(pcBlockBegin, s.GetOffset(), std::move(replacement));
        }
        const auto subcelCount=s.U16Read();
        for (std::uint16_t sub=0; sub<subcelCount; ++sub) {
            const auto pcBlockBegin = s.GetOffset();
            const auto pc=s.U8Read();
            std::vector<Vec3> positions(pc);
            for (Vec3& position : positions) position = s.ReadVector();

            const auto ec=s.U8Read();
            std::vector<std::array<std::uint8_t, 4>> edges(ec);
            for (auto& edge : edges)
                for (auto& index : edge)
                    index = s.U8Read();

            const auto bi=s.U8Read();
            std::vector<std::uint8_t> boneMap(bi);
            for (auto& bone : boneMap) bone = s.U8Read();
            std::vector<float> weights(static_cast<std::size_t>(bi) * pc);
            for (float& weight : weights) weight = s.F32Read();

            std::vector<Vec3> posePositions;
            std::vector<std::uint16_t> posePositionIndices;
            if (poseCount) {
                const auto pp=s.U16Read();
                posePositions.resize(pp);
                for (Vec3& position : posePositions) position = s.ReadVector();
                posePositionIndices.resize(static_cast<std::size_t>(poseCount) * pc);
                for (auto& index : posePositionIndices) index = s.U16Read();
            }

            std::vector<std::uint8_t> replacement;
            replacement.reserve(4u + static_cast<std::size_t>(ec) * 4u * 40u +
                static_cast<std::size_t>(ec) * 4u * poseCount * 16u);
            AppendU32(replacement, ec);

            std::vector<std::uint8_t> pointPositionIndices;
            pointPositionIndices.reserve(static_cast<std::size_t>(ec) * 4u);
            for (const auto& edge : edges) {
                // Runtime cel-border orientation expects E0, E1, OA, OB.
                pointPositionIndices.push_back(edge[1]);
                pointPositionIndices.push_back(edge[2]);
                pointPositionIndices.push_back(edge[0]);
                pointPositionIndices.push_back(edge[3]);
            }

            for (const auto positionIndex : pointPositionIndices) {
                if (positionIndex >= positions.size())
                    throw std::runtime_error("cel-border position index is out of range");
                AppendVec3(replacement, positions[positionIndex]);
                AppendF32(replacement, 1.0f);

                for (std::uint32_t influence = 0; influence < 4; ++influence)
                    AppendU16(replacement, static_cast<std::uint16_t>(
                        influence < boneMap.size() ? boneMap[influence] : 0u));
                for (std::uint32_t influence = 0; influence < 4; ++influence) {
                    const auto weightIndex = static_cast<std::size_t>(positionIndex) * bi + influence;
                    AppendF32(replacement, influence < bi && weightIndex < weights.size()
                        ? weights[weightIndex] : 0.0f);
                }
            }

            for (const auto positionIndex : pointPositionIndices) {
                for (std::uint8_t pose = 0; pose < poseCount; ++pose) {
                    const auto lookup = static_cast<std::size_t>(pose) * pc + positionIndex;
                    const auto posePosition = posePositionIndices[lookup];
                    const Vec3& base = positions[positionIndex];
                    const Vec3 posed = posePosition < posePositions.size()
                        ? posePositions[posePosition] : base;
                    AppendF32(replacement, posed.x - base.x);
                    AppendF32(replacement, posed.y - base.y);
                    AppendF32(replacement, posed.z - base.z);
                    AppendF32(replacement, 0.0f);
                }
            }

            RecordReplacement(pcBlockBegin, s.GetOffset(), std::move(replacement));
        }
    }
    return poseCount;
}
