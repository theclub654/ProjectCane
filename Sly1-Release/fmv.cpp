#include "fmv.h"
#include "game.h"
#include "screen.h"
#include "sound.h"
#include "ui.h"

#include <Windows.h>
#include <mmsystem.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <thread>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
}

namespace
{
    constexpr uint32_t STARTUP_SPLASH_SECTOR = 0x000C8F03;
    constexpr OID STARTUP_SPLASH_QUEUE_ID = static_cast<OID>(0x10000344);

    // Movie events may contain cutscene table index 0, so OID_Unknown (also 0)
    // cannot be used as the empty-queue marker.
    OID s_aoidPending[2] = { OID_Nil, OID_Nil };
    bool s_afSkippable[2] = {};

    // Identity word (descriptor[2] ^ descriptor[6]) for each of the retail
    // game's encrypted 32-byte cutscene descriptors. The runtime-loaded
    // startup splash shares identity 0x344 with cutscene 16, so its sector is
    // checked separately in QueueCutsceneDescriptor.
    constexpr uint32_t s_cutsceneDescriptorKeys[] =
    {
        0x340, 0x341, 0x33C, 0x33F,
        0x345, 0x346, 0x33D, 0x33E,
        0x347, 0x348, 0x342, 0x343,
        0x336, 0x33A, 0x33B, 0x338,
        0x344, 0x344
    };

    const char* const s_achzFmvTitle[] =
    {
        "The Secret Police File",
        "The Tokyo Police File",
        "The Making Of Sly Cooper",
        "Commercials & Outtakes",
        "Tide of Terror",
        "Raleigh Hung out to Dry",
        "Sunset Snake Eyes",
        "Muggshot Busted",
        "Vicious Voodoo",
        "Mz Ruby Vexed",
        "Fire in the Sky",
        "The Panda King on Ice",
        "The Cold Heart of Hate",
        "Ten Seconds",
        "The Thievius Raccoonus",
        "Credits",
        "Additional Cutscene",
        "Startup Splash"
    };

    int Mod4(int value)
    {
        value %= 4;
        return value < 0 ? value + 4 : value;
    }

    CFontBrx* FmvPreviewFont()
    {
        CFontBrx* preferred = FFontLoaded(4) ? PfontFromFont(4) : nullptr;
        if (preferred != nullptr && preferred->m_pbmp != nullptr &&
            preferred->m_pbmp->glDiffuseMap != 0 &&
            preferred->FValid('_') && preferred->FValid('a'))
            return preferred;

        // Some converted BRX files do not preserve the retail font ordering.
        // Locate the picture-glyph atlas by its FMV glyph set instead.
        CFontBrx* partialMatch = nullptr;
        for (CFontBrx& font : g_afontBrx)
        {
            if (font.m_pbmp == nullptr || font.m_pbmp->glDiffuseMap == 0 ||
                !font.FValid('_') || !font.FValid('a'))
                continue;

            if (partialMatch == nullptr)
                partialMatch = &font;

            bool hasEveryPreview = true;
            for (char ch = 'a'; ch <= 'p'; ++ch)
            {
                if (!font.FValid(ch))
                {
                    hasEveryPreview = false;
                    break;
                }
            }

            if (hasEveryPreview)
                return &font;
        }

        return partialMatch;
    }

    std::filesystem::path CutsceneDirectory(const std::string& videoFileName)
    {
        wchar_t executable[MAX_PATH]{};
        const DWORD length = GetModuleFileNameW(nullptr, executable, MAX_PATH);
        if (length != 0 && length < MAX_PATH)
        {
        const std::filesystem::path besideExecutable =
            std::filesystem::path(executable).parent_path() / L"Cutscenes";
            if (std::filesystem::exists(besideExecutable / videoFileName))
                return besideExecutable;
        }

    const std::filesystem::path workingDirectory =
        std::filesystem::current_path() / L"Cutscenes";
        if (std::filesystem::exists(workingDirectory / videoFileName))
            return workingDirectory;

    const std::filesystem::path projectDirectory =
        std::filesystem::current_path() / L"Sly1-Release" / L"Cutscenes";
        if (std::filesystem::exists(projectDirectory / videoFileName))
            return projectDirectory;

        return workingDirectory;
    }

