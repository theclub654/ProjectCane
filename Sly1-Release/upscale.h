#include <vector>
#include <cstdint>
#include <algorithm>
#include <cmath>

using byte = uint8_t;

// -------- sRGB<->linear + wrap/clamp --------
inline float srgbToLinear(float c) { return (c <= 0.04045f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f); }
inline float linearToSrgb(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    return (c <= 0.0031308f) ? 12.92f * c : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}
inline int wrapi(int v, int maxv) { int m = v % maxv; return (m < 0) ? m + maxv : m; }
inline int clampi(int v, int lo, int hi) { return (v < lo) ? lo : (v > hi ? hi : v); }

enum class SampleEdge { Wrap, Clamp };

// -------- 4x nearest upscale (RGBA8) --------
void nearestScaleRGBA4x(const std::vector<byte>& src, int w, int h,
    std::vector<byte>& dst)
{
    const int scale = 4;
    const int W = w * scale, H = h * scale;
    dst.resize(size_t(W) * H * 4);

    for (int y = 0; y < H; ++y) {
        int sy = y / scale;
        const byte* srow = src.data() + 4 * (size_t(sy) * w);
        byte* drow = dst.data() + 4 * (size_t(y) * W);
        for (int x = 0; x < W; ++x) {
            int sx = x / scale;
            const byte* sp = srow + 4 * sx;
            byte* dp = drow + 4 * x;
            dp[0] = sp[0]; dp[1] = sp[1]; dp[2] = sp[2]; dp[3] = sp[3];
        }
    }
}

// -------- separable 3-tap Gaussian blur (premultiplied+linear) --------
// kernel ~ [0.27901, 0.44198, 0.27901]  (sigma 0.85)
struct Kernel3 { float w0 = 0.27901f, w1 = 0.44198f, w2 = 0.27901f; };

static inline void fetchRGBA(const std::vector<byte>& img, int w, int h, int x, int y,
    SampleEdge edge, byte out[4])
{
    if (edge == SampleEdge::Wrap) { x = wrapi(x, w); y = wrapi(y, h); }
    else { x = clampi(x, 0, w - 1); y = clampi(y, 0, h - 1); }
    const byte* p = img.data() + 4 * (size_t(y) * w + x);
    out[0] = p[0]; out[1] = p[1]; out[2] = p[2]; out[3] = p[3];
}

void gaussian3_premult_linear(std::vector<byte>& img, int w, int h, SampleEdge edge)
{
    Kernel3 k;
    std::vector<byte> tmp(img.size());

    auto toLinPremult = [](const byte p[4], float& r, float& g, float& b, float& a) {
        a = p[3] / 255.0f;
        float sr = p[0] / 255.0f, sg = p[1] / 255.0f, sb = p[2] / 255.0f;
        // linearize RGB
        sr = srgbToLinear(sr); sg = srgbToLinear(sg); sb = srgbToLinear(sb);
        // premultiply
        r = sr * a; g = sg * a; b = sb * a;
        };
    auto fromLinPremult = [](float r, float g, float b, float a, byte out[4]) {
        if (a > 1e-8f) { r /= a; g /= a; b /= a; }
        else { r = g = b = 0.0f; }
        // back to sRGB
        r = linearToSrgb(r); g = linearToSrgb(g); b = linearToSrgb(b);
        out[0] = (byte)std::lround(std::clamp(r, 0.0f, 1.0f) * 255.0f);
        out[1] = (byte)std::lround(std::clamp(g, 0.0f, 1.0f) * 255.0f);
        out[2] = (byte)std::lround(std::clamp(b, 0.0f, 1.0f) * 255.0f);
        out[3] = (byte)std::lround(std::clamp(a, 0.0f, 1.0f) * 255.0f);
        };

    // Horizontal
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            byte p0[4], p1[4], p2[4];
            fetchRGBA(img, w, h, x - 1, y, edge, p0);
            fetchRGBA(img, w, h, x, y, edge, p1);
            fetchRGBA(img, w, h, x + 1, y, edge, p2);

            float r0, g0, b0, a0, r1, g1, b1, a1, r2, g2, b2, a2;
            toLinPremult(p0, r0, g0, b0, a0);
            toLinPremult(p1, r1, g1, b1, a1);
            toLinPremult(p2, r2, g2, b2, a2);

            float r = k.w0 * r0 + k.w1 * r1 + k.w2 * r2;
            float g = k.w0 * g0 + k.w1 * g1 + k.w2 * g2;
            float b = k.w0 * b0 + k.w1 * b1 + k.w2 * b2;
            float a = k.w0 * a0 + k.w1 * a1 + k.w2 * a2;

            byte* d = tmp.data() + 4 * (size_t(y) * w + x);
            byte out[4]; fromLinPremult(r, g, b, a, out);
            d[0] = out[0]; d[1] = out[1]; d[2] = out[2]; d[3] = out[3];
        }
    }
    img.swap(tmp);

    // Vertical
    tmp.resize(img.size());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            byte p0[4], p1[4], p2[4];
            fetchRGBA(img, w, h, x, y - 1, edge, p0);
            fetchRGBA(img, w, h, x, y, edge, p1);
            fetchRGBA(img, w, h, x, y + 1, edge, p2);

            float r0, g0, b0, a0, r1, g1, b1, a1, r2, g2, b2, a2;
            toLinPremult(p0, r0, g0, b0, a0);
            toLinPremult(p1, r1, g1, b1, a1);
            toLinPremult(p2, r2, g2, b2, a2);

            float r = k.w0 * r0 + k.w1 * r1 + k.w2 * r2;
            float g = k.w0 * g0 + k.w1 * g1 + k.w2 * g2;
            float b = k.w0 * b0 + k.w1 * b1 + k.w2 * b2;
            float a = k.w0 * a0 + k.w1 * a1 + k.w2 * a2;

            byte* d = tmp.data() + 4 * (size_t(y) * w + x);
            byte out[4]; fromLinPremult(r, g, b, a, out);
            d[0] = out[0]; d[1] = out[1]; d[2] = out[2]; d[3] = out[3];
        }
    }
    img.swap(tmp);
}

