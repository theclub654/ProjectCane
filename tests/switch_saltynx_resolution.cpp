#include <cassert>
#include <cstdint>
#include <stdexcept>

namespace
{
unsigned reportedWidth{}, reportedHeight{}, reportedFrames{};
unsigned presentations{};
bool queueError{};

void saltynx_set_resolution(uint32_t width, uint32_t height)
{
    reportedWidth=width;
    reportedHeight=height;
}
void saltynx_frame(uint8_t api)
{
    assert(api==1);
    ++reportedFrames;
}
void dkCmdBufSignalFence(int, int*, bool) {}
int dkCmdBufFinishList(int command) { return command; }
void dkQueueSubmitCommands(int, int) {}
void dkQueuePresentImage(int, int, int) { ++presentations; }
bool dkQueueIsInErrorState(int) { return queueError; }
[[noreturn]] void Fail(const char* message) { throw std::runtime_error(message); }
}

namespace CaneDeko
{
constexpr unsigned FrameCount=3;
unsigned outputWidth=1280, outputHeight=720, frameIndex{};
unsigned frameResolutionWidth{}, frameResolutionHeight{};
int queue{}, swapchain{};
bool recording{};
struct Frame { int command{}, fence{}, imageSlot{}; bool submitted{}; };
Frame frames[FrameCount];
Frame& Current() { return frames[frameIndex]; }
void EnsureFrame() { recording=true; }

// These are the actual backend functions, extracted by the Python test runner.
#include "saltynx_present.inc"
}

int main()
{
    using namespace CaneDeko;
    // Handheld output remains 720p while the scene supersamples at 1080p.
    SetFrameResolution(1920,1080);
    Present();
    assert(reportedWidth==1920 && reportedHeight==1080);
    assert(outputWidth==1280 && outputHeight==720);
    assert(reportedFrames==1 && presentations==1 && !recording);

    SetFrameResolution(2560,1440);
    Present();
    assert(reportedWidth==2560 && reportedHeight==1440);

    SetFrameResolution(960,540);
    Present();
    assert(reportedWidth==960 && reportedHeight==540);

    // A direct-to-output movie/installer frame must not retain a scene override.
    Present();
    assert(reportedWidth==1280 && reportedHeight==720);
    SetFrameResolution(0,1080);
    Present();
    assert(reportedWidth==1280 && reportedHeight==720);

    // Docking changes output dimensions independently of the internal target.
    outputWidth=1920; outputHeight=1080;
    SetFrameResolution(1280,720);
    Present();
    assert(reportedWidth==1280 && reportedHeight==720);
    Present();
    assert(reportedWidth==1920 && reportedHeight==1080);

    const auto previousFrames=reportedFrames;
    queueError=true;
    try { Present(); assert(false); }
    catch(const std::runtime_error&) {}
    assert(reportedFrames==previousFrames);
}
