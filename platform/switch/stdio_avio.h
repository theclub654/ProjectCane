#pragma once
#include <cerrno>
#include <cstdio>
#include <cstdint>
extern "C"
{
#include <libavformat/avformat.h>
#include <libavutil/mem.h>
}

namespace CaneFileIO
{
inline int Read(void* opaque, uint8_t* data, int size)
{
    if (size <= 0) return AVERROR(EINVAL);
    FILE* file = static_cast<FILE*>(opaque);
    const size_t count = std::fread(data, 1, size_t(size), file);
    if (count) return int(count);
    return std::ferror(file) ? AVERROR(errno ? errno : EIO) : AVERROR_EOF;
}
inline int64_t Tell(FILE* file)
{
#ifdef _WIN32
    return _ftelli64(file);
#else
    return ftello(file);
#endif
}
inline int SeekFile(FILE* file, int64_t offset, int whence)
{
#ifdef _WIN32
    return _fseeki64(file, offset, whence);
#else
    return fseeko(file, offset, whence);
#endif
}
inline int64_t Seek(void* opaque, int64_t offset, int whence)
{
    FILE* file = static_cast<FILE*>(opaque);
    whence &= ~AVSEEK_FORCE;
    if (whence == AVSEEK_SIZE)
    {
        const int64_t previous = Tell(file);
        if (previous < 0 || SeekFile(file, 0, SEEK_END)) return AVERROR(errno ? errno : EIO);
        const int64_t size = Tell(file);
        if (SeekFile(file, previous, SEEK_SET)) return AVERROR(errno ? errno : EIO);
        return size < 0 ? AVERROR(errno ? errno : EIO) : size;
    }
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) return AVERROR(EINVAL);
    if (SeekFile(file, offset, whence)) return AVERROR(errno ? errno : EIO);
    const int64_t position = Tell(file);
    return position < 0 ? AVERROR(errno ? errno : EIO) : position;
}
class Input
{
    FILE* file = nullptr;
    AVIOContext* io = nullptr;
public:
    int Open(AVFormatContext** format, const char* path)
    {
        file = std::fopen(path, "rb");
        if (!file) return AVERROR(errno ? errno : EIO);
        auto* buffer = static_cast<uint8_t*>(av_malloc(64 * 1024));
        if (!buffer) return AVERROR(ENOMEM);
        io = avio_alloc_context(buffer, 64 * 1024, 0, file, Read, nullptr, Seek);
        if (!io) { av_free(buffer); return AVERROR(ENOMEM); }
        *format = avformat_alloc_context();
        if (!*format) return AVERROR(ENOMEM);
        (*format)->pb = io;
        (*format)->flags |= AVFMT_FLAG_CUSTOM_IO;
        return avformat_open_input(format, path, nullptr, nullptr);
    }
    ~Input()
    {
        if (io) { av_freep(&io->buffer); avio_context_free(&io); }
        if (file) std::fclose(file);
    }
};
}
