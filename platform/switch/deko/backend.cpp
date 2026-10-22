#include "backend.h"
#include "allocator.h"
#include "dksh.h"
#include "texture_decode.h"
#include "deko_shaders.h"
#include <switch.h>
#include <deko3d.h>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace CaneDeko
{
namespace
{
constexpr unsigned FrameCount = 3, ImageDescriptorCount = 8192, SamplerDescriptorCount = 4096;
constexpr uint32_t PoolSize = 16 * 1024 * 1024, UploadChunkSize = 4 * 1024 * 1024;
DkDevice device{};
DkQueue queue{};
DkSwapchain swapchain{};
DkMemBlock swapMemory{};
std::array<DkImage, FrameCount> swapImages{};
unsigned outputWidth{}, outputHeight{}, frameIndex{};
uint64_t nextSerial = 1;
bool recording = false;
int swapInterval = 1;
GLuint nextObject = 1;
[[noreturn]] void Fail(const std::string& message)
{
    throw std::runtime_error("Deko3D: " + message);
}
DkMemBlock MakeMemory(uint32_t size, uint32_t flags)
{
    DkMemBlockMaker maker; dkMemBlockMakerDefaults(&maker, device, Align(size, DK_MEMBLOCK_ALIGNMENT));
    maker.flags = flags;
    auto result = dkMemBlockCreate(&maker);
    if (!result) Fail("GPU memory allocation failed (" + std::to_string(size) + " bytes)");
    return result;
}
struct Memory
{
    DkMemBlock block;
    explicit Memory(uint32_t size, uint32_t flags) : block(MakeMemory(size, flags)) {}
    ~Memory() { dkMemBlockDestroy(block); }
    DkGpuAddr Gpu() const { return dkMemBlockGetGpuAddr(block); }
    uint8_t* Cpu() const { return static_cast<uint8_t*>(dkMemBlockGetCpuAddr(block)); }
    uint32_t Size() const { return dkMemBlockGetSize(block); }
};
struct Page
{
    std::shared_ptr<Memory> memory;
    RangeAllocator ranges;
    Page(uint32_t size, uint32_t flags) : memory(std::make_shared<Memory>(size, flags)), ranges(size) {}
};
struct Allocation
{
    std::shared_ptr<Page> page;
    uint32_t offset, size;
    Allocation(std::shared_ptr<Page> p, uint32_t o, uint32_t s) : page(std::move(p)), offset(o), size(s) {}
    ~Allocation() { page->ranges.Release(offset, size); }
    DkGpuAddr Gpu() const { return page->memory->Gpu() + offset; }
    uint8_t* Cpu() const { return page->memory->Cpu() + offset; }
};
std::vector<std::shared_ptr<Page>> bufferPages, imagePages;
std::shared_ptr<Allocation> Allocate(uint32_t size, uint32_t alignment, bool image = false)
{
    size = Align(std::max(size, 1u), alignment);
    auto& pages = image ? imagePages : bufferPages;
    for (const auto& page : pages)
        if (auto offset = page->ranges.Allocate(size, alignment))
            return std::make_shared<Allocation>(page, *offset, size);
    const uint32_t flags = DkMemBlockFlags_GpuCached | (image ? DkMemBlockFlags_Image : DkMemBlockFlags_CpuUncached);
    auto page = std::make_shared<Page>(Align(std::max(PoolSize, size), DK_MEMBLOCK_ALIGNMENT), flags);
    pages.push_back(page);
    return std::make_shared<Allocation>(page, *page->ranges.Allocate(size, alignment), size);
}
struct UploadChunk { std::shared_ptr<Memory> memory; uint32_t cursor{}; };
struct Frame
{
    DkCmdBuf command{};
    std::vector<std::shared_ptr<Memory>> commandMemory;
    size_t nextCommandChunk{};
    std::vector<UploadChunk> upload;
    std::shared_ptr<Memory> descriptors;
    std::vector<std::shared_ptr<void>> keepAlive;
    std::unordered_set<const void*> retained;
    std::unordered_map<uint64_t, unsigned> images, samplers;
    unsigned imageCount{}, samplerCount{};
    DkFence fence{};
    uint64_t serial{};
    bool submitted{};
    int imageSlot{};
};
std::array<Frame, FrameCount> frames;
Frame& Current() { return frames[frameIndex]; }
void GrowCommands(void* user, DkCmdBuf command, size_t minimum)
{
    auto& frame = *static_cast<Frame*>(user);
    const uint32_t size = Align(uint32_t(std::max<size_t>(minimum, 1024 * 1024)), DK_MEMBLOCK_ALIGNMENT);
    if (frame.nextCommandChunk == frame.commandMemory.size())
        frame.commandMemory.push_back(std::make_shared<Memory>(size, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached));
    auto& memory = frame.commandMemory[frame.nextCommandChunk++];
    if (memory->Size() < size)
        memory = std::make_shared<Memory>(size, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    dkCmdBufAddMemory(command, memory->block, 0, memory->Size());
}
void InvalidateState();
void EnsureFrame()
{
    if (recording) return;
    auto& frame = Current();
    if (frame.submitted)
    {

        if (dkFenceWait(&frame.fence, -1) != DkResult_Success) Fail("Frame retirement fence failed");
    }
    frame.keepAlive.clear(); frame.retained.clear();

    std::erase_if(imagePages, [](const auto& page) {
        return page.use_count() == 1 && page->memory->Size() > PoolSize &&
            page->ranges.FreeBytes() == page->memory->Size();
    });
    for (auto& chunk : frame.upload) chunk.cursor = 0;
    frame.images.clear(); frame.samplers.clear();
    frame.imageCount = frame.samplerCount = 0;
    frame.submitted = false; frame.serial = nextSerial++;
    dkCmdBufClear(frame.command);
    frame.nextCommandChunk = 1;
    dkCmdBufAddMemory(frame.command, frame.commandMemory[0]->block, 0, frame.commandMemory[0]->Size());
    {

        frame.imageSlot = dkQueueAcquireImage(queue, swapchain);
    }
    if (frame.imageSlot < 0) Fail("Swapchain acquire failed");
    dkCmdBufBindImageDescriptorSet(frame.command, frame.descriptors->Gpu(), ImageDescriptorCount);
    dkCmdBufBindSamplerDescriptorSet(frame.command, frame.descriptors->Gpu() + ImageDescriptorCount * sizeof(DkImageDescriptor), SamplerDescriptorCount);
    dkCmdBufBarrier(frame.command, DkBarrier_None, DkInvalidateFlags_Image | DkInvalidateFlags_Shader | DkInvalidateFlags_Descriptors);
    recording = true;
    InvalidateState();
}
DkCmdBuf Commands() { EnsureFrame(); return Current().command; }
template<class T> void Retain(const std::shared_ptr<T>& resource)
{
    EnsureFrame();
    if (Current().retained.insert(resource.get()).second) Current().keepAlive.push_back(resource);
}
struct Upload { DkGpuAddr gpu; uint8_t* cpu; uint32_t size; };
Upload ReserveUpload(uint32_t size, uint32_t alignment = 256)
{
    EnsureFrame();
    size = Align(std::max(size, 1u), 4);

    for (auto& chunk : Current().upload)
    {
        const uint32_t offset = Align(chunk.cursor, alignment);
        if (uint64_t(offset) + size <= chunk.memory->Size())
        {
            chunk.cursor = offset + size;
            return {chunk.memory->Gpu() + offset, chunk.memory->Cpu() + offset, size};
        }
    }
    auto memory = std::make_shared<Memory>(Align(std::max(UploadChunkSize, size), DK_MEMBLOCK_ALIGNMENT), DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    Current().upload.push_back({memory, size});
    return {memory->Gpu(), memory->Cpu(), size};
}
void Submit()
{
    if (!recording) return;
    dkQueueSubmitCommands(queue, dkCmdBufFinishList(Current().command));
    dkQueueFlush(queue);
}

struct Buffer
{
    uint32_t size{};
    GLenum usage = GL_STATIC_DRAW;
    std::vector<uint8_t> shadow;
    std::shared_ptr<Allocation> allocation;
    uint64_t version = 1, uploadedSerial{}, uploadedVersion{};
    DkGpuAddr uploadedAddress{};
    bool vertexUser{};
};
uint64_t vertexDataEpoch=1, boundVertexDataEpoch{};
struct Attribute
{
    GLuint buffer{};
    uint32_t offset{}, stride{}, divisor{};
    unsigned components = 4;
    GLenum type = GL_FLOAT;
    bool enabled{}, normalized{}, integer{};
};
struct VertexArray { std::array<Attribute, 16> attributes; GLuint indices{}; uint64_t version = 1; };
struct Image
{
    std::shared_ptr<Allocation> allocation;
    DkImageLayout layout{};
    DkImage image{};
    uint32_t width{}, height{}, layers = 1, levels = 1, samples = 1;
    DkImageFormat format = DkImageFormat_RGBA8_Unorm;
};
std::shared_ptr<Image> defaultDepth;
struct SamplerState
{
    GLenum minFilter = GL_NEAREST_MIPMAP_LINEAR, magFilter = GL_LINEAR;
    GLenum wrapS = GL_REPEAT, wrapT = GL_REPEAT;
    unsigned anisotropy = 1;
};
struct Texture : SamplerState
{
    std::shared_ptr<Image> image;
    GLenum target = GL_TEXTURE_2D;
    uint64_t version = 1;
    mutable uint64_t descriptorSerial{}, descriptorVersion{}, descriptorSampler{};
    mutable DkResHandle descriptorHandle{};
    mutable bool descriptorArray{};
};
Texture whiteArray;
struct Attachment { GLuint object{}; bool renderbuffer{}; unsigned layer{}, level{}; };
struct Framebuffer { Attachment color, depth; bool drawColor = true; };
struct Program
{
    const ProgramInfo* info{};
    std::shared_ptr<Memory> code;
    DkShader vertex{}, fragment{};
    std::vector<uint8_t> constants;
    std::array<GLuint, 32> units{};
    struct Sampler { unsigned binding; bool array; };
    std::vector<Sampler> samplers;
    uint64_t version = 1, uploadedSerial{}, uploadedVersion{};
    DkGpuAddr uploadedAddress{};
};
struct Range { GLuint buffer{}; uint32_t offset{}, size{}; bool whole{}; };
struct Sync { DkFence fence{}; };
std::unordered_map<GLuint, Buffer> buffers;
std::unordered_map<GLuint, VertexArray> arrays{{0, {}}};
std::unordered_map<GLuint, Texture> textures;
std::unordered_map<GLuint, SamplerState> samplerObjects;
std::unordered_map<GLuint, std::shared_ptr<Image>> renderbuffers;
std::unordered_map<GLuint, Framebuffer> framebuffers;
std::unordered_map<GLuint, std::shared_ptr<Program>> shaderPrograms;
std::unordered_map<GLsync, std::shared_ptr<Sync>> syncs;
GLuint arrayBuffer{}, uniformBuffer{}, storageBuffer{}, vertexArray{}, currentProgram{}, renderbuffer{}, drawFramebuffer{}, readFramebuffer{}, activeUnit{};
std::array<std::array<GLuint, 2>, 32> textureUnits{};
std::array<GLuint, 32> samplerUnits{};
std::array<Range, 16> uniformRanges{}, storageRanges{};
std::array<std::array<float, 4>, 16> genericAttributes{};
uint64_t genericVersion = 1;
GLint packAlignment = 4;
int unpackAlignment = 4, unpackRowLength = 0;
bool depthTest{}, blendEnable{}, cullEnable{}, stencilTest{}, scissorEnable{};
GLboolean depthWrite = GL_TRUE;
GLenum depthFunction = GL_LESS, frontFace = GL_CCW, cullFace = GL_BACK;
DkPolygonMode polygonMode = DkPolygonMode_Fill;
GLenum blendSource = GL_ONE, blendDestination = GL_ZERO, alphaSource = GL_ONE, alphaDestination = GL_ZERO;
GLenum colorEquation = GL_FUNC_ADD, alphaEquation = GL_FUNC_ADD;
GLenum stencilFunction = GL_ALWAYS, stencilFail = GL_KEEP, stencilDepthFail = GL_KEEP, stencilPass = GL_KEEP;
GLuint stencilMask = 255, stencilCompareMask = 255;
GLint stencilReference{};
unsigned colorMask = 15;
std::array<float, 4> clearColor{};
float clearDepth = 1, pointSize = 1, lineWidth = 1;
GLint clearStencil{};
GLint viewportX{}, viewportY{};
GLsizei viewportWidth{}, viewportHeight{};
double depthNear = 0, depthFar = 1;
std::array<GLint, 4> scissor{};
bool pipelineDirty = true, targetDirty = true, viewportDirty = true, vertexDirty = true;
GLuint boundProgram = ~GLuint(0), boundArray = ~GLuint(0);
uint64_t boundArrayVersion{}, boundGenericVersion{};
std::array<std::array<DkBufExtents, 16>, 2> boundUniforms{}, boundStorage{};
std::array<DkResHandle, 32> boundTextures{};
std::array<DkBufExtents, 16> boundVertexBuffers{};
DkGpuAddr boundIndexAddress = DK_GPU_ADDR_INVALID;
GLenum boundIndexType{};
void InvalidateState()
{
    pipelineDirty = targetDirty = viewportDirty = vertexDirty = true;
    boundProgram = boundArray = ~GLuint(0);
    for (auto& stage : boundUniforms) stage.fill({DK_GPU_ADDR_INVALID, 0});
    for (auto& stage : boundStorage) stage.fill({DK_GPU_ADDR_INVALID, 0});
    boundTextures.fill(~DkResHandle(0)); boundVertexBuffers.fill({DK_GPU_ADDR_INVALID, 0});
    boundIndexAddress = DK_GPU_ADDR_INVALID; boundIndexType = 0;
}
GLuint& BoundBuffer(GLenum target)
{
    if (target == GL_ARRAY_BUFFER) return arrayBuffer;
    if (target == GL_ELEMENT_ARRAY_BUFFER) return arrays.at(vertexArray).indices;
    if (target == GL_UNIFORM_BUFFER) return uniformBuffer;
    if (target == GL_SHADER_STORAGE_BUFFER) return storageBuffer;
    Fail("Unsupported buffer target " + std::to_string(target));
}
Texture& BoundTexture(GLenum target)
{
    const GLuint id = textureUnits.at(activeUnit).at(target == GL_TEXTURE_2D_ARRAY ? 1 : 0);
    if (!id) Fail("Texture operation without a bound texture");
    return textures.at(id);
}
DkMsMode Samples(unsigned samples)
{
    switch (samples) { case 1: return DkMsMode_1x; case 2: return DkMsMode_2x; case 4: return DkMsMode_4x; case 8: return DkMsMode_8x; default: Fail("Unsupported MSAA sample count"); }
}
std::shared_ptr<Image> MakeImage(unsigned width, unsigned height, unsigned layers, unsigned levels, unsigned samples, DkImageFormat format, bool array)
{
    if (!width || !height || !layers || !levels) Fail("Invalid image dimensions");
    auto result = std::make_shared<Image>();
    result->width = width; result->height = height; result->layers = layers; result->levels = levels; result->samples = samples; result->format = format;
    DkImageLayoutMaker maker; dkImageLayoutMakerDefaults(&maker, device);
    maker.type = array ? DkImageType_2DArray : (samples > 1 ? DkImageType_2DMS : DkImageType_2D);
    maker.format = format; maker.dimensions[0] = width; maker.dimensions[1] = height; maker.dimensions[2] = layers;
    maker.mipLevels = levels; maker.msMode = Samples(samples);
    maker.flags = DkImageFlags_UsageRender;
    if (format == DkImageFormat_RGBA8_Unorm) maker.flags |= DkImageFlags_Usage2DEngine;
    dkImageLayoutInitialize(&result->layout, &maker);
    result->allocation = Allocate(dkImageLayoutGetSize(&result->layout), dkImageLayoutGetAlignment(&result->layout), true);
    dkImageInitialize(&result->image, &result->layout, result->allocation->page->memory->block, result->allocation->offset);
    return result;
}
DkImageView View(const std::shared_ptr<Image>& image, unsigned level = 0, unsigned layer = 0, bool attachment = false)
{
    if (!image || level >= image->levels || layer >= image->layers) Fail("Invalid image view");
    DkImageView result; dkImageViewDefaults(&result, &image->image);
    if (attachment)
    {
        result.type = image->samples > 1 ? DkImageType_2DMS : DkImageType_2D;
        result.layerOffset = layer; result.layerCount = 1;
        result.mipLevelOffset = level; result.mipLevelCount = 1;
    }
    Retain(image);
    return result;
}
std::shared_ptr<Image> AttachmentImage(const Attachment& a)
{
    if (!a.object) return {};
    return a.renderbuffer ? renderbuffers.at(a.object) : textures.at(a.object).image;
}

void APIENTRY GenBuffers(GLsizei count, GLuint* ids)
{
    for (int i = 0; i < count; ++i) { ids[i] = nextObject++; buffers.try_emplace(ids[i]); }
}
void APIENTRY BindBuffer(GLenum target, GLuint id)
{
    if (id && !buffers.contains(id)) buffers.try_emplace(id);
    BoundBuffer(target) = id;
}
void APIENTRY BufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
    if (size < 0 || uint64_t(size) > UINT32_MAX) Fail("Invalid buffer size");
    const auto id = BoundBuffer(target);
    if (!id) Fail("BufferData without a buffer");
    auto& b = buffers.at(id);
    if (b.vertexUser) ++vertexDataEpoch;
    b.size = uint32_t(size); b.usage = usage; ++b.version;
    b.allocation.reset(); b.shadow.clear(); b.uploadedSerial = 0;
    if (usage == GL_STATIC_DRAW && size)
    {
        b.allocation = Allocate(uint32_t(size), 256);
        if (data) std::memcpy(b.allocation->Cpu(), data, size);
        else std::memset(b.allocation->Cpu(), 0, b.allocation->size);
    }
    else
    {
        b.shadow.resize(size);
        if (data && size) std::memcpy(b.shadow.data(), data, size);
    }
    if (target == GL_ARRAY_BUFFER || target == GL_ELEMENT_ARRAY_BUFFER) vertexDirty = true;
}
void APIENTRY BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
    auto& b = buffers.at(BoundBuffer(target));
    if (offset < 0 || size < 0 || uint64_t(offset) + uint64_t(size) > b.size || (size && !data)) Fail("BufferSubData outside its allocation");
    if (!size) return;
    if (b.allocation)
    {

        auto upload = ReserveUpload(uint32_t(size));
        std::memcpy(upload.cpu, data, size);
        Retain(b.allocation);
        dkCmdBufBarrier(Commands(), DkBarrier_Primitives, 0);
        dkCmdBufCopyBuffer(Commands(), upload.gpu, b.allocation->Gpu() + offset, uint32_t(size));
        dkCmdBufBarrier(Commands(), DkBarrier_None, DkInvalidateFlags_Shader | DkInvalidateFlags_L2Cache);
    }
    else std::memcpy(b.shadow.data() + offset, data, size);
    ++b.version;
    if (b.vertexUser) ++vertexDataEpoch;
}
void APIENTRY DeleteBuffers(GLsizei count, const GLuint* ids)
{
    for (int i = 0; i < count; ++i)
    {
        const GLuint id = ids[i];
        if (!id) continue;
        buffers.erase(id);
        if (arrayBuffer == id) arrayBuffer = 0;
        if (uniformBuffer == id) uniformBuffer = 0;
        if (storageBuffer == id) storageBuffer = 0;
        for (auto& [name, vao] : arrays)
        {
            if (vao.indices == id) vao.indices = 0;
            for (auto& a : vao.attributes) if (a.buffer == id) a.buffer = 0;
            ++vao.version;
        }
        for (auto& r : uniformRanges) if (r.buffer == id) r = {};
        for (auto& r : storageRanges) if (r.buffer == id) r = {};
    }
    vertexDirty = true;
}
void APIENTRY BindBufferRange(GLenum target, GLuint index, GLuint id, GLintptr offset, GLsizeiptr size)
{
    if (target != GL_UNIFORM_BUFFER && target != GL_SHADER_STORAGE_BUFFER) Fail("Unsupported indexed buffer target");
    if (index >= 16 || offset < 0 || size < 0 || uint64_t(offset) + uint64_t(size) > UINT32_MAX) Fail("Invalid indexed buffer range");
    if (id && uint64_t(offset) + uint64_t(size) > buffers.at(id).size) Fail("Indexed buffer range exceeds storage");
    auto& ranges = target == GL_UNIFORM_BUFFER ? uniformRanges : storageRanges;
    ranges[index] = {id, uint32_t(offset), uint32_t(size)};
    BoundBuffer(target) = id;
}
void APIENTRY BindBufferBase(GLenum target, GLuint index, GLuint id)
{
    BindBufferRange(target, index, id, 0, id ? buffers.at(id).size : 0);
    (target == GL_UNIFORM_BUFFER ? uniformRanges : storageRanges)[index].whole = true;
}
uint32_t RangeSize(const Range& range)
{
    if (!range.buffer) return 0;
    const auto& buffer = buffers.at(range.buffer);
    const auto size = range.whole ? buffer.size : range.size;
    if (uint64_t(range.offset) + size > buffer.size) Fail("Indexed buffer storage shrank below its bound range");
    return size;
}
GLboolean APIENTRY UnmapBuffer(GLenum) { return GL_TRUE; }
DkBufExtents ResolveBuffer(GLuint id)
{
    if (!id) return {0, 0};
    auto& b = buffers.at(id);
    if (!b.size) return {0, 0};
    if (b.allocation)
    {
        Retain(b.allocation);
        return {b.allocation->Gpu(), b.size};
    }
    if (b.uploadedSerial != Current().serial || b.uploadedVersion != b.version)
    {

        auto upload = ReserveUpload(b.size);
        std::memcpy(upload.cpu, b.shadow.data(), b.size);
        b.uploadedAddress = upload.gpu; b.uploadedSerial = Current().serial; b.uploadedVersion = b.version;
    }
    return {b.uploadedAddress, b.size};
}
struct ResolvedRange { Range range{}; uint64_t version{}, serial{}; DkBufExtents extent{}; };
std::array<ResolvedRange, 16> resolvedUniforms;
DkBufExtents ResolveUniform(unsigned index)
{
    auto r = uniformRanges[index];
    r.size = RangeSize(r);
    if (!r.buffer || !r.size) return {0, 0};
    auto& b = buffers.at(r.buffer);
    if (b.allocation)
    {
        Retain(b.allocation);
        return {b.allocation->Gpu() + r.offset, r.size};
    }
    auto& cached = resolvedUniforms[index];
    if (cached.serial != Current().serial || cached.version != b.version || cached.range.buffer != r.buffer || cached.range.offset != r.offset || cached.range.size != r.size)
    {

        auto upload = ReserveUpload(Align(r.size, DK_UNIFORM_BUF_ALIGNMENT));
        std::memcpy(upload.cpu, b.shadow.data() + r.offset, r.size);
        if (upload.size > r.size) std::memset(upload.cpu + r.size, 0, upload.size - r.size);
        cached = {r, b.version, Current().serial, {upload.gpu, Align(r.size, 16)}};
    }
    return cached.extent;
}
void APIENTRY GenVertexArrays(GLsizei count, GLuint* ids)
{
    for (int i = 0; i < count; ++i) { ids[i] = nextObject++; arrays.try_emplace(ids[i]); }
}
void APIENTRY BindVertexArray(GLuint id)
{
    if (!arrays.contains(id)) Fail("Binding an unknown vertex array");
    if (vertexArray != id) { vertexArray = id; vertexDirty = true; }
}
void APIENTRY DeleteVertexArrays(GLsizei count, const GLuint* ids)
{
    for (int i = 0; i < count; ++i) if (ids[i])
    { arrays.erase(ids[i]); if (vertexArray == ids[i]) vertexArray = 0; }
    vertexDirty = true;
}
unsigned ComponentBytes(GLenum type)
{
    switch (type) { case GL_FLOAT: case GL_INT: case GL_UNSIGNED_INT: return 4;
    case GL_HALF_FLOAT: case GL_SHORT: case GL_UNSIGNED_SHORT: return 2;
    case GL_BYTE: case GL_UNSIGNED_BYTE: return 1; default: Fail("Unsupported vertex component type"); }
}
void SetAttribute(GLuint index, GLint components, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer, bool integer)
{
    if (index >= 15 || components < 1 || components > 4 || stride < 0 || reinterpret_cast<uintptr_t>(pointer) > UINT32_MAX) Fail("Invalid vertex attribute");
    auto& vao = arrays.at(vertexArray);
    auto& a = vao.attributes[index];
    if (arrayBuffer) buffers.at(arrayBuffer).vertexUser = true;
    a.buffer = arrayBuffer; a.components = components; a.type = type; a.normalized = normalized; a.integer = integer;
    a.offset = uint32_t(reinterpret_cast<uintptr_t>(pointer));
    a.stride = stride ? uint32_t(stride) : components * ComponentBytes(type);
    ++vao.version; vertexDirty = true;
}
void APIENTRY VertexAttribPointer(GLuint index, GLint components, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{ SetAttribute(index, components, type, normalized, stride, pointer, false); }
void APIENTRY VertexAttribIPointer(GLuint index, GLint components, GLenum type, GLsizei stride, const void* pointer)
{ SetAttribute(index, components, type, false, stride, pointer, true); }
void APIENTRY EnableVertexAttribArray(GLuint index)
{ auto& vao = arrays.at(vertexArray); vao.attributes.at(index).enabled = true; ++vao.version; vertexDirty = true; }
void APIENTRY DisableVertexAttribArray(GLuint index)
{ auto& vao = arrays.at(vertexArray); vao.attributes.at(index).enabled = false; ++vao.version; vertexDirty = true; }
void APIENTRY VertexAttribDivisor(GLuint index, GLuint divisor)
{ auto& vao = arrays.at(vertexArray); vao.attributes.at(index).divisor = divisor; ++vao.version; vertexDirty = true; }
void APIENTRY VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{ genericAttributes.at(index) = {x,y,z,w}; ++genericVersion; vertexDirty = true; }
void APIENTRY VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) { VertexAttrib4f(index,x,y,z,1); }
void APIENTRY VertexAttrib2f(GLuint index, GLfloat x, GLfloat y) { VertexAttrib4f(index,x,y,0,1); }

void APIENTRY UseProgram(GLuint id)
{
    if (id && !shaderPrograms.contains(id)) Fail("Unknown shader program");
    currentProgram = id;
}
void APIENTRY DeleteProgram(GLuint id)
{
    shaderPrograms.erase(id);
    if (currentProgram == id) currentProgram = 0;
}
GLint APIENTRY GetUniformLocation(GLuint id, const GLchar* name)
{
    if (!name) return -1;
    const auto& p = *shaderPrograms.at(id);
    std::string key(name);
    if (key.ends_with("[0]")) key.resize(key.size()-3);
    for (unsigned i = 0; i < p.info->uniformCount; ++i)
        if (key == p.info->uniforms[i].name) return GLint(i);
    return -1;
}
void WriteUniform(GLint location, const void* data, unsigned components, unsigned count, bool integer, unsigned matrix = 0, bool transpose = false)
{
    if (location < 0) return;
    auto& p = *shaderPrograms.at(currentProgram);
    if (unsigned(location) >= p.info->uniformCount || !data) Fail("Invalid program uniform");
    const auto& u = p.info->uniforms[location];
    if (!std::strncmp(u.type, "sampler", 7))
    {
        if (!integer || count != 1 || components != 1) Fail("Invalid sampler uniform");
        const auto unit = *static_cast<const GLint*>(data);
        if (unit < 0 || unit >= 32) Fail("Sampler texture unit out of range");
        p.units[u.offset] = unit; return;
    }
    if (count > u.count) Fail("Uniform array upload exceeds its declared length");
    const unsigned stride = u.count > 1 ? u.stride : u.size;
    auto* target = p.constants.data() + u.offset;
    const auto* source = static_cast<const uint8_t*>(data);
    bool changed = false;
    auto copy = [&](uint8_t* destination, const void* value, unsigned bytes)
    {
        if (std::memcmp(destination, value, bytes))
        { std::memcpy(destination, value, bytes); changed = true; }
    };
    for (unsigned n = 0; n < count; ++n)
    {
        if (matrix)
        {
            if (u.size / u.count < matrix * 16) Fail("Uniform matrix size mismatch");
            const auto* values = reinterpret_cast<const float*>(source) + n * matrix * matrix;
            for (unsigned column = 0; column < matrix; ++column)
                for (unsigned row = 0; row < matrix; ++row)
                {
                    const float v = values[transpose ? row*matrix+column : column*matrix+row];
                    copy(target + n*stride + column*16 + row*4, &v, 4);
                }
        }
        else
        {
            if (components * 4 > stride) Fail("Uniform component size mismatch");
            copy(target + n*stride, source + n*components*4, components*4);
        }
    }
    if (changed) ++p.version;
}
void APIENTRY Uniform1i(GLint l, GLint x) { WriteUniform(l,&x,1,1,true); }
void APIENTRY Uniform1f(GLint l, GLfloat x) { WriteUniform(l,&x,1,1,false); }
void APIENTRY Uniform2f(GLint l, GLfloat x, GLfloat y) { const float v[]{x,y}; WriteUniform(l,v,2,1,false); }
void APIENTRY Uniform3f(GLint l, GLfloat x, GLfloat y, GLfloat z) { const float v[]{x,y,z}; WriteUniform(l,v,3,1,false); }
void APIENTRY Uniform4f(GLint l, GLfloat x, GLfloat y, GLfloat z, GLfloat w) { const float v[]{x,y,z,w}; WriteUniform(l,v,4,1,false); }
void APIENTRY Uniform1fv(GLint l, GLsizei n, const GLfloat* v) { WriteUniform(l,v,1,n,false); }
void APIENTRY Uniform3fv(GLint l, GLsizei n, const GLfloat* v) { WriteUniform(l,v,3,n,false); }
void APIENTRY Uniform4fv(GLint l, GLsizei n, const GLfloat* v) { WriteUniform(l,v,4,n,false); }
void APIENTRY UniformMatrix3fv(GLint l, GLsizei n, GLboolean t, const GLfloat* v) { WriteUniform(l,v,3,n,false,3,t); }
void APIENTRY UniformMatrix4fv(GLint l, GLsizei n, GLboolean t, const GLfloat* v) { WriteUniform(l,v,4,n,false,4,t); }
GLuint APIENTRY GetUniformBlockIndex(GLuint, const GLchar* name)
{
    if (!std::strcmp(name,"CMGL")) return 0;
    if (!std::strcmp(name,"BlipGroup") || !std::strcmp(name,"RO") || !std::strcmp(name,"ROCEL")) return 1;
    return GL_INVALID_INDEX;
}
void APIENTRY UniformBlockBinding(GLuint, GLuint block, GLuint binding)
{ if (block != binding) Fail("The precompiled shader requires its declared UBO binding"); }

DkCompareOp Compare(GLenum op)
{
    switch (op) { case GL_NEVER: return DkCompareOp_Never; case GL_LESS: return DkCompareOp_Less;
    case GL_EQUAL: return DkCompareOp_Equal; case GL_LEQUAL: return DkCompareOp_Lequal;
    case GL_GREATER: return DkCompareOp_Greater; case GL_NOTEQUAL: return DkCompareOp_NotEqual;
    case GL_GEQUAL: return DkCompareOp_Gequal; case GL_ALWAYS: return DkCompareOp_Always;
    default: Fail("Unsupported comparison operation"); }
}
DkBlendFactor BlendFactor(GLenum factor)
{
    switch (factor) { case GL_ZERO: return DkBlendFactor_Zero; case GL_ONE: return DkBlendFactor_One;
    case GL_SRC_ALPHA: return DkBlendFactor_SrcAlpha; case GL_ONE_MINUS_SRC_ALPHA: return DkBlendFactor_InvSrcAlpha;
    case GL_DST_ALPHA: return DkBlendFactor_DstAlpha; case GL_ONE_MINUS_DST_ALPHA: return DkBlendFactor_InvDstAlpha;
    case GL_SRC_COLOR: return DkBlendFactor_SrcColor; case GL_ONE_MINUS_SRC_COLOR: return DkBlendFactor_InvSrcColor;
    case GL_DST_COLOR: return DkBlendFactor_DstColor; case GL_ONE_MINUS_DST_COLOR: return DkBlendFactor_InvDstColor;
    default: Fail("Unsupported blend factor"); }
}
DkBlendOp BlendOp(GLenum op)
{
    switch (op) { case GL_FUNC_ADD: return DkBlendOp_Add; case GL_FUNC_SUBTRACT: return DkBlendOp_Sub;
    case GL_FUNC_REVERSE_SUBTRACT: return DkBlendOp_RevSub; case GL_MIN: return DkBlendOp_Min;
    case GL_MAX: return DkBlendOp_Max; default: Fail("Unsupported blend operation"); }
}
DkStencilOp StencilOp(GLenum op)
{
    switch (op) { case GL_KEEP: return DkStencilOp_Keep; case GL_ZERO: return DkStencilOp_Zero;
    case GL_REPLACE: return DkStencilOp_Replace; case GL_INCR: return DkStencilOp_Incr;
    case GL_DECR: return DkStencilOp_Decr; case GL_INVERT: return DkStencilOp_Invert;
    case GL_INCR_WRAP: return DkStencilOp_IncrWrap; case GL_DECR_WRAP: return DkStencilOp_DecrWrap;
    default: Fail("Unsupported stencil operation"); }
}
bool* Capability(GLenum cap)
{
    switch (cap) { case GL_DEPTH_TEST: return &depthTest; case GL_BLEND: return &blendEnable;
    case GL_CULL_FACE: return &cullEnable; case GL_STENCIL_TEST: return &stencilTest;
    case GL_SCISSOR_TEST: return &scissorEnable;
    case GL_DEBUG_OUTPUT: case GL_DEBUG_OUTPUT_SYNCHRONOUS: return nullptr;
    default: Fail("Unsupported render capability " + std::to_string(cap)); }
}
void SetCapability(GLenum cap, bool enable)
{ if (bool* state = Capability(cap); state && *state != enable) { *state = enable; pipelineDirty = true; viewportDirty = true; } }
void APIENTRY Enable(GLenum cap) { SetCapability(cap, true); }
void APIENTRY Disable(GLenum cap) { SetCapability(cap, false); }
GLboolean APIENTRY IsEnabled(GLenum cap) { const auto* state = Capability(cap); return state && *state; }
void APIENTRY DepthMask(GLboolean value) { if (depthWrite != value) { depthWrite = value; pipelineDirty = true; } }
void APIENTRY DepthFunc(GLenum value) { if (depthFunction != value) { depthFunction = value; pipelineDirty = true; } }
void APIENTRY FrontFace(GLenum value) { if (frontFace != value) { frontFace = value; pipelineDirty = true; } }
void APIENTRY CullFace(GLenum value) { if (cullFace != value) { cullFace = value; pipelineDirty = true; } }
void APIENTRY BlendFuncSeparate(GLenum src, GLenum dst, GLenum asrc, GLenum adst)
{
    if (src != blendSource || dst != blendDestination || asrc != alphaSource || adst != alphaDestination)
    { blendSource = src; blendDestination = dst; alphaSource = asrc; alphaDestination = adst; pipelineDirty = true; }
}
void APIENTRY BlendFunc(GLenum src, GLenum dst) { BlendFuncSeparate(src,dst,src,dst); }
void APIENTRY BlendEquationSeparate(GLenum color, GLenum alpha)
{ if (color != colorEquation || alpha != alphaEquation) { colorEquation = color; alphaEquation = alpha; pipelineDirty = true; } }
void APIENTRY BlendEquation(GLenum op) { BlendEquationSeparate(op,op); }
void APIENTRY ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{ const unsigned mask = (r?1:0)|(g?2:0)|(b?4:0)|(a?8:0); if (colorMask != mask) { colorMask = mask; pipelineDirty = true; } }
void APIENTRY StencilFunc(GLenum function, GLint reference, GLuint mask)
{ stencilFunction = function; stencilReference = reference; stencilCompareMask = mask; pipelineDirty = true; }
void APIENTRY StencilMask(GLuint mask) { if (stencilMask != mask) { stencilMask = mask; pipelineDirty = true; } }
void APIENTRY StencilOp(GLenum fail, GLenum depthFail, GLenum pass)
{ stencilFail = fail; stencilDepthFail = depthFail; stencilPass = pass; pipelineDirty = true; }
void APIENTRY Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{ viewportX = x; viewportY = y; viewportWidth = width; viewportHeight = height; viewportDirty = true; }
void APIENTRY DepthRange(GLdouble nearValue, GLdouble farValue)
{ depthNear = std::clamp(nearValue,0.0,1.0); depthFar = std::clamp(farValue,0.0,1.0); viewportDirty = true; }
void APIENTRY Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{ scissor = {x,y,width,height}; viewportDirty = true; }
void APIENTRY PointSize(GLfloat size) { pointSize = size; pipelineDirty = true; }
void APIENTRY LineWidth(GLfloat width) { lineWidth = width; pipelineDirty = true; }
void APIENTRY ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { clearColor = {r,g,b,a}; }
void APIENTRY ClearDepth(GLdouble value) { clearDepth = float(std::clamp(value,0.0,1.0)); }
void APIENTRY ClearStencil(GLint value) { clearStencil = value; }
void APIENTRY MemoryBarrier(GLbitfield bits)
{ if (bits) dkCmdBufBarrier(Commands(), DkBarrier_Primitives, DkInvalidateFlags_Shader | DkInvalidateFlags_Image); }

void APIENTRY GenTextures(GLsizei count, GLuint* ids)
{ for (int i=0;i<count;++i) { ids[i] = nextObject++; textures.try_emplace(ids[i]); } }
void APIENTRY ActiveTexture(GLenum unit)
{ if (unit < GL_TEXTURE0 || unit >= GL_TEXTURE0 + 32) Fail("Texture unit out of range"); activeUnit = unit - GL_TEXTURE0; }
void APIENTRY BindTexture(GLenum target, GLuint id)
{
    if (target != GL_TEXTURE_2D && target != GL_TEXTURE_2D_ARRAY) Fail("Unsupported texture target");
    if (id && !textures.contains(id)) textures.try_emplace(id);
    if (id) textures.at(id).target = target;
    textureUnits[activeUnit][target == GL_TEXTURE_2D_ARRAY ? 1 : 0] = id;
}
void APIENTRY DeleteTextures(GLsizei count, const GLuint* ids)
{
    for (int i=0;i<count;++i) if (ids[i])
    {
        textures.erase(ids[i]);
        for (auto& unit : textureUnits) for (auto& id : unit) if (id == ids[i]) id = 0;
        for (auto& [name,f] : framebuffers)
        {
            if (!f.color.renderbuffer && f.color.object == ids[i]) f.color = {};
            if (!f.depth.renderbuffer && f.depth.object == ids[i]) f.depth = {};
        }
    }
    targetDirty = true;
}
void APIENTRY TexParameteri(GLenum target, GLenum parameter, GLint value)
{
    auto& t = BoundTexture(target);
    auto set = [&](GLenum& field) { if (field != GLenum(value)) { field = value; ++t.version; } };
    switch (parameter)
    {
        case GL_TEXTURE_MIN_FILTER: set(t.minFilter); break;
        case GL_TEXTURE_MAG_FILTER: set(t.magFilter); break;
        case GL_TEXTURE_WRAP_S: set(t.wrapS); break;
        case GL_TEXTURE_WRAP_T: set(t.wrapT); break;
        case GL_TEXTURE_BASE_LEVEL: if (value != 0) Fail("Nonzero texture base level unsupported"); break;
        case GL_TEXTURE_MAX_LEVEL: break;
        default: Fail("Unsupported texture parameter");
    }
}
void APIENTRY TexParameterf(GLenum target, GLenum parameter, GLfloat value)
{
    if (parameter != 0x84FE || !std::isfinite(value)) Fail("Unsupported floating texture parameter");
    auto& texture = BoundTexture(target);
    texture.anisotropy = unsigned(std::clamp(value, 1.0f, 16.0f));
    ++texture.version;
}
void APIENTRY GenSamplers(GLsizei count, GLuint* ids)
{ for (int i=0;i<count;++i) { ids[i]=nextObject++;samplerObjects.try_emplace(ids[i]); } }
void APIENTRY BindSampler(GLuint unit, GLuint id)
{
    if (unit>=samplerUnits.size() || (id && !samplerObjects.contains(id))) Fail("Invalid sampler binding");
    samplerUnits[unit]=id;
}
void APIENTRY SamplerParameteri(GLuint id, GLenum parameter, GLint value)
{
    auto& sampler=samplerObjects.at(id);
    switch (parameter)
    {
        case GL_TEXTURE_MIN_FILTER: sampler.minFilter=value;break;
        case GL_TEXTURE_MAG_FILTER: sampler.magFilter=value;break;
        case GL_TEXTURE_WRAP_S: sampler.wrapS=value;break;
        case GL_TEXTURE_WRAP_T: sampler.wrapT=value;break;
        default: Fail("Unsupported sampler parameter");
    }
}
void APIENTRY DeleteSamplers(GLsizei count, const GLuint* ids)
{
    for (int i=0;i<count;++i) if (ids[i])
    {
        samplerObjects.erase(ids[i]);
        for (auto& unit:samplerUnits) if (unit==ids[i]) unit=0;
    }
}
void APIENTRY PixelStorei(GLenum parameter, GLint value)
{
    if (parameter == GL_UNPACK_ALIGNMENT)
    { if (value != 1 && value != 2 && value != 4 && value != 8) Fail("Invalid pixel alignment"); unpackAlignment = value; }
    else if (parameter == GL_PACK_ALIGNMENT) { if (value != 1 && value != 2 && value != 4 && value != 8) Fail("Invalid pixel alignment"); packAlignment = value; }
    else if (parameter == GL_UNPACK_ROW_LENGTH) { if (value < 0) Fail("Negative pixel row length"); unpackRowLength = value; }
    else Fail("Unsupported pixel storage setting");
}
void UploadPixels(const std::shared_ptr<Image>& image, unsigned level, unsigned x, unsigned y, unsigned width, unsigned height, GLenum format, GLenum type, const void* pixels)
{
    if (!pixels || !width || !height) return;
    if (!image || level >= image->levels || image->format != DkImageFormat_RGBA8_Unorm || type != GL_UNSIGNED_BYTE) Fail("Unsupported texture upload");
    if (uint64_t(x) + width > std::max(1u,image->width >> level) || uint64_t(y) + height > std::max(1u,image->height >> level)) Fail("Texture upload exceeds mip level");
    const unsigned components = format == GL_RGBA || format == GL_BGRA ? 4 : format == GL_RGB || format == GL_BGR ? 3 : format == GL_RED ? 1 : 0;
    if (!components) Fail("Unsupported pixel format");
    const uint32_t sourcePitch = Align((unpackRowLength ? unpackRowLength : width) * components, unpackAlignment);
    const uint32_t pitch = Align(width * 4, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
    const uint64_t bytes = uint64_t(pitch) * height;
    if (bytes > UINT32_MAX) Fail("Texture upload too large");
    auto upload = ReserveUpload(uint32_t(bytes));
    const auto* source = static_cast<const uint8_t*>(pixels);
    for (unsigned row=0;row<height;++row)
    {
        auto* dst = upload.cpu + row*pitch;
        const auto* src = source + row*sourcePitch;
        if (format == GL_RGBA) std::memcpy(dst,src,width*4);
        else for (unsigned column=0;column<width;++column)
        {
            const auto* p = src + column*components;
            dst[column*4+0] = p[(format==GL_BGRA || format==GL_BGR) ? 2 : 0];
            dst[column*4+1] = components >= 3 ? p[1] : 0;
            dst[column*4+2] = components >= 3 ? p[(format==GL_BGRA || format==GL_BGR) ? 0 : 2] : 0;
            dst[column*4+3] = components == 4 ? p[3] : 255;
        }
    }
    auto view = View(image,level,0,true);
    const DkCopyBuf src{upload.gpu,pitch,0};
    const DkImageRect rect{x,y,0,width,height,1};

    dkCmdBufBarrier(Commands(),DkBarrier_Primitives,0);
    dkCmdBufCopyBufferToImage(Commands(),&src,&view,&rect,0);
    dkCmdBufBarrier(Commands(),DkBarrier_None,DkInvalidateFlags_Image);
}
void APIENTRY TexStorage2D(GLenum target, GLsizei levels, GLenum format, GLsizei width, GLsizei height)
{
    if ((format != GL_RGBA8 && format != 0x83F0 && format != 0x83F3) || target != GL_TEXTURE_2D || levels <= 0 || width <= 0 || height <= 0) Fail("Invalid texture storage");
    auto& texture = BoundTexture(target);
    texture.image = MakeImage(width,height,1,levels,1,DkImageFormat_RGBA8_Unorm,false);
    ++texture.version;
    targetDirty = true;
}
void APIENTRY TexStorage3D(GLenum target, GLsizei levels, GLenum format, GLsizei width, GLsizei height, GLsizei layers)
{
    if (format != GL_RGBA8 || target != GL_TEXTURE_2D_ARRAY || levels <= 0 || width <= 0 || height <= 0 || layers <= 0) Fail("Invalid texture array storage");
    auto& texture = BoundTexture(target);
    texture.image = MakeImage(width,height,layers,levels,1,DkImageFormat_RGBA8_Unorm,true);
    ++texture.version;
    targetDirty = true;
}
void APIENTRY TexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
    if (target != GL_TEXTURE_2D || level < 0 || border || width <= 0 || height <= 0 || (internalFormat != GL_RGBA8 && internalFormat != GL_RGBA)) Fail("Unsupported texture image");
    auto& t = BoundTexture(target);
    if (level == 0) { t.image = MakeImage(width,height,1,1,1,DkImageFormat_RGBA8_Unorm,false); ++t.version; }
    else if (!t.image || unsigned(level) >= t.image->levels) Fail("Mip level needs immutable storage");
    UploadPixels(t.image,level,0,0,width,height,format,type,pixels);
    targetDirty = true;
}
void APIENTRY TexSubImage2D(GLenum target, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
    if (level < 0 || x < 0 || y < 0 || width < 0 || height < 0) Fail("Invalid texture subimage");
    UploadPixels(BoundTexture(target).image,level,x,y,width,height,format,type,pixels);
}
void APIENTRY CompressedTexSubImage2D(GLenum target, GLint level, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLsizei size, const void* data)
{
    if (target != GL_TEXTURE_2D || (format != 0x83F0 && format != 0x83F3) || level < 0 ||
        x < 0 || y < 0 || width <= 0 || height <= 0 || !data ||
        uint64_t((width + 3) / 4) * ((height + 3) / 4) * (format == 0x83F3 ? 16 : 8) != uint64_t(size))
        Fail("Invalid compressed texture upload");
    const auto pixels = DecodeBc(static_cast<const uint8_t*>(data), width, height, format == 0x83F3);
    const int alignment = unpackAlignment, rowLength = unpackRowLength;
    unpackAlignment = 1; unpackRowLength = 0;
    TexSubImage2D(target, level, x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    unpackAlignment = alignment; unpackRowLength = rowLength;
}
void APIENTRY GenerateMipmap(GLenum target)
{
    auto& t = BoundTexture(target);
    auto image = t.image;
    if (!image || image->layers != 1 || image->samples != 1) Fail("Unsupported mipmap target");
    unsigned levels = 1;
    for (unsigned n=std::max(image->width,image->height);n>1;n>>=1) ++levels;
    if (image->levels < levels)
    {
        auto replacement = MakeImage(image->width,image->height,1,levels,1,image->format,false);
        auto src=View(image,0,0,true), dst=View(replacement,0,0,true);
        const DkImageRect rect{0,0,0,image->width,image->height,1};
        dkCmdBufBarrier(Commands(),DkBarrier_Primitives,0);
        dkCmdBufCopyImage(Commands(),&src,&rect,&dst,&rect,0);
        image = t.image = replacement; ++t.version; targetDirty = true;
    }
    for (unsigned level=1;level<image->levels;++level)
    {
        auto src=View(image,level-1,0,true),dst=View(image,level,0,true);
        const DkImageRect srcRect{0,0,0,std::max(1u,image->width>>(level-1)),std::max(1u,image->height>>(level-1)),1};
        const DkImageRect dstRect{0,0,0,std::max(1u,image->width>>level),std::max(1u,image->height>>level),1};
        dkCmdBufBarrier(Commands(),DkBarrier_Primitives,0);
        dkCmdBufBlitImage(Commands(),&src,&srcRect,&dst,&dstRect,DkBlitFlag_FilterLinear,0);
    }
    dkCmdBufBarrier(Commands(),DkBarrier_None,DkInvalidateFlags_Image);
}
void APIENTRY GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void* pixels)
{
    const auto image = BoundTexture(target).image;
    if (target != GL_TEXTURE_2D || !image || level < 0 || unsigned(level) >= image->levels ||
        image->format != DkImageFormat_RGBA8_Unorm || format != GL_RGBA || type != GL_UNSIGNED_BYTE || !pixels)
        Fail("Unsupported texture readback");
    const unsigned width = std::max(1u, image->width >> level), height = std::max(1u, image->height >> level);
    const unsigned pitch = Align(width * 4, DK_IMAGE_LINEAR_STRIDE_ALIGNMENT);
    auto readback = std::make_shared<Memory>(pitch * height, DkMemBlockFlags_CpuUncached | DkMemBlockFlags_GpuCached);
    auto view = View(image, level, 0, true);
    const DkCopyBuf dst{readback->Gpu(), pitch, 0};
    const DkImageRect rect{0, 0, 0, width, height, 1};
    dkCmdBufBarrier(Commands(), DkBarrier_Primitives, 0);
    dkCmdBufCopyImageToBuffer(Commands(), &view, &rect, &dst, 0);
    Submit();
    dkQueueWaitIdle(queue);
    const unsigned outputPitch = Align(width * 4, packAlignment);
    for (unsigned y = 0; y < height; ++y)
        std::memcpy(static_cast<uint8_t*>(pixels) + y * outputPitch, readback->Cpu() + y * pitch, width * 4);
}
void APIENTRY GetTexLevelParameteriv(GLenum target, GLint level, GLenum parameter, GLint* value)
{
    const auto& image=BoundTexture(target).image;
    if (!value) return;
    if (!image || level < 0 || unsigned(level) >= image->levels) { *value=0;return; }
    if (parameter==GL_TEXTURE_WIDTH) *value=std::max(1u,image->width>>level);
    else if (parameter==GL_TEXTURE_HEIGHT) *value=std::max(1u,image->height>>level);
    else Fail("Unsupported texture level query");
}
DkResHandle TextureHandle(const Texture& texture, bool array, const SamplerState* override = nullptr)
{
    EnsureFrame();
    const auto& sampling=override ? *override : static_cast<const SamplerState&>(texture);
    const uint64_t samplerKey=uint64_t(sampling.minFilter) | (uint64_t(sampling.magFilter)<<16) |
        (uint64_t(sampling.wrapS==GL_CLAMP_TO_EDGE)<<32) | (uint64_t(sampling.wrapT==GL_CLAMP_TO_EDGE)<<33) |
        (uint64_t(sampling.anisotropy)<<34);
    if (texture.descriptorSerial == Current().serial && texture.descriptorVersion == texture.version &&
        texture.descriptorArray == array && texture.descriptorSampler == samplerKey)
    return texture.descriptorHandle;

    const auto& image = texture.image ? texture.image : array ? whiteArray.image : textures.at(0).image;
    const uint64_t imageKey = image->allocation->Gpu();
    auto& f=Current();
    unsigned imageId;
    if (auto it=f.images.find(imageKey);it!=f.images.end()) imageId=it->second;
    else
    {
        if (f.imageCount==ImageDescriptorCount) Fail("Frame image descriptor capacity exceeded");
        imageId=f.imageCount++;
        DkImageDescriptor descriptor;
        auto view=View(image);
        dkImageDescriptorInitialize(&descriptor,&view,false,false);
        dkCmdBufPushData(Commands(),f.descriptors->Gpu()+imageId*sizeof(descriptor),&descriptor,sizeof(descriptor));
        f.images.emplace(imageKey,imageId);
        dkCmdBufBarrier(Commands(),DkBarrier_None,DkInvalidateFlags_Descriptors);
    }
    unsigned samplerId;
    if (auto it=f.samplers.find(samplerKey);it!=f.samplers.end()) samplerId=it->second;
    else
    {
        if (f.samplerCount==SamplerDescriptorCount) Fail("Frame sampler descriptor capacity exceeded");
        samplerId=f.samplerCount++;
        DkSampler sampler; dkSamplerDefaults(&sampler);
        sampler.maxAnisotropy = float(sampling.anisotropy);
        sampler.minFilter=(sampling.minFilter==GL_NEAREST || sampling.minFilter==GL_NEAREST_MIPMAP_NEAREST || sampling.minFilter==GL_NEAREST_MIPMAP_LINEAR) ? DkFilter_Nearest : DkFilter_Linear;
        sampler.magFilter=sampling.magFilter==GL_NEAREST ? DkFilter_Nearest : DkFilter_Linear;
        sampler.mipFilter=(sampling.minFilter==GL_NEAREST || sampling.minFilter==GL_LINEAR) ? DkMipFilter_None : (sampling.minFilter==GL_NEAREST_MIPMAP_NEAREST || sampling.minFilter==GL_LINEAR_MIPMAP_NEAREST) ? DkMipFilter_Nearest : DkMipFilter_Linear;
        sampler.wrapMode[0]=sampling.wrapS==GL_CLAMP_TO_EDGE ? DkWrapMode_ClampToEdge : DkWrapMode_Repeat;
        sampler.wrapMode[1]=sampling.wrapT==GL_CLAMP_TO_EDGE ? DkWrapMode_ClampToEdge : DkWrapMode_Repeat;
        DkSamplerDescriptor descriptor; dkSamplerDescriptorInitialize(&descriptor,&sampler);
        dkCmdBufPushData(Commands(),f.descriptors->Gpu()+ImageDescriptorCount*sizeof(DkImageDescriptor)+samplerId*sizeof(descriptor),&descriptor,sizeof(descriptor));
        f.samplers.emplace(samplerKey,samplerId);
        dkCmdBufBarrier(Commands(),DkBarrier_None,DkInvalidateFlags_Descriptors);
    }
    texture.descriptorSerial = Current().serial;
    texture.descriptorVersion = texture.version;
    texture.descriptorSampler = samplerKey;
    texture.descriptorArray = array;
    texture.descriptorHandle = dkMakeTextureHandle(imageId,samplerId);
    return texture.descriptorHandle;
}

void APIENTRY GenFramebuffers(GLsizei count, GLuint* ids)
{ for (int i=0;i<count;++i) { ids[i]=nextObject++; framebuffers.try_emplace(ids[i]); } }
void APIENTRY BindFramebuffer(GLenum target, GLuint id)
{
    if (id && !framebuffers.contains(id)) framebuffers.try_emplace(id);
    if (target==GL_FRAMEBUFFER || target==GL_DRAW_FRAMEBUFFER)
    { if (drawFramebuffer!=id) { drawFramebuffer=id;targetDirty=true; } }
    if (target==GL_FRAMEBUFFER || target==GL_READ_FRAMEBUFFER) readFramebuffer=id;
}
void APIENTRY DeleteFramebuffers(GLsizei count, const GLuint* ids)
{
    for (int i=0;i<count;++i) if (ids[i])
    { framebuffers.erase(ids[i]); if (drawFramebuffer==ids[i]) drawFramebuffer=0; if (readFramebuffer==ids[i]) readFramebuffer=0; }
    targetDirty=true;
}
Framebuffer& BoundFramebuffer(GLenum target)
{
    const auto id = target==GL_READ_FRAMEBUFFER ? readFramebuffer : drawFramebuffer;
    if (!id) Fail("Attachment operation on default framebuffer");
    return framebuffers.at(id);
}
void SetAttachment(GLenum target, GLenum attachment, GLuint object, bool rb, unsigned layer, unsigned level)
{
    auto& f=BoundFramebuffer(target);
    if (attachment==GL_COLOR_ATTACHMENT0) f.color={object,rb,layer,level};
    else if (attachment==GL_DEPTH_STENCIL_ATTACHMENT || attachment==GL_DEPTH_ATTACHMENT) f.depth={object,rb,layer,level};
    else Fail("Unsupported framebuffer attachment");
    targetDirty=true;
}
void APIENTRY FramebufferTexture2D(GLenum target, GLenum attachment, GLenum, GLuint id, GLint level)
{ if (level<0) Fail("Invalid framebuffer mip level"); SetAttachment(target,attachment,id,false,0,level); }
void APIENTRY FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint id, GLint level, GLint layer)
{ if (level<0 || layer<0) Fail("Invalid framebuffer layer"); SetAttachment(target,attachment,id,false,layer,level); }
void APIENTRY FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum, GLuint id)
{ SetAttachment(target,attachment,id,true,0,0); }
void APIENTRY GenRenderbuffers(GLsizei count, GLuint* ids)
{ for (int i=0;i<count;++i) { ids[i]=nextObject++; renderbuffers.try_emplace(ids[i]); } }
void APIENTRY BindRenderbuffer(GLenum, GLuint id) { renderbuffer=id; }
void APIENTRY DeleteRenderbuffers(GLsizei count, const GLuint* ids)
{
    for (int i=0;i<count;++i) if (ids[i])
    {
        renderbuffers.erase(ids[i]); if (renderbuffer==ids[i]) renderbuffer=0;
        for (auto& [name,f]:framebuffers)
        {
            if (f.color.renderbuffer && f.color.object==ids[i]) f.color={};
            if (f.depth.renderbuffer && f.depth.object==ids[i]) f.depth={};
        }
    }
    targetDirty=true;
}
void APIENTRY RenderbufferStorageMultisample(GLenum, GLsizei samples, GLenum format, GLsizei width, GLsizei height)
{
    if (!renderbuffer || width<=0 || height<=0 || samples<=0 || (format!=GL_RGBA8 && format!=GL_DEPTH24_STENCIL8)) Fail("Invalid renderbuffer storage");
    renderbuffers.at(renderbuffer)=MakeImage(width,height,1,1,samples,format==GL_RGBA8 ? DkImageFormat_RGBA8_Unorm : DkImageFormat_Z24S8,false);
    targetDirty=true;
}
void APIENTRY RenderbufferStorage(GLenum target, GLenum format, GLsizei width, GLsizei height)
{ RenderbufferStorageMultisample(target,1,format,width,height); }
GLenum APIENTRY CheckFramebufferStatus(GLenum target)
{
    const auto id=target==GL_READ_FRAMEBUFFER ? readFramebuffer : drawFramebuffer;
    if (!id) return GL_FRAMEBUFFER_COMPLETE;
    const auto& f=framebuffers.at(id);
    auto color=AttachmentImage(f.color), depth=AttachmentImage(f.depth);
    if (!color && !depth) return GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    if ((f.color.object && !color) || (f.depth.object && !depth)) return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    if (color && depth && (color->width>>f.color.level != depth->width>>f.depth.level || color->height>>f.color.level != depth->height>>f.depth.level || color->samples!=depth->samples)) return GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    return GL_FRAMEBUFFER_COMPLETE;
}
void APIENTRY DrawBuffer(GLenum value)
{ if (drawFramebuffer) { framebuffers.at(drawFramebuffer).drawColor=value!=GL_NONE;targetDirty=true; } }
void APIENTRY DrawBuffers(GLsizei count, const GLenum* values)
{ if (count>1) Fail("Multiple color render targets are unused by ProjectCane"); DrawBuffer(count ? values[0] : GL_NONE); }
void APIENTRY ReadBuffer(GLenum value)
{ if (value!=GL_COLOR_ATTACHMENT0 && value!=GL_BACK && value!=GL_NONE) Fail("Unsupported read attachment"); }