// -------- gamma-aware bilinear downsample (RGBA8) --------
void resizeRGBA_bilinear_gamma(const std::vector<byte>& src, int srcW, int srcH,
    std::vector<byte>& dst, int dstW, int dstH,
    SampleEdge edge)
{
    dst.resize(size_t(dstW) * dstH * 4);
    const float sx = float(srcW) / float(dstW);
    const float sy = float(srcH) / float(dstH);

    auto sample = [&](int x, int y, byte out[4]) {
        fetchRGBA(src, srcW, srcH, x, y, edge, out);
        };

    for (int y = 0; y < dstH; ++y) {
        float syf = (y + 0.5f) * sy - 0.5f;
        int y0 = (int)std::floor(syf), y1 = y0 + 1;
        float fy = syf - y0;

        for (int x = 0; x < dstW; ++x) {
            float sxf = (x + 0.5f) * sx - 0.5f;
            int x0 = (int)std::floor(sxf), x1 = x0 + 1;
            float fx = sxf - x0;

            byte p00[4], p10[4], p01[4], p11[4];
            sample(x0, y0, p00); sample(x1, y0, p10); sample(x0, y1, p01); sample(x1, y1, p11);

            // linearize RGB, alpha stays linear; (optionally premult, but after blur it's not critical)
            auto RGBL = [&](const byte p[4], float& r, float& g, float& b, float& a) {
                a = p[3] / 255.0f;
                r = srgbToLinear(p[0] / 255.0f);
                g = srgbToLinear(p[1] / 255.0f);
                b = srgbToLinear(p[2] / 255.0f);
                };

            float r00, g00, b00, a00, r10, g10, b10, a10, r01, g01, b01, a01, r11, g11, b11, a11;
            RGBL(p00, r00, g00, b00, a00); RGBL(p10, r10, g10, b10, a10);
            RGBL(p01, r01, g01, b01, a01); RGBL(p11, r11, g11, b11, a11);

            auto mix = [&](float a, float b, float t) { return a * (1.0f - t) + b * t; };
            float r0 = mix(r00, r10, fx), r1 = mix(r01, r11, fx);
            float g0 = mix(g00, g10, fx), g1 = mix(g01, g11, fx);
            float b0 = mix(b00, b10, fx), b1 = mix(b01, b11, fx);
            float a0 = mix(a00, a10, fx), a1 = mix(a01, a11, fx);

            float r = mix(r0, r1, fy);
            float g = mix(g0, g1, fy);
            float b = mix(b0, b1, fy);
            float a = mix(a0, a1, fy);

            // back to sRGB
            r = linearToSrgb(r); g = linearToSrgb(g); b = linearToSrgb(b);

            byte* d = dst.data() + 4 * (size_t(y) * dstW + x);
            d[0] = (byte)std::lround(std::clamp(r, 0.0f, 1.0f) * 255.0f);
            d[1] = (byte)std::lround(std::clamp(g, 0.0f, 1.0f) * 255.0f);
            d[2] = (byte)std::lround(std::clamp(b, 0.0f, 1.0f) * 255.0f);
            d[3] = (byte)std::lround(std::clamp(a, 0.0f, 1.0f) * 255.0f);
        }
    }
}
