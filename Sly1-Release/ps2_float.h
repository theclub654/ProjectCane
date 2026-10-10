#pragma once

#include <cfenv>

#if defined(__SSE__) || defined(_M_IX86) || defined(_M_X64)
#include <xmmintrin.h>
#define CANE_PS2_FLOAT_X86 1
#endif

// The EE and VU execute with chop-to-zero rounding and flush denormal inputs
// and results to zero. Keep this state local to the translated PS2 operation
// so native UI, audio, and renderer code retain the host floating-point mode.
class ScopedPs2FloatMode
{
public:
	ScopedPs2FloatMode()
		: m_roundMode(std::fegetround())
#if defined(CANE_PS2_FLOAT_X86)
		, m_mxcsr(_mm_getcsr())
#endif
	{
		std::fesetround(FE_TOWARDZERO);

#if defined(CANE_PS2_FLOAT_X86)
		constexpr unsigned int kRoundingMask = 3u << 13;
		constexpr unsigned int kRoundTowardZero = 3u << 13;
		constexpr unsigned int kDenormalsAreZero = 1u << 6;
		constexpr unsigned int kFlushToZero = 1u << 15;

		unsigned int mxcsr = _mm_getcsr();
		mxcsr = (mxcsr & ~kRoundingMask) | kRoundTowardZero;
		mxcsr |= kDenormalsAreZero | kFlushToZero;
		_mm_setcsr(mxcsr);
#endif
}
	~ScopedPs2FloatMode()
	{
#if defined(CANE_PS2_FLOAT_X86)
		_mm_setcsr(m_mxcsr);
#endif
		std::fesetround(m_roundMode);
	}

	ScopedPs2FloatMode(const ScopedPs2FloatMode&) = delete;
	ScopedPs2FloatMode& operator=(const ScopedPs2FloatMode&) = delete;

private:
	int m_roundMode;
#if defined(CANE_PS2_FLOAT_X86)
	unsigned int m_mxcsr;
#endif
};