unsigned targetWidth{}, targetHeight{}, targetSamples=1;
bool targetHasColor{}, targetHasDepth{};
void ApplyViewport()
{
    if (!viewportDirty) return;
    const DkViewport viewport{float(viewportX),float(viewportY),float(viewportWidth),float(viewportHeight),float(depthNear),float(depthFar)};
    dkCmdBufSetViewports(Commands(),0,&viewport,1);
    const int x=scissorEnable ? std::clamp(scissor[0],0,int(targetWidth)) : 0;
    const int y=scissorEnable ? std::clamp(scissor[1],0,int(targetHeight)) : 0;
    const int right=scissorEnable ? int(std::clamp<int64_t>(int64_t(scissor[0])+scissor[2],x,targetWidth)) : targetWidth;
    const int top=scissorEnable ? int(std::clamp<int64_t>(int64_t(scissor[1])+scissor[3],y,targetHeight)) : targetHeight;
    const DkScissor nativeScissor{uint32_t(x),uint32_t(y),uint32_t(right-x),uint32_t(top-y)};
    dkCmdBufSetScissors(Commands(),0,&nativeScissor,1);
    viewportDirty=false;
}
void ApplyTarget()
{
    EnsureFrame();
    if (!targetDirty) return;
    DkImageView colorView{},depthView{};
    const DkImageView* color=nullptr; const DkImageView* depth=nullptr;
    if (!drawFramebuffer)
    {
        dkImageViewDefaults(&colorView,&swapImages[Current().imageSlot]); color=&colorView;
        depthView=View(defaultDepth,0,0,true); depth=&depthView;
        targetWidth=outputWidth; targetHeight=outputHeight; targetSamples=1;
    }
    else
    {
        const auto& f=framebuffers.at(drawFramebuffer);
        auto colorImage=AttachmentImage(f.color), depthImage=AttachmentImage(f.depth);
        auto dimensions=colorImage ? colorImage : depthImage;
        if (!dimensions) Fail("Draw to an incomplete framebuffer");
        const auto level=colorImage ? f.color.level : f.depth.level;
        targetWidth=std::max(1u,dimensions->width>>level); targetHeight=std::max(1u,dimensions->height>>level); targetSamples=dimensions->samples;
        if (colorImage && f.drawColor) { colorView=View(colorImage,f.color.level,f.color.layer,true);color=&colorView; }
        if (depthImage) { depthView=View(depthImage,f.depth.level,f.depth.layer,true);depth=&depthView; }
    }
    dkCmdBufBarrier(Commands(),DkBarrier_Fragments,DkInvalidateFlags_Image|DkInvalidateFlags_Zcull);
    dkCmdBufBindRenderTargets(Commands(),color ? &color : nullptr,color ? 1 : 0,depth);
    targetHasColor = color != nullptr; targetHasDepth = depth != nullptr;
    DkMultisampleState ms; dkMultisampleStateDefaults(&ms); ms.mode=ms.rasterizerMode=Samples(targetSamples);
    dkMultisampleStateSetLocations(&ms,nullptr,0);
    dkCmdBufBindMultisampleState(Commands(),&ms);
    viewportDirty=pipelineDirty=true; targetDirty=false;
}
void ApplyPipeline()
{
    if (!pipelineDirty) return;
    DkRasterizerState raster; dkRasterizerStateDefaults(&raster);
    raster.cullMode=!cullEnable ? DkFace_None : cullFace==GL_FRONT ? DkFace_Front : cullFace==GL_FRONT_AND_BACK ? DkFace_FrontAndBack : DkFace_Back;
    raster.frontFace=frontFace==GL_CW ? DkFrontFace_CW : DkFrontFace_CCW;
    raster.polygonModeFront=raster.polygonModeBack=polygonMode;
    dkCmdBufBindRasterizerState(Commands(),&raster);
    DkColorState color; dkColorStateDefaults(&color); color.blendEnableMask=blendEnable ? 1 : 0;
    dkCmdBufBindColorState(Commands(),&color);
    DkColorWriteState write; dkColorWriteStateDefaults(&write); dkColorWriteStateSetMask(&write,0,colorMask);
    dkCmdBufBindColorWriteState(Commands(),&write);
    DkBlendState blend; dkBlendStateDefaults(&blend);
    dkBlendStateSetOps(&blend,BlendOp(colorEquation),BlendOp(alphaEquation));
    dkBlendStateSetFactors(&blend,BlendFactor(blendSource),BlendFactor(blendDestination),BlendFactor(alphaSource),BlendFactor(alphaDestination));
    dkCmdBufBindBlendState(Commands(),0,&blend);
    DkDepthStencilState depth; dkDepthStencilStateDefaults(&depth);
    depth.depthTestEnable=depthTest && targetHasDepth; depth.depthWriteEnable=depthTest && depthWrite && targetHasDepth;
    depth.depthCompareOp=Compare(depthFunction); depth.stencilTestEnable=stencilTest && targetHasDepth;
    depth.stencilFrontCompareOp=depth.stencilBackCompareOp=Compare(stencilFunction);
    depth.stencilFrontFailOp=depth.stencilBackFailOp=StencilOp(stencilFail);
    depth.stencilFrontDepthFailOp=depth.stencilBackDepthFailOp=StencilOp(stencilDepthFail);
    depth.stencilFrontPassOp=depth.stencilBackPassOp=StencilOp(stencilPass);
    dkCmdBufBindDepthStencilState(Commands(),&depth);
    dkCmdBufSetStencil(Commands(),DkFace_FrontAndBack,uint8_t(stencilMask),uint8_t(stencilReference),uint8_t(stencilCompareMask));
    dkCmdBufSetPointSize(Commands(),pointSize);dkCmdBufSetLineWidth(Commands(),lineWidth);
    pipelineDirty=false;
}
void APIENTRY Clear(GLbitfield mask)
{
    ApplyTarget(); ApplyViewport();
    if ((mask&GL_COLOR_BUFFER_BIT) && targetHasColor) dkCmdBufClearColor(Commands(),0,colorMask,clearColor.data());
    if ((mask&(GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT)) && targetHasDepth)
        dkCmdBufClearDepthStencil(Commands(),bool(mask&GL_DEPTH_BUFFER_BIT)&&depthWrite,clearDepth,mask&GL_STENCIL_BUFFER_BIT ? uint8_t(stencilMask) : 0,uint8_t(clearStencil));
}
void APIENTRY ClearBufferfv(GLenum buffer, GLint index, const GLfloat* value)
{
    if (buffer!=GL_COLOR || index) Fail("Unsupported clear-buffer target");
    ApplyTarget(); ApplyViewport(); if (targetHasColor) dkCmdBufClearColor(Commands(),0,colorMask,value);
}
DkImageView FramebufferColorView(GLuint id)
{
    if (!id) { DkImageView v; dkImageViewDefaults(&v,&swapImages[Current().imageSlot]);return v; }
    const auto& f=framebuffers.at(id);return View(AttachmentImage(f.color),f.color.level,f.color.layer,true);
}
void APIENTRY BlitFramebuffer(GLint sx0, GLint sy0, GLint sx1, GLint sy1, GLint dx0, GLint dy0, GLint dx1, GLint dy1, GLbitfield mask, GLenum filter)
{
    if (mask!=GL_COLOR_BUFFER_BIT || sx0<0 || sy0<0 || dx0<0 || dy0<0 || sx1<=sx0 || sy1<=sy0 || dx1<=dx0 || dy1<=dy0) Fail("Unsupported framebuffer blit");
    EnsureFrame(); auto src=FramebufferColorView(readFramebuffer),dst=FramebufferColorView(drawFramebuffer);
    dkCmdBufBarrier(Commands(),DkBarrier_Primitives,0);
    const unsigned samples=readFramebuffer ? AttachmentImage(framebuffers.at(readFramebuffer).color)->samples : 1;
    if (samples>1)
    {
        if (sx0 || sy0 || dx0 || dy0 || sx1!=dx1 || sy1!=dy1) Fail("MSAA resolve rectangles must match");
        dkCmdBufResolveImage(Commands(),&src,&dst);
    }
    else
    {
        const DkImageRect srcRect{uint32_t(sx0),uint32_t(sy0),0,uint32_t(sx1-sx0),uint32_t(sy1-sy0),1};
        const DkImageRect dstRect{uint32_t(dx0),uint32_t(dy0),0,uint32_t(dx1-dx0),uint32_t(dy1-dy0),1};
        dkCmdBufBlitImage(Commands(),&src,&srcRect,&dst,&dstRect,filter==GL_LINEAR ? DkBlitFlag_FilterLinear : 0,0);
    }
    dkCmdBufBarrier(Commands(),DkBarrier_None,DkInvalidateFlags_Image);
    targetDirty=true;
}