    std::string AvError(int error)
    {
        char message[AV_ERROR_MAX_STRING_SIZE]{};
        av_strerror(error, message, sizeof(message));
        return message;
    }

    bool SkipPressed()
    {
        constexpr JOY_BUTTON buttons[] =
        {
            BTN_L2, BTN_R2, BTN_L1, BTN_R1, BTN_TRIANGLE, BTN_SELECT, BTN_START
        };
        for (JOY_BUTTON button : buttons)
        {
            if (g_joy.IsPressed(button))
            {
                for (JOY_BUTTON handled : buttons)
                    g_joy.SetHandled(handled);
                return true;
            }
        }
        return false;
    }

    bool PresentCutsceneFrame(GLuint texture, int videoWidth, int videoHeight)
    {
        int framebufferWidth = 0;
        int framebufferHeight = 0;
        glfwGetFramebufferSize(g_gl.window, &framebufferWidth, &framebufferHeight);
        if (framebufferWidth <= 0 || framebufferHeight <= 0)
            return false;

        const float videoAspect = static_cast<float>(videoWidth) / static_cast<float>(videoHeight);
        const float windowAspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
        int width = framebufferWidth;
        int height = framebufferHeight;
        int x = 0;
        int y = 0;
        if (windowAspect > videoAspect)
        {
            width = static_cast<int>(framebufferHeight * videoAspect);
            x = (framebufferWidth - width) / 2;
        }
        else
        {
            height = static_cast<int>(framebufferWidth / videoAspect);
            y = (framebufferHeight - height) / 2;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, framebufferWidth, framebufferHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glViewport(x, y, width, height);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glScreenShader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(g_gl.sao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(g_gl.window);
        return true;
    }

    bool PlayCutscene(const CUTSCENEINFO& info, bool skippable)
    {
        const std::filesystem::path directory = CutsceneDirectory(info.videoFileName);
        const std::filesystem::path videoPath = directory / info.videoFileName;
        const std::filesystem::path audioPath = directory / info.audioFileName;

        AVFormatContext* format = nullptr;
        int result = avformat_open_input(&format, videoPath.string().c_str(), nullptr, nullptr);
        if (result < 0)
        {
            std::cerr << "Failed to open cutscene video: " << videoPath << " (" << AvError(result) << ")\n";
            return false;
        }

        AVCodecContext* decoder = nullptr;
        AVFrame* frame = nullptr;
        AVPacket* packet = nullptr;
        SwsContext* scaler = nullptr;
        GLuint texture = 0;
        bool played = false;

        do
        {
            result = avformat_find_stream_info(format, nullptr);
            if (result < 0)
                break;
            const int streamIndex = av_find_best_stream(format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
            if (streamIndex < 0)
            {
                result = streamIndex;
                break;
            }

            AVStream* stream = format->streams[streamIndex];
            const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
            if (codec == nullptr)
            {
                result = AVERROR_DECODER_NOT_FOUND;
                break;
            }
            decoder = avcodec_alloc_context3(codec);
            if (decoder == nullptr)
            {
                result = AVERROR(ENOMEM);
                break;
            }
            if ((result = avcodec_parameters_to_context(decoder, stream->codecpar)) < 0 ||
                (result = avcodec_open2(decoder, codec, nullptr)) < 0)
                break;

            frame = av_frame_alloc();
            packet = av_packet_alloc();
            if (frame == nullptr || packet == nullptr)
            {
                result = AVERROR(ENOMEM);
                break;
            }

            scaler = sws_getContext(decoder->width, decoder->height, decoder->pix_fmt,
                decoder->width, decoder->height, AV_PIX_FMT_RGBA, SWS_BILINEAR,
                nullptr, nullptr, nullptr);
            if (scaler == nullptr)
            {
                result = AVERROR(EINVAL);
                break;
            }

            std::vector<uint8_t> pixels(static_cast<size_t>(decoder->width) * decoder->height * 4);
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, decoder->width, decoder->height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            AVRational rate = av_guess_frame_rate(format, stream, nullptr);
            double fps = av_q2d(rate);
            if (fps <= 1.0 || fps > 120.0)
                fps = 29.97;

            const auto start = std::chrono::steady_clock::now();
            int64_t frameNumber = 0;
            bool quit = false;
            bool audioStarted = false;

            auto outputFrames = [&]()
            {
                while (!quit)
                {
                    const int receive = avcodec_receive_frame(decoder, frame);
                    if (receive == AVERROR(EAGAIN) || receive == AVERROR_EOF)
                        return;
                    if (receive < 0)
                    {
                        result = receive;
                        quit = true;
                        return;
                    }

                    uint8_t* destination[] =
                    {
                        pixels.data() + static_cast<size_t>(decoder->height - 1) * decoder->width * 4
                    };
                    int destinationStride[] = { -decoder->width * 4 };
                    sws_scale(scaler, frame->data, frame->linesize, 0, decoder->height,
                        destination, destinationStride);

                    if (!audioStarted)
                    {
                        if (!PlaySoundW(audioPath.c_str(), nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT))
                            std::cerr << "Failed to start cutscene audio: " << audioPath << '\n';
                        audioStarted = true;
                    }

                    const auto due = start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                        std::chrono::duration<double>(static_cast<double>(frameNumber) / fps));
                    while (!quit && std::chrono::steady_clock::now() < due)
                    {
                        glfwPollEvents();
                        g_joy.Update(g_gl.window);
                        if (glfwWindowShouldClose(g_gl.window) || (skippable && SkipPressed()))
                            quit = true;
                        if (!quit)
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    }
                    if (quit)
                        return;

                    glBindTexture(GL_TEXTURE_2D, texture);
                    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, decoder->width, decoder->height,
                        GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                    PresentCutsceneFrame(texture, decoder->width, decoder->height);
                    glfwPollEvents();
                    g_joy.Update(g_gl.window);
                    if (glfwWindowShouldClose(g_gl.window) || (skippable && SkipPressed()))
                        quit = true;
                    ++frameNumber;
                    played = true;
                }
            };

            while (!quit && av_read_frame(format, packet) >= 0)
            {
                if (packet->stream_index == streamIndex)
                {
                    result = avcodec_send_packet(decoder, packet);
                    if (result >= 0)
                        outputFrames();
                }
                av_packet_unref(packet);
            }
            if (!quit)
            {
                avcodec_send_packet(decoder, nullptr);
                outputFrames();
            }
            result = 0;
        } while (false);

        PlaySoundW(nullptr, nullptr, 0);
        if (texture != 0)
            glDeleteTextures(1, &texture);
        sws_freeContext(scaler);
        av_packet_free(&packet);
        av_frame_free(&frame);
        avcodec_free_context(&decoder);
        avformat_close_input(&format);

        if (result < 0)
            std::cerr << "Cutscene decode failed for " << videoPath << ": " << AvError(result) << '\n';
        return played;
    }
}

void QueueCutscene(OID identityId)
{
    const CUTSCENEINFO* info = PcutsceneinfoFromIdentityId(identityId);
    // Retail event data sometimes stores the table index directly instead of
    // the identity OID. In particular, the first cutscene is encoded as zero.
    if (info == nullptr)
        info = PcutsceneinfoFromId(static_cast<int>(identityId));

    if (info == nullptr)
    {
        std::cerr << "Unknown cutscene OID: " << static_cast<int>(identityId) << '\n';
        return;
    }

    if (g_pgsCur != nullptr)
        g_pgsCur->unlockedCutscenes |= 1U << info->cutsceneId;

    const int slot = s_aoidPending[0] == OID_Nil ? 0 :
                     s_aoidPending[1] == OID_Nil ? 1 : -1;
    if (slot >= 0)
    {
        // Normalize queued values to the table identity so both encodings
        // follow exactly the same playback path.
        s_aoidPending[slot] = info->identityId;
        // Cutscene playback polls JOY directly, so allow Start/Options to
        // skip a movie even on its first viewing.
        s_afSkippable[slot] = true;
    }
}

void QueueCutsceneDescriptor(const char* descriptor)
{
    if (descriptor == nullptr)
    {
        std::cerr << "Null cutscene descriptor\n";
        return;
    }

    uint32_t encodedSectorA = 0;
    uint32_t encodedSectorB = 0;
    uint32_t encodedIdentityA = 0;
    uint32_t encodedIdentityB = 0;
    std::memcpy(&encodedSectorA, descriptor, sizeof(encodedSectorA));
    std::memcpy(&encodedSectorB, descriptor + 20, sizeof(encodedSectorB));
    std::memcpy(&encodedIdentityA, descriptor + 8, sizeof(encodedIdentityA));
    std::memcpy(&encodedIdentityB, descriptor + 24, sizeof(encodedIdentityB));
    const uint32_t sector = encodedSectorA ^ encodedSectorB;
    const uint32_t identityKey = encodedIdentityA ^ encodedIdentityB;

    if (sector == STARTUP_SPLASH_SECTOR && identityKey == 0x344)
    {
        QueueCutscene(STARTUP_SPLASH_QUEUE_ID);
        return;
    }

    for (int cutsceneId = 0; cutsceneId < CUTSCENE_COUNT; ++cutsceneId)
    {
        if (s_cutsceneDescriptorKeys[cutsceneId] == identityKey)
        {
            QueueCutscene(g_cutsceneTable[cutsceneId].identityId);
            return;
        }
    }

    std::cerr << "Unknown cutscene descriptor identity: 0x" << std::hex
              << identityKey << std::dec << '\n';
}

bool FCutscenePending()
{
    return s_aoidPending[0] != OID_Nil;
}

void ExecutePendingCutscenes()
{
    OID pending[2] = { s_aoidPending[0], s_aoidPending[1] };
    bool skippable[2] = { s_afSkippable[0], s_afSkippable[1] };
    s_aoidPending[0] = s_aoidPending[1] = OID_Nil;
    s_afSkippable[0] = s_afSkippable[1] = false;

    for (int i = 0; i < 2 && pending[i] != OID_Nil; ++i)
    {
        const CUTSCENEINFO* info = PcutsceneinfoFromIdentityId(pending[i]);
        if (info != nullptr)
            PlayCutscene(*info, skippable[i]);
    }
}

void StartupFmvMenu(FMVMENU* pfmvmenu)
{
	g_teFmv = {};
	g_teFmv.m_rgba = glm::vec4(0.0f, 75.0f / 255.0f, 125.0f / 255.0f, 1.0f);
	g_teFmv.m_ch = '-';
	g_teFmv.m_dyExtra = 1.0f;
	g_teFmv.m_ryScaling = 0.3f;
	g_teFmv.m_rxScaling = 0.3f;
	g_teFmv.m_dxExtra = 2.0f;

    pfmvmenu->pvtfmv = &g_vtfmvmenu;
}

void PostFmvMenuLoad(FMVMENU* pfmvmenu)
{
    PostBlotLoad(pfmvmenu);
    if (FFontLoaded(2))
    {
        pfmvmenu->pte = &g_teFmv;
        g_teFmv.m_pfont = PfontFromFont(2);
    }

    if (pfmvmenu->pfont != nullptr)
        pfmvmenu->pfont = pfmvmenu->pfont->PfontClone(1.0f, 1.0f);

    pfmvmenu->rgba = glm::vec4(127.0f / 255.0f, 127.0f / 255.0f,
                               127.0f / 255.0f, 223.0f / 255.0f);

    CFontBrx* pfontIcons = FmvPreviewFont();
    if (pfmvmenu->pfont == nullptr || pfontIcons == nullptr)
        return;

    pfontIcons->PushScaling(1.3f, 1.3f);
    const float dx = 90.0f + pfontIcons->DxFromCh('_') * 4.0f;
    const float dy = static_cast<float>(pfmvmenu->pfont->m_dyUnscaled) * pfmvmenu->pfont->m_ryScale
                   + 60.0f
                   + static_cast<float>(pfontIcons->m_dyUnscaled) * pfontIcons->m_ryScale * 4.0f;
    pfontIcons->PopScaling();
    ResizeBlot(pfmvmenu, dx, dy);
}

uint32_t FFmvUnlocked(int iCutscene)
{
    if (g_pgsCur == nullptr || iCutscene < 0 || iCutscene >= CUTSCENE_COUNT)
        return 0;
    return static_cast<uint32_t>(g_pgsCur->unlockedCutscenes) & (1U << iCutscene);
}

const char* PchzFriendlyFromCid(int iCutscene)
{
    if (iCutscene >= 0 && iCutscene < CUTSCENE_COUNT)
        return s_achzFmvTitle[iCutscene];
    return iCutscene == -1 ? "Nil" : "(unknown)";
}

const CUTSCENEINFO* PcutsceneinfoFromId(int cutsceneId)
{
    if (cutsceneId < 0 || cutsceneId >= CUTSCENE_COUNT)
        return nullptr;

    return &g_cutsceneTable[cutsceneId];
}

const CUTSCENEINFO* PcutsceneinfoFromIdentityId(OID identityId)
{
    for (const CUTSCENEINFO& cutsceneInfo : g_cutsceneTable)
    {
        if (cutsceneInfo.identityId == identityId)
            return &cutsceneInfo;
    }

    return nullptr;
}

void OnFmvMenuActive(FMVMENU* pfmvmenu, int fActive)
{
    const bool activate = fActive != 0;
    if (activate == (pfmvmenu->fActive != 0))
        return;

    if (!activate)
    {
        RemoveGrfusr(1);
        pfmvmenu->pvtblot->pfnHideBlot(pfmvmenu);
    }
    else
    {
        AddGrfusr(1);
        g_joy.StartJoySelection();
        pfmvmenu->pvtblot->pfnShowBlot(pfmvmenu);

        int iCutscene = 15;
        while (iCutscene > 0 && FFmvUnlocked(iCutscene) == 0)
            --iCutscene;
        pfmvmenu->iCutscene = iCutscene;
    }

    pfmvmenu->fActive = activate;
}

void UpdateFmvMenuActive(FMVMENU* pfmvmenu, JOY* pjoy)
{
    if (pjoy == nullptr)
        return;

    const bool exit = pjoy->IsPressed(BTN_TRIANGLE) || pjoy->IsPressed(BTN_SELECT) ||
                      pjoy->IsPressed(BTN_START) ||
                      (pjoy->joys == JOYS_Searching && g_clock.t >= 5.0f);
    if (exit)
    {
        pjoy->SetHandled(BTN_TRIANGLE);
        pjoy->SetHandled(BTN_SELECT);
        pjoy->SetHandled(BTN_START);
        PopUiActiveBlot(&g_ui);
        return;
    }

    if (pjoy->IsPressed(BTN_CROSS))
    {
        pjoy->SetHandled(BTN_CROSS);
        if (FFmvUnlocked(pfmvmenu->iCutscene) == 0)
            StartSound(static_cast<SFXID>(123), nullptr, nullptr, nullptr,
                       3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
        else
            QueueCutscene(g_cutsceneTable[pfmvmenu->iCutscene].identityId);
        return;
    }

    const int dx = pjoy->DxSelectionJoy(g_clock.tReal);
    const int dyInput = pjoy->DySelectionJoy(g_clock.tReal);
    if (dx == 0 && dyInput == 0)
        return;

    pjoy->tLatchX = g_clock.tReal + pjoy->dtLatchX;
    pjoy->tLatchY = g_clock.tReal + pjoy->dtLatchY;

    const int old = pfmvmenu->iCutscene;
    const int column = Mod4(old);
    int dy = dyInput;
    if (dx < 0 && column == 0)
        dy = -1;
    if (dx > 0 && column == 3)
        dy = 1;

    const int row = Mod4(old / 4 + dy);
    pfmvmenu->iCutscene = row * 4 + Mod4(column + dx);

    StartSound(static_cast<SFXID>(121), nullptr, nullptr, nullptr,
               3000.0f, 300.0f, 1.0f, 0.0f, 0.0f, nullptr, nullptr);
}

void DrawFmvMenu(FMVMENU* pfmvmenu)
{
    if (pfmvmenu == nullptr || pfmvmenu->pfont == nullptr)
        return;

    glm::vec4 color = pfmvmenu->rgba;
    color.a *= pfmvmenu->uOn;

    CTextBox textBox;
    textBox.SetPos(pfmvmenu->x, pfmvmenu->y);
    textBox.SetSize(pfmvmenu->dx, pfmvmenu->dy);
    textBox.SetTextColor(&color);
    textBox.SetHorizontalJust(JH_Center);
    textBox.SetVerticalJust(JV_Top);

    if (pfmvmenu->pte != nullptr && pfmvmenu->pte->m_pfont != nullptr)
    {
        const glm::vec4 edgeColor = pfmvmenu->pte->m_rgba;
        pfmvmenu->pte->m_rgba.a *= pfmvmenu->uOn;
        pfmvmenu->pte->m_pfont->EdgeRect(pfmvmenu->pte, &textBox);
        pfmvmenu->pte->m_rgba = edgeColor;
    }

    char* title = const_cast<char*>(FFmvUnlocked(pfmvmenu->iCutscene)
        ? PchzFriendlyFromCid(pfmvmenu->iCutscene)
        : "Locked");
    pfmvmenu->pfont->DrawPchz(title, &textBox);

    CFontBrx* pfontIcons = FmvPreviewFont();
    if (pfontIcons == nullptr)
        return;

    GLYFF* placeholderGlyph = pfontIcons->PglyffFromCh('_');
    if (placeholderGlyph == nullptr || pfontIcons->m_pbmp == nullptr)
        return;

    const float previewScale = 1.3f;
    const float glyphWidth = static_cast<float>(placeholderGlyph->dx + 1)
        * pfontIcons->m_rxScale * previewScale;
    const float glyphHeight = static_cast<float>(pfontIcons->m_dyUnscaled)
        * pfontIcons->m_ryScale * previewScale;
    const float xStart = pfmvmenu->x + 18.0f + glyphWidth * 0.5f;
    const float yStart = pfmvmenu->y
        + static_cast<float>(pfmvmenu->pfont->m_dyUnscaled) * pfmvmenu->pfont->m_ryScale
        + 12.0f
        + glyphHeight * 0.5f;
    const float xStep = glyphWidth + 18.0f;
    const float yStep = glyphHeight + 12.0f;

    // The FMV atlas stores preview images as glyphs, but these are fixed-size
    // image cells rather than text. Draw the atlas rectangles directly so
    // text justification and nested font scaling cannot move them offscreen.
    pfontIcons->SetupDraw();

    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
        {
            const int index = row * 4 + column;
            const char icon = FFmvUnlocked(index) ? static_cast<char>('a' + index) : '_';
            GLYFF* glyph = pfontIcons->PglyffFromCh(icon);
            if (glyph == nullptr)
                continue;

            float pulse = 1.0f;
            if (index == pfmvmenu->iCutscene)
                pulse = 1.0f + (1.0f - (std::sin(g_clock.tReal * 10.0f) * 0.5f + 0.5f)) * 0.1f;

            const float centerX = xStart + column * xStep;
            const float centerY = yStart + row * yStep;
            const float width = glyphWidth * pfmvmenu->uOn * pulse;
            const float height = glyphHeight * pfmvmenu->uOn * pulse;

            const float s0 = glyph->x / static_cast<float>(pfontIcons->m_pbmp->bmpWidth);
            const float t0 = glyph->y / static_cast<float>(pfontIcons->m_pbmp->bmpHeight);
            const float s1 = (glyph->x + glyph->dx) / static_cast<float>(pfontIcons->m_pbmp->bmpWidth);
            const float t1 = (glyph->y + pfontIcons->m_dyUnscaled)
                / static_cast<float>(pfontIcons->m_pbmp->bmpHeight);

            const glm::mat4 model =
                glm::translate(glm::mat4(1.0f), glm::vec3(centerX - width * 0.5f,
                                                          centerY - height * 0.5f, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
            glUniformMatrix4fv(u_modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniform4f(uvRectLoc, s0, t0, s1, t1);
            glUniform4fv(blotColorLoc, 1, glm::value_ptr(color));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        }
    }
    pfontIcons->CleanUpDraw();

    const float lineY = pfmvmenu->y
        + static_cast<float>(pfmvmenu->pfont->m_dyUnscaled) * pfmvmenu->pfont->m_ryScale - 1.0f;
    DrawLineScreen(pfmvmenu->x, lineY, 0.0f, pfmvmenu->x + pfmvmenu->dx,
                   lineY, 0.0f, g_teFmv.m_rgba, false);
}

CTextEdge g_teFmv;
VTFMV g_vtfmvmenu;
FMVMENU g_fmvmenu;

CUTSCENEINFO g_cutsceneTable[] =
{
    {  0, static_cast<OID>(0x340), "Cutscene_00.m2v", "Cutscene_00.wav" },
    {  1, static_cast<OID>(0x341), "Cutscene_01.m2v", "Cutscene_01.wav" },
    {  2, static_cast<OID>(0x33C), "Cutscene_02.m2v", "Cutscene_02.wav" },
    {  3, static_cast<OID>(0x33F), "Cutscene_03.m2v", "Cutscene_03.wav" },
    {  4, static_cast<OID>(0x345), "Cutscene_04.m2v", "Cutscene_04.wav" },
    {  5, static_cast<OID>(0x346), "Cutscene_05.m2v", "Cutscene_05.wav" },
    {  6, static_cast<OID>(0x33D), "Cutscene_06.m2v", "Cutscene_06.wav" },
    {  7, static_cast<OID>(0x33E), "Cutscene_07.m2v", "Cutscene_07.wav" },
    {  8, static_cast<OID>(0x347), "Cutscene_08.m2v", "Cutscene_08.wav" },
    {  9, static_cast<OID>(0x348), "Cutscene_09.m2v", "Cutscene_09.wav" },
    { 10, static_cast<OID>(0x342), "Cutscene_10.m2v", "Cutscene_10.wav" },
    { 11, static_cast<OID>(0x343), "Cutscene_11.m2v", "Cutscene_11.wav" },
    { 12, static_cast<OID>(0x336), "Cutscene_12.m2v", "Cutscene_12.wav" },
    { 13, static_cast<OID>(0x33A), "Cutscene_13.m2v", "Cutscene_13.wav" },
    { 14, static_cast<OID>(0x33B), "Cutscene_14.m2v", "Cutscene_14.wav" },
    { 15, static_cast<OID>(0x338), "Cutscene_15.m2v", "Cutscene_15.wav" },
    { 16, static_cast<OID>(0x344), "Cutscene_16.m2v", "Cutscene_16.wav" },
    { 17, STARTUP_SPLASH_QUEUE_ID, "Cutscene_17.m2v", "Cutscene_17.wav" }
};
