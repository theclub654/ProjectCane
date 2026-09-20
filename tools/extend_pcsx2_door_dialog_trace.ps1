$path = 'C:\Users\denar\source\repos\pcsx2-master\pcsx2\Interpreter.cpp'
$text = Get-Content -LiteralPath $path -Raw

$oldConstants = @'
	constexpr u32 DIALOG_KIND = 0x2d0;
	constexpr u32 DIALOG_STATE = 0x2d4;
'@
$newConstants = @'
	constexpr u32 DIALOG_KIND = 0x2d0;
	constexpr u32 DIALOG_STATE = 0x2d4;
	constexpr u32 DIALOG_CDE = 0x2dc;
	constexpr u32 DIALOG_IDE_CUR = 0x2e4;
	constexpr u32 DIALOG_DP_FLAGS = 0x2e8;
	constexpr u32 DIALOG_DP_DPK = 0x2ec;
	constexpr u32 DIALOG_DP_LIPSYNC = 0x2f8;
'@

if (-not $text.Contains($oldConstants)) {
	throw 'Could not find the dialog-offset block in Interpreter.cpp.'
}
$text = $text.Replace($oldConstants, $newConstants)

$oldTrack = @'
	static u32 tracked_dialog = 0;
	static s32 tracked_state = -1;
'@
$newTrack = @'
	static u32 tracked_dialog = 0;
	static s32 tracked_state = -1;
	static s32 tracked_ide = -1;
	static u32 tracked_flags = 0xffffffffu;
	static u32 tracked_lipsync = 0xffffffffu;
'@

if (-not $text.Contains($oldTrack)) {
	throw 'Could not find the tracked-dialog block in Interpreter.cpp.'
}
$text = $text.Replace($oldTrack, $newTrack)

$oldState = @'
		else if (state != tracked_state)
		{
			Console.WriteLn("[DOOR DIALOG ORIGINAL STATE_WRITE] afterPc=%08X dialog=%08X old=%d new=%d",
				pc, active_dialog, tracked_state, state);
			tracked_state = state;
		}
'@
$newState = @'
		const s32 ide = static_cast<s32>(memRead32(active_dialog + DIALOG_IDE_CUR));
		const u32 flags = memRead32(active_dialog + DIALOG_DP_FLAGS);
		const u32 lipsync = memRead32(active_dialog + DIALOG_DP_LIPSYNC);
		if (state != tracked_state || ide != tracked_ide || flags != tracked_flags || lipsync != tracked_lipsync)
		{
			Console.WriteLn("[DOOR DIALOG ORIGINAL PROGRESS] afterPc=%08X dialog=%08X state=%d ide=%d/%d flags=%u dpk=%d lipsync=%08X",
				pc, active_dialog, state, ide,
				static_cast<s32>(memRead32(active_dialog + DIALOG_CDE)), flags,
				static_cast<s32>(memRead32(active_dialog + DIALOG_DP_DPK)), lipsync);
			tracked_state = state;
			tracked_ide = ide;
			tracked_flags = flags;
			tracked_lipsync = lipsync;
		}
'@

if (-not $text.Contains($oldState)) {
	throw 'Could not find the state-change block in Interpreter.cpp.'
}
$text = $text.Replace($oldState, $newState)

Set-Content -LiteralPath $path -Value $text -NoNewline