bool SameExtent(const DkBufExtents& a,const DkBufExtents& b) { return a.addr==b.addr && a.size==b.size; }
void BindUniform(unsigned index,DkBufExtents extent,unsigned stages)
{
    for (unsigned stage=0;stage<2;++stage)
        if ((stages & (1u<<stage)) && !SameExtent(boundUniforms[stage][index],extent))
        {
            dkCmdBufBindUniformBuffers(Commands(),stage ? DkStage_Fragment : DkStage_Vertex,index,&extent,1);
            boundUniforms[stage][index]=extent;
        }
}
void ApplyResources()
{
    if (!currentProgram) Fail("Draw without a precompiled shader program");
    auto p=shaderPrograms.at(currentProgram);
    if (boundProgram!=currentProgram)
    {
        const DkShader* stages[]{&p->vertex,&p->fragment};
        dkCmdBufBindShaders(Commands(),DkStageFlag_GraphicsMask,stages,2);

        Retain(p);
        boundProgram=currentProgram;
    }
    unsigned uniformMask = (p->info->uniformMask[0] | p->info->uniformMask[1]) & 0x7fff;
    while (uniformMask)
    {
        const unsigned index = std::countr_zero(uniformMask);
        uniformMask &= uniformMask-1;
        const unsigned stages = ((p->info->uniformMask[0]>>index)&1) | (((p->info->uniformMask[1]>>index)&1)<<1);
        BindUniform(index,ResolveUniform(index),stages);
    }
    unsigned storageMask = p->info->storageMask[0] | p->info->storageMask[1];
    while (storageMask)
    {
        const unsigned index = std::countr_zero(storageMask);
        storageMask &= storageMask-1;

        const auto& range=storageRanges[index];
        auto extent=ResolveBuffer(range.buffer);
        if (extent.addr)
        {
            extent.addr+=range.offset;
            extent.size=RangeSize(range);
        }

        for (unsigned stage=0;stage<2;++stage)
            if ((p->info->storageMask[stage] & (1u<<index)) && !SameExtent(boundStorage[stage][index],extent))
            {
                dkCmdBufBindStorageBuffers(Commands(),stage ? DkStage_Fragment : DkStage_Vertex,index,&extent,1);
                boundStorage[stage][index]=extent;
            }
    }
    if (!p->constants.empty())
    {
        if (p->uploadedSerial!=Current().serial || p->uploadedVersion!=p->version)
        {

            auto upload=ReserveUpload(Align(p->constants.size(),256));
            std::memcpy(upload.cpu,p->constants.data(),p->constants.size());
            std::memset(upload.cpu+p->constants.size(),0,upload.size-p->constants.size());
            p->uploadedAddress=upload.gpu; p->uploadedSerial=Current().serial; p->uploadedVersion=p->version;
        }
        const unsigned stages = (p->info->uniformMask[0]>>15) | ((p->info->uniformMask[1]>>15)<<1);
        BindUniform(15,{p->uploadedAddress,uint32_t(p->constants.size())},stages);
    }
    for (const auto sampler : p->samplers)
    {
        const auto unit=p->units[sampler.binding];
        const bool array = sampler.array;
        const auto id=textureUnits[unit][array ? 1 : 0];
        const auto samplerId=samplerUnits[unit];
        const auto handle=TextureHandle(!id && array ? whiteArray : textures.at(id), array,
            samplerId ? &samplerObjects.at(samplerId) : nullptr);
        if (boundTextures[sampler.binding]!=handle)
        {
            dkCmdBufBindTexture(Commands(),DkStage_Fragment,sampler.binding,handle);

            boundTextures[sampler.binding]=handle;
        }
    }
}
DkVtxAttribSize AttributeSize(unsigned components,unsigned bytes)
{
    static constexpr DkVtxAttribSize sizes[3][4] = {
        {DkVtxAttribSize_1x8,DkVtxAttribSize_2x8,DkVtxAttribSize_3x8,DkVtxAttribSize_4x8},
        {DkVtxAttribSize_1x16,DkVtxAttribSize_2x16,DkVtxAttribSize_3x16,DkVtxAttribSize_4x16},
        {DkVtxAttribSize_1x32,DkVtxAttribSize_2x32,DkVtxAttribSize_3x32,DkVtxAttribSize_4x32}};
    return sizes[bytes==1 ? 0 : bytes==2 ? 1 : 2][components-1];
}
DkVtxAttribType AttributeType(const Attribute& a)
{
    if (a.type==GL_FLOAT || a.type==GL_HALF_FLOAT) return DkVtxAttribType_Float;
    const bool sign=a.type==GL_BYTE || a.type==GL_SHORT || a.type==GL_INT;
    if (a.integer) return sign ? DkVtxAttribType_Sint : DkVtxAttribType_Uint;
    if (a.normalized) return sign ? DkVtxAttribType_Snorm : DkVtxAttribType_Unorm;
    return sign ? DkVtxAttribType_Sscaled : DkVtxAttribType_Uscaled;
}
uint64_t genericUploadedSerial{},genericUploadedVersion{};
DkGpuAddr genericAddress{};
void ApplyVertices()
{
    auto& vao=arrays.at(vertexArray);
    const bool changed=vertexDirty || boundArray!=vertexArray || boundArrayVersion!=vao.version || boundGenericVersion!=genericVersion;
    if (!changed && boundVertexDataEpoch==vertexDataEpoch)
    { return; }
    if (genericUploadedSerial!=Current().serial || genericUploadedVersion!=genericVersion)
    {
        auto upload=ReserveUpload(sizeof(genericAttributes));
        std::memcpy(upload.cpu,genericAttributes.data(),sizeof(genericAttributes));
        genericAddress=upload.gpu; genericUploadedSerial=Current().serial;genericUploadedVersion=genericVersion;
    }
    std::array<DkVtxAttribState,16> attributes{};
    std::array<DkVtxBufferState,16> states{};
    for (unsigned i=0;i<15;++i)
    {
        const auto& a=vao.attributes[i];
        auto& native=attributes[i];
        DkBufExtents extent{};
        if (a.enabled && a.buffer)
        {
            extent=ResolveBuffer(a.buffer);
            if (a.offset>extent.size) Fail("Vertex attribute offset exceeds buffer");
            extent.addr+=a.offset; extent.size-=a.offset;
            native.bufferId=i;native.size=AttributeSize(a.components,ComponentBytes(a.type));native.type=AttributeType(a);
            states[i]={a.stride,a.divisor};
        }
        else
        {

            native.bufferId=15;native.offset=i*16;native.size=DkVtxAttribSize_4x32;native.type=DkVtxAttribType_Float;
        }
        if (!SameExtent(boundVertexBuffers[i],extent))
        { dkCmdBufBindVtxBuffers(Commands(),i,&extent,1);boundVertexBuffers[i]=extent; }
    }
    attributes[15].isFixed=1;attributes[15].size=DkVtxAttribSize_4x32;attributes[15].type=DkVtxAttribType_Float;
    const DkBufExtents constants{genericAddress,sizeof(genericAttributes)};
    if (!SameExtent(boundVertexBuffers[15],constants))
    { dkCmdBufBindVtxBuffers(Commands(),15,&constants,1);boundVertexBuffers[15]=constants; }
    if (changed)
    {
        dkCmdBufBindVtxAttribState(Commands(),attributes.data(),attributes.size());
        dkCmdBufBindVtxBufferState(Commands(),states.data(),states.size());
        boundArray=vertexArray;boundArrayVersion=vao.version;boundGenericVersion=genericVersion;vertexDirty=false;
    }
    boundVertexDataEpoch=vertexDataEpoch;
}
DkPrimitive Primitive(GLenum mode)
{
    switch (mode) { case GL_POINTS: return DkPrimitive_Points;case GL_LINES: return DkPrimitive_Lines;
    case GL_LINE_STRIP: return DkPrimitive_LineStrip;case GL_LINE_LOOP: return DkPrimitive_LineLoop;
    case GL_TRIANGLES: return DkPrimitive_Triangles;case GL_TRIANGLE_STRIP: return DkPrimitive_TriangleStrip;
    case GL_TRIANGLE_FAN: return DkPrimitive_TriangleFan;default: Fail("Unsupported primitive topology"); }
}
void PrepareDraw()
{
    ApplyTarget();
    ApplyViewport();
    ApplyPipeline();
    ApplyResources();
    ApplyVertices();
}
void APIENTRY DrawArrays(GLenum mode,GLint first,GLsizei count)
{
    if (first<0 || count<0) Fail("Invalid draw range");
    if (!count) return;
    PrepareDraw();dkCmdBufDraw(Commands(),Primitive(mode),count,1,first,0);
}
void DrawIndexed(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instances,GLint vertexOffset,GLuint firstInstance)
{
    if (count<0 || instances<0) Fail("Invalid indexed draw range");
    if (!count || !instances) return;
    PrepareDraw();
    const auto extent=ResolveBuffer(arrays.at(vertexArray).indices);
    const auto offset=reinterpret_cast<uintptr_t>(indices);
    const unsigned bytes=ComponentBytes(type);
    if (!extent.addr || offset%bytes || offset+uint64_t(count)*bytes>extent.size) Fail("Index range exceeds its buffer");
    if (type!=GL_UNSIGNED_BYTE && type!=GL_UNSIGNED_SHORT && type!=GL_UNSIGNED_INT) Fail("Invalid index format");
    if (boundIndexAddress != extent.addr || boundIndexType != type)
    {
        dkCmdBufBindIdxBuffer(Commands(),type==GL_UNSIGNED_SHORT ? DkIdxFormat_Uint16 : type==GL_UNSIGNED_INT ? DkIdxFormat_Uint32 : DkIdxFormat_Uint8,extent.addr);
        boundIndexAddress = extent.addr; boundIndexType = type;
    }
    dkCmdBufDrawIndexed(Commands(),Primitive(mode),count,instances,offset/bytes,vertexOffset,firstInstance);
}
void APIENTRY DrawElements(GLenum mode,GLsizei count,GLenum type,const void* indices) { DrawIndexed(mode,count,type,indices,1,0,0); }
void APIENTRY DrawElementsBaseVertex(GLenum mode,GLsizei count,GLenum type,const void* indices,GLint base) { DrawIndexed(mode,count,type,indices,1,base,0); }
void APIENTRY DrawElementsInstancedBaseInstance(GLenum mode,GLsizei count,GLenum type,const void* indices,GLsizei instances,GLuint base) { DrawIndexed(mode,count,type,indices,instances,0,base); }

