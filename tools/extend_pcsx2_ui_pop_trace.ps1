$path = 'C:\Users\denar\source\repos\pcsx2-master\pcsx2\Interpreter.cpp'
$text = Get-Content -LiteralPath $path -Raw

$old = "`tconstexpr u32 STOP_VAG = 0x001be998;"
$new = "`tconstexpr u32 STOP_VAG = 0x001be998;`r`n`tconstexpr u32 POP_UI_ACTIVE_BLOT = 0x001e9570;"
if (-not $text.Contains($old)) {
	throw 'Could not find STOP_VAG constant.'
}
$text = $text.Replace($old, $new)

$marker = @'
	if (pc == STOP_VAG)
	{
'@
$insert = @'
	if (pc == POP_UI_ACTIVE_BLOT)
	{
		const u32 dialog = memRead32(G_BINOC + BINOC_PDIALOG_PLAYING);
		if (dialog != 0 && static_cast<s32>(memRead32(dialog + LO_OID)) == 828)
		{
			Console.WriteLn("[DOOR DIALOG ORIGINAL UI_POP] ra=%08X dialog=%08X state=%d ide=%d/%d flags=%u",
				cpuRegs.GPR.n.ra.UL[0], dialog,
				static_cast<s32>(memRead32(dialog + DIALOG_STATE)),
				static_cast<s32>(memRead32(dialog + DIALOG_IDE_CUR)),
				static_cast<s32>(memRead32(dialog + DIALOG_CDE)),
				memRead32(dialog + DIALOG_DP_FLAGS));
		}
		return;
	}

	if (pc == STOP_VAG)
	{
'@
if (-not $text.Contains($marker)) {
	throw 'Could not find STOP_VAG trace block.'
}
$text = $text.Replace($marker, $insert)

Set-Content -LiteralPath $path -Value $text -NoNewline
