$ErrorActionPreference = 'Stop'

$path = 'C:\Users\denar\source\repos\pcsx2-master\pcsx2\Interpreter.cpp'
$text = [IO.File]::ReadAllText($path)

$begin = '// BEGIN PROJECTCANE ROULETTE RPL TRACE'
$end = '// END PROJECTCANE ROULETTE RPL TRACE'

if ($text.Contains($begin)) {
    $pattern = [regex]::Escape($begin) + '.*?' + [regex]::Escape($end) + "\r?\n"
    $text = [regex]::Replace($text, $pattern, '', [Text.RegularExpressions.RegexOptions]::Singleline)
}

# Remove calls left by an earlier installer revision. The trace belongs only in
# execI(), after that function captures the current EE PC.
$text = $text.Replace("`r`n`tTraceProjectCaneRouletteRpl(pc);", '')
$text = $text.Replace("`n`tTraceProjectCaneRouletteRpl(pc);", '')

$helper = @'
// BEGIN PROJECTCANE ROULETTE RPL TRACE
static void TraceProjectCaneRouletteRpl(u32 pc)
{
	// Retail SCUS-971.98 SubmitRpl(RPL*) entry.
	if (pc != 0x0019d410)
		return;

	const u32 rpl = cpuRegs.GPR.n.a0.UL[0];
	if (rpl == 0)
		return;

	// Original 32-bit RPL layout built by RenderAloGlobset:
	// +0x70 ALO*, +0x74 GLOBI*, +0x78 GLOB*, +0x7c GLOBI/aux*.
	const u32 palo = memRead32(rpl + 0x70);
	if (palo == 0)
		return;

	const s16 oid = static_cast<s16>(memRead16(palo + 0x04));
	if (oid < 1581 || oid > 1587)
		return;

	const u32 pglobi = memRead32(rpl + 0x74);
	const u32 pglob = memRead32(rpl + 0x78);
	const u32 paux = memRead32(rpl + 0x7c);
	const u32 rp = memRead32(rpl + 0x08);
	const u32 alphaBits = memRead32(rpl + 0x54);

	// Print a packet only when its meaningful render identity changes. This keeps
	// the log readable while still exposing alarm state transitions.
	static u64 lastSignature[7] = {};
	u64 signature = (static_cast<u64>(pglob) << 32) ^ pglobi ^
		(static_cast<u64>(paux) << 7) ^ (static_cast<u64>(rp) << 19) ^ alphaBits;
	const int slot = oid - 1581;
	if (lastSignature[slot] == signature)
		return;
	lastSignature[slot] = signature;

	Console.WriteLn("[ROULETTE ORIGINAL RPL] oid=%d pc=%08X rpl=%08X alo=%08X rp=%u alphaBits=%08X pglobi=%08X glob=%08X aux=%08X",
		oid, pc, rpl, palo, rp, alphaBits, pglobi, pglob, paux);

	if (pglob != 0)
	{
		Console.WriteLn("[ROULETTE ORIGINAL GLOB] oid=%d g00=%08X g04=%08X g08=%08X g0C=%08X g10=%08X g14=%08X g18=%08X g1C=%08X g20=%08X g24=%08X g28=%08X g2C=%08X g30=%08X g34=%08X g38=%08X g3C=%08X g40=%08X g44=%08X g48=%08X g4C=%08X g50=%08X g54=%08X g58=%08X g5C=%08X g60=%08X g64=%08X g68=%08X g6C=%08X",
			oid,
			memRead32(pglob + 0x00), memRead32(pglob + 0x04), memRead32(pglob + 0x08), memRead32(pglob + 0x0c),
			memRead32(pglob + 0x10), memRead32(pglob + 0x14), memRead32(pglob + 0x18), memRead32(pglob + 0x1c),
			memRead32(pglob + 0x20), memRead32(pglob + 0x24), memRead32(pglob + 0x28), memRead32(pglob + 0x2c),
			memRead32(pglob + 0x30), memRead32(pglob + 0x34), memRead32(pglob + 0x38), memRead32(pglob + 0x3c),
			memRead32(pglob + 0x40), memRead32(pglob + 0x44), memRead32(pglob + 0x48), memRead32(pglob + 0x4c),
			memRead32(pglob + 0x50), memRead32(pglob + 0x54), memRead32(pglob + 0x58), memRead32(pglob + 0x5c),
			memRead32(pglob + 0x60), memRead32(pglob + 0x64), memRead32(pglob + 0x68), memRead32(pglob + 0x6c));
	}
}
// END PROJECTCANE ROULETTE RPL TRACE

'@

$anchor = 'static void execI()'
if (-not $text.Contains($anchor)) { throw "Could not find execI anchor in $path" }
$text = $text.Replace($anchor, $helper + $anchor)

$execAnchor = 'static void execI()'
if (-not $text.Contains($execAnchor)) { throw "Could not find execI body anchor in $path" }
$pcAnchor = "`tconst u32 pc = cpuRegs.pc;"
$execStart = $text.IndexOf($execAnchor)
$pcAt = $text.IndexOf($pcAnchor, $execStart)
if ($pcAt -lt 0) { throw "Could not find execI PC anchor in $path" }
$insertAt = $pcAt + $pcAnchor.Length
$text = $text.Insert($insertAt, "`r`n`tTraceProjectCaneRouletteRpl(pc);")

[IO.File]::WriteAllText($path, $text, [Text.UTF8Encoding]::new($false))
Write-Host "Installed roulette RPL tracer in $path"