GLsync APIENTRY FenceSync(GLenum condition,GLbitfield)
{
    if (condition!=GL_SYNC_GPU_COMMANDS_COMPLETE) Fail("Unsupported fence condition");
    auto sync=std::make_shared<Sync>();
    auto id=reinterpret_cast<GLsync>(sync.get());
    dkCmdBufSignalFence(Commands(),&sync->fence,false);
    syncs.emplace(id,sync);Retain(sync);
    return id;
}
GLenum APIENTRY ClientWaitSync(GLsync id,GLbitfield flags,GLuint64 timeout)
{
    if (flags&GL_SYNC_FLUSH_COMMANDS_BIT) Submit();
    auto it=syncs.find(id);if (it==syncs.end()) Fail("Unknown GPU fence");
    const auto result=dkFenceWait(&it->second->fence,timeout==GL_TIMEOUT_IGNORED ? -1 : int64_t(std::min<uint64_t>(timeout,INT64_MAX)));
    return result==DkResult_Success ? GL_ALREADY_SIGNALED : result==DkResult_Timeout ? GL_TIMEOUT_EXPIRED : GL_WAIT_FAILED;
}
void APIENTRY DeleteSync(GLsync id) { syncs.erase(id); }
void APIENTRY Finish() { Submit();if (queue) dkQueueWaitIdle(queue); }
void APIENTRY PolygonMode(GLenum face, GLenum mode)
{
    if (face!=GL_FRONT_AND_BACK || (mode!=GL_FILL && mode!=GL_LINE && mode!=GL_POINT)) Fail("Unsupported polygon mode");
    polygonMode=mode==GL_LINE ? DkPolygonMode_Line : mode==GL_POINT ? DkPolygonMode_Point : DkPolygonMode_Fill;
    pipelineDirty=true;
}
void APIENTRY GetIntegerv(GLenum parameter,GLint* value)
{
    switch (parameter)
    {
        case GL_NUM_EXTENSIONS: *value=1;break;
        case GL_PACK_ALIGNMENT: *value=packAlignment;break;
        case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT: *value=DK_UNIFORM_BUF_ALIGNMENT;break;
        case GL_MAX_ARRAY_TEXTURE_LAYERS: *value=2048;break;
        case GL_MAX_SAMPLES: *value=8;break;
        case GL_MAX_TEXTURE_SIZE:
        case GL_MAX_RENDERBUFFER_SIZE: *value=16384;break;
        case GL_VERTEX_ARRAY_BINDING: *value=vertexArray;break;
        case GL_CURRENT_PROGRAM: *value=currentProgram;break;
        case GL_ARRAY_BUFFER_BINDING: *value=arrayBuffer;break;
        case GL_ELEMENT_ARRAY_BUFFER_BINDING: *value=arrays.at(vertexArray).indices;break;
        case GL_READ_FRAMEBUFFER_BINDING: *value=readFramebuffer;break;
        case GL_DRAW_FRAMEBUFFER_BINDING: *value=drawFramebuffer;break;
        case GL_TEXTURE_BINDING_2D: *value=textureUnits[activeUnit][0];break;
        case GL_TEXTURE_BINDING_2D_ARRAY: *value=textureUnits[activeUnit][1];break;
        case GL_ACTIVE_TEXTURE: *value=GL_TEXTURE0+activeUnit;break;
        case GL_SAMPLER_BINDING: *value=samplerUnits[activeUnit];break;
        case GL_BLEND_EQUATION_RGB: *value=colorEquation;break;
        case GL_BLEND_EQUATION_ALPHA: *value=alphaEquation;break;
        case GL_BLEND_SRC_RGB: *value=blendSource;break;
        case GL_BLEND_DST_RGB: *value=blendDestination;break;
        case GL_BLEND_SRC_ALPHA: *value=alphaSource;break;
        case GL_BLEND_DST_ALPHA: *value=alphaDestination;break;
        case GL_DEPTH_FUNC: *value=depthFunction;break;
        case GL_STENCIL_FUNC: *value=stencilFunction;break;
        case GL_STENCIL_REF: *value=stencilReference;break;
        case GL_STENCIL_VALUE_MASK: *value=stencilCompareMask;break;
        case GL_STENCIL_WRITEMASK: *value=stencilMask;break;
        case GL_STENCIL_FAIL: *value=stencilFail;break;
        case GL_STENCIL_PASS_DEPTH_FAIL: *value=stencilDepthFail;break;
        case GL_STENCIL_PASS_DEPTH_PASS: *value=stencilPass;break;
        case GL_VIEWPORT: value[0]=viewportX;value[1]=viewportY;value[2]=viewportWidth;value[3]=viewportHeight;break;
        case GL_SCISSOR_BOX: std::copy(scissor.begin(),scissor.end(),value);break;
        case GL_UNPACK_ALIGNMENT: *value=unpackAlignment;break;
        case GL_UNPACK_ROW_LENGTH: *value=unpackRowLength;break;
        default: Fail("Unsupported integer state query " + std::to_string(parameter));
    }
}
void APIENTRY GetBooleanv(GLenum parameter,GLboolean* value)
{
    if (parameter==GL_DEPTH_WRITEMASK) *value=depthWrite;
    else if (parameter==GL_COLOR_WRITEMASK) for (unsigned i=0;i<4;++i) value[i]=bool(colorMask&(1<<i));
    else *value=IsEnabled(parameter);
}
void APIENTRY GetFloatv(GLenum parameter,GLfloat* value)
{
    if (parameter==0x84FF) *value=16.0f;
    else if (parameter==GL_POINT_SIZE) *value=pointSize;
    else if (parameter==GL_LINE_WIDTH) *value=lineWidth;
    else if (parameter==GL_DEPTH_RANGE) { value[0]=depthNear;value[1]=depthFar; }
    else Fail("Unsupported floating point state query");
}
void APIENTRY GetDoublev(GLenum parameter,GLdouble* value)
{
    if (parameter==GL_DEPTH_RANGE) { value[0]=depthNear;value[1]=depthFar; }
    else Fail("Unsupported double state query");
}
const GLubyte* APIENTRY GetString(GLenum parameter)
{
    const char* value=parameter==GL_VENDOR ? "devkitPro" : parameter==GL_RENDERER ? "Deko3D native Maxwell" : parameter==GL_VERSION ? "Deko3D; offline DKSH shaders" : "";
    return reinterpret_cast<const GLubyte*>(value);
}
const GLubyte* APIENTRY GetStringi(GLenum parameter, GLuint index)
{
    return parameter == GL_EXTENSIONS && index == 0 ?
        reinterpret_cast<const GLubyte*>("GL_EXT_texture_filter_anisotropic") : nullptr;
}
GLenum APIENTRY GetError() { return GL_NO_ERROR; }
void CreateSwapchain(unsigned width, unsigned height)
{
    DkImageLayout layout;
    DkImageLayoutMaker lm;dkImageLayoutMakerDefaults(&lm,device);
    lm.format=DkImageFormat_RGBA8_Unorm;lm.dimensions[0]=width;lm.dimensions[1]=height;lm.dimensions[2]=1;
    lm.flags=DkImageFlags_UsageRender|DkImageFlags_UsagePresent|DkImageFlags_Usage2DEngine|DkImageFlags_HwCompression;
    dkImageLayoutInitialize(&layout,&lm);
    const auto stride=Align(dkImageLayoutGetSize(&layout),dkImageLayoutGetAlignment(&layout));
    swapMemory=MakeMemory(stride*FrameCount,DkMemBlockFlags_Image|DkMemBlockFlags_GpuCached);
    const DkImage* images[FrameCount];
    for (unsigned i=0;i<FrameCount;++i) { dkImageInitialize(&swapImages[i],&layout,swapMemory,stride*i);images[i]=&swapImages[i]; }
    DkSwapchainMaker sm;dkSwapchainMakerDefaults(&sm,device,nwindowGetDefault(),images,FrameCount);
    swapchain=dkSwapchainCreate(&sm);if (!swapchain) Fail("Swapchain creation failed");
    dkSwapchainSetSwapInterval(swapchain,swapInterval);
    defaultDepth=MakeImage(width,height,1,1,1,DkImageFormat_Z24S8,false);
    outputWidth=width;outputHeight=height;
}
}

bool Initialize(unsigned width,unsigned height)
{
    if (device) return true;
    outputWidth=width;outputHeight=height;
    viewportWidth=width;viewportHeight=height;scissor={0,0,int(width),int(height)};
    for (auto& attribute:genericAttributes) attribute={0,0,0,1};
    DkDeviceMaker maker;dkDeviceMakerDefaults(&maker);
    maker.flags=DkDeviceFlags_DepthMinusOneToOne|DkDeviceFlags_OriginLowerLeft;
    device=dkDeviceCreate(&maker);if (!device) Fail("Device creation failed");
    DkQueueMaker qm;dkQueueMakerDefaults(&qm,device);
    unsigned scratch=DK_PER_WARP_SCRATCH_MEM_ALIGNMENT;
    for (const auto& p:programs)
    {
        scratch=std::max(scratch,DecodeDksh(p.vertex,p.vertexSize).scratchSize);
        scratch=std::max(scratch,DecodeDksh(p.fragment,p.fragmentSize).scratchSize);
    }
    scratch=Align(scratch,DK_PER_WARP_SCRATCH_MEM_ALIGNMENT);
    qm.flags=DkQueueFlags_Graphics;qm.commandMemorySize=1024*1024;qm.flushThreshold=128*1024;qm.perWarpScratchMemorySize=scratch;
    queue=dkQueueCreate(&qm);if (!queue) Fail("Graphics queue creation failed");

    CreateSwapchain(width,height);
    for (auto& frame:frames)
    {
        DkCmdBufMaker cm;dkCmdBufMakerDefaults(&cm,device);cm.userData=&frame;cm.cbAddMem=GrowCommands;
        frame.command=dkCmdBufCreate(&cm);
        if (!frame.command) Fail("Command buffer creation failed");
        frame.commandMemory.push_back(std::make_shared<Memory>(1024*1024,DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached));
        frame.descriptors=std::make_shared<Memory>(ImageDescriptorCount*sizeof(DkImageDescriptor)+SamplerDescriptorCount*sizeof(DkSamplerDescriptor),DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached);
    }
    Texture white;white.image=MakeImage(1,1,1,1,1,DkImageFormat_RGBA8_Unorm,false);white.minFilter=GL_LINEAR;
    textures.emplace(0,white);
    const uint32_t pixel=0xffffffff;UploadPixels(white.image,0,0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&pixel);
    whiteArray.image=MakeImage(1,1,1,1,1,DkImageFormat_RGBA8_Unorm,true);whiteArray.target=GL_TEXTURE_2D_ARRAY;whiteArray.minFilter=GL_LINEAR;
    UploadPixels(whiteArray.image,0,0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,&pixel);
    InstallEntryPoints();
    return true;
}
void ResizeOutput(unsigned width,unsigned height)
{
    if (!device || (width==outputWidth && height==outputHeight)) return;
    if (!width || !height) Fail("Invalid output dimensions");
    if (recording) { Submit();recording=false; }
    dkQueueWaitIdle(queue);
    for (auto& frame:frames)
    {
        dkCmdBufClear(frame.command);
        frame.keepAlive.clear();frame.retained.clear();
        frame.images.clear();frame.samplers.clear();
        frame.imageCount=frame.samplerCount=0;
        frame.submitted=false;frame.imageSlot=-1;
    }
    dkSwapchainDestroy(swapchain);swapchain=nullptr;
    dkMemBlockDestroy(swapMemory);swapMemory=nullptr;
    defaultDepth.reset();
    std::erase_if(imagePages,[](const auto& page) {
        return page.use_count()==1 && page->ranges.FreeBytes()==page->memory->Size();
    });
    CreateSwapchain(width,height);
    viewportWidth=width;viewportHeight=height;scissor={0,0,int(width),int(height)};
    InvalidateState();
}
void SetSwapInterval(int interval)
{
    swapInterval=std::clamp(interval,0,1);
    if (swapchain) dkSwapchainSetSwapInterval(swapchain,swapInterval);
}
void Present()
{
    EnsureFrame();
    auto& f=Current();
    dkCmdBufSignalFence(f.command,&f.fence,true);
    dkQueueSubmitCommands(queue,dkCmdBufFinishList(f.command));
    f.submitted=true;
    dkQueuePresentImage(queue,swapchain,f.imageSlot);
    if (dkQueueIsInErrorState(queue)) Fail("GPU queue reported an error");

    recording=false;frameIndex=(frameIndex+1)%FrameCount;
}
GLuint CreateProgram(const char* vertex,const char* geometry,const char* fragment)
{
    if (!vertex || !fragment || geometry) Fail("Unsupported precompiled shader combination");
    const std::string vs(vertex),fs(fragment);
    const ProgramInfo* info=nullptr;
    for (const auto& candidate:programs)
        if (vs==std::string(candidate.name)+".vert" && fs==std::string(candidate.name)+".frag") { info=&candidate;break; }
    if (!info) Fail("Shader program missing from offline manifest: "+vs+" / "+fs);
    auto p=std::make_shared<Program>();p->info=info;p->constants.resize(info->constantSize);
    for (unsigned i=0;i<info->uniformCount;++i)
        if (!std::strncmp(info->uniforms[i].type,"sampler",7))
            p->samplers.push_back({info->uniforms[i].offset,!std::strcmp(info->uniforms[i].type,"sampler2DArray")});
    const auto vertexBinary=DecodeDksh(info->vertex,info->vertexSize),fragmentBinary=DecodeDksh(info->fragment,info->fragmentSize);
    const auto fragmentOffset=Align(vertexBinary.codeSize,DK_SHADER_CODE_ALIGNMENT);
    const auto codeSize=Align(fragmentOffset+fragmentBinary.codeSize,DK_SHADER_CODE_ALIGNMENT)+DK_SHADER_CODE_UNUSABLE_SIZE;
    p->code=std::make_shared<Memory>(codeSize,DkMemBlockFlags_CpuUncached|DkMemBlockFlags_GpuCached|DkMemBlockFlags_Code);
    std::memcpy(p->code->Cpu(),vertexBinary.code,vertexBinary.codeSize);
    std::memcpy(p->code->Cpu()+fragmentOffset,fragmentBinary.code,fragmentBinary.codeSize);
    DkShaderMaker vm;dkShaderMakerDefaults(&vm,p->code->block,0);vm.control=vertexBinary.control;dkShaderInitialize(&p->vertex,&vm);
    DkShaderMaker fm;dkShaderMakerDefaults(&fm,p->code->block,fragmentOffset);fm.control=fragmentBinary.control;dkShaderInitialize(&p->fragment,&fm);
    if (!dkShaderIsValid(&p->vertex) || !dkShaderIsValid(&p->fragment)) Fail("DKSH shader initialization failed: "+vs);
    const auto id=nextObject++;shaderPrograms.emplace(id,p);

    return id;
}
void Shutdown()
{
    if (!device) return;
    if (recording) { Submit();recording=false; }
    dkQueueWaitIdle(queue);
    buffers.clear();arrays.clear();textures.clear();samplerObjects.clear();samplerUnits.fill(0);renderbuffers.clear();framebuffers.clear();shaderPrograms.clear();syncs.clear();defaultDepth.reset();whiteArray.image.reset();
    for (auto& frame:frames)
    {
        frame.keepAlive.clear();frame.retained.clear();frame.upload.clear();frame.descriptors.reset();
        if (frame.command) dkCmdBufDestroy(frame.command);
        frame.command=nullptr;frame.commandMemory.clear();
    }
    bufferPages.clear();imagePages.clear();
    if (swapchain) dkSwapchainDestroy(swapchain);
    swapchain=nullptr;
    if (swapMemory) dkMemBlockDestroy(swapMemory);
    swapMemory=nullptr;
    if (queue) dkQueueDestroy(queue);
    queue=nullptr;dkDeviceDestroy(device);device=nullptr;
}
void InstallEntryPoints()
{
#define ENTRY(name) glad_gl##name = &name
    ENTRY(GenBuffers);ENTRY(BindBuffer);ENTRY(BufferData);ENTRY(BufferSubData);ENTRY(DeleteBuffers);ENTRY(BindBufferBase);ENTRY(BindBufferRange);ENTRY(UnmapBuffer);
    ENTRY(GenVertexArrays);ENTRY(BindVertexArray);ENTRY(DeleteVertexArrays);ENTRY(VertexAttribPointer);ENTRY(VertexAttribIPointer);ENTRY(EnableVertexAttribArray);ENTRY(DisableVertexAttribArray);ENTRY(VertexAttribDivisor);ENTRY(VertexAttrib2f);ENTRY(VertexAttrib3f);ENTRY(VertexAttrib4f);
    ENTRY(UseProgram);ENTRY(DeleteProgram);ENTRY(GetUniformLocation);ENTRY(GetUniformBlockIndex);ENTRY(UniformBlockBinding);
    ENTRY(Uniform1i);ENTRY(Uniform1f);ENTRY(Uniform1fv);ENTRY(Uniform2f);ENTRY(Uniform3f);ENTRY(Uniform3fv);ENTRY(Uniform4f);ENTRY(Uniform4fv);ENTRY(UniformMatrix3fv);ENTRY(UniformMatrix4fv);
    ENTRY(Enable);ENTRY(Disable);ENTRY(IsEnabled);ENTRY(DepthMask);ENTRY(DepthFunc);ENTRY(DepthRange);ENTRY(FrontFace);ENTRY(CullFace);ENTRY(BlendFunc);ENTRY(BlendFuncSeparate);ENTRY(BlendEquation);ENTRY(BlendEquationSeparate);ENTRY(ColorMask);ENTRY(StencilFunc);ENTRY(StencilMask);ENTRY(StencilOp);ENTRY(Viewport);ENTRY(Scissor);ENTRY(PointSize);ENTRY(LineWidth);ENTRY(ClearColor);ENTRY(ClearDepth);ENTRY(ClearStencil);ENTRY(Clear);ENTRY(ClearBufferfv);ENTRY(MemoryBarrier);
    ENTRY(GenTextures);ENTRY(ActiveTexture);ENTRY(BindTexture);ENTRY(DeleteTextures);ENTRY(TexParameteri);ENTRY(PixelStorei);ENTRY(TexStorage2D);ENTRY(TexStorage3D);ENTRY(TexImage2D);ENTRY(TexSubImage2D);ENTRY(GenerateMipmap);ENTRY(GetTexLevelParameteriv);
    ENTRY(CompressedTexSubImage2D);ENTRY(TexParameterf);ENTRY(GetTexImage);ENTRY(GetStringi);
    ENTRY(GenSamplers);ENTRY(BindSampler);ENTRY(SamplerParameteri);ENTRY(DeleteSamplers);
    ENTRY(GenFramebuffers);ENTRY(BindFramebuffer);ENTRY(DeleteFramebuffers);ENTRY(FramebufferTexture2D);ENTRY(FramebufferTextureLayer);ENTRY(FramebufferRenderbuffer);ENTRY(CheckFramebufferStatus);ENTRY(DrawBuffer);ENTRY(DrawBuffers);ENTRY(ReadBuffer);ENTRY(BlitFramebuffer);
    ENTRY(GenRenderbuffers);ENTRY(BindRenderbuffer);ENTRY(DeleteRenderbuffers);ENTRY(RenderbufferStorage);ENTRY(RenderbufferStorageMultisample);
    ENTRY(DrawArrays);ENTRY(DrawElements);ENTRY(DrawElementsBaseVertex);ENTRY(DrawElementsInstancedBaseInstance);
    ENTRY(FenceSync);ENTRY(ClientWaitSync);ENTRY(DeleteSync);ENTRY(Finish);ENTRY(PolygonMode);
    ENTRY(GetIntegerv);ENTRY(GetBooleanv);ENTRY(GetFloatv);ENTRY(GetDoublev);ENTRY(GetString);ENTRY(GetError);
#undef ENTRY
}
}
