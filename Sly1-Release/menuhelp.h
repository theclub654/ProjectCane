#pragma once

// -----------------------------------------------------------------------------
// Info box text for menu options (shown next to the menu for the highlighted
// option). Only included by prompt.cpp, after resp.h. Plain ASCII; avoid '&'
// (rich-text markup) in these strings.
// -----------------------------------------------------------------------------

inline const char* PchzMenuStyleHelp(MSOPT opt)
{
	switch (opt)
	{
	case MSOPT_Theme: return "Preset color schemes for the menus. Changing any other Customization setting switches this to Custom.";
	case MSOPT_Font: return "Font used by the menus. Default is the game's normal menu font.";
	case MSOPT_TextSize: return "Size of the menu options.";
	case MSOPT_TitleSize: return "Size of the menu titles.";
	case MSOPT_TextR: return "Red amount of the menu text color (0-255).";
	case MSOPT_TextG: return "Green amount of the menu text color (0-255).";
	case MSOPT_TextB: return "Blue amount of the menu text color (0-255).";
	case MSOPT_TitleColor: return "Color of the menu titles.";
	case MSOPT_TextShadow: return "Draws a soft drop shadow behind menu text so it stays readable on bright scenes.";
	case MSOPT_BoxStyle: return "Classic uses the original game frame. Flat and Gradient draw a solid panel using the colors below.";
	case MSOPT_BoxR: return "Red amount of the menu background (Flat and Gradient styles).";
	case MSOPT_BoxG: return "Green amount of the menu background (Flat and Gradient styles).";
	case MSOPT_BoxB: return "Blue amount of the menu background (Flat and Gradient styles).";
	case MSOPT_BoxOpacity: return "How see-through the menu background is. 100 is fully solid.";
	case MSOPT_BoxRainbow: return "Slowly cycles the background through every color.";
	case MSOPT_BoxWidth: return "Makes the menu box wider or narrower.";
	case MSOPT_Border: return "Draws an outline around the menu box.";
	case MSOPT_BorderR: return "Red amount of the border color.";
	case MSOPT_BorderG: return "Green amount of the border color.";
	case MSOPT_BorderB: return "Blue amount of the border color.";
	case MSOPT_BorderOpacity: return "How see-through the border is.";
	case MSOPT_BorderThickness: return "Thickness of the border in pixels (at 720p; scales up with resolution).";
	case MSOPT_BorderRainbow: return "Slowly cycles the border through every color.";
	case MSOPT_BorderPulse: return "Makes the border gently fade in and out.";
	case MSOPT_HlR: return "Red amount of the highlighted option's color.";
	case MSOPT_HlG: return "Green amount of the highlighted option's color.";
	case MSOPT_HlB: return "Blue amount of the highlighted option's color.";
	case MSOPT_HlPulse: return "Makes the highlighted option shimmer like the original game.";
	case MSOPT_HlBounce: return "Makes the highlighted option grow and shrink slightly.";
	case MSOPT_HlCursor: return "Adds a marker next to the highlighted option.";
	case MSOPT_Divider: return "Shows the thin line between the menu title and its options.";
	case MSOPT_Position: return "Where the menu sits on screen.";
	case MSOPT_OpenAnim: return "Plays the zoom-in animation when a menu opens. Off makes menus appear instantly.";
	case MSOPT_ScreenDim: return "Darkens the game behind the menu so it is easier to read.";
	case MSOPT_Sounds: return "Plays the click sound when moving between options.";
	case MSOPT_Brightness: return "Brightens or darkens every menu color at once.";
	case MSOPT_Contrast: return "Pushes menu colors further apart (higher) or closer together (lower).";
	case MSOPT_Layout: return "Modern puts each setting's name on the left and its value on the right, with switches and sliders. Classic centers every row like the original game.";
	case MSOPT_PromptBar: return "Shows the buttons you can press along the bottom of the screen. The icons follow whatever you used last: controller, keyboard or mouse.";
	default: return nullptr;
	}
}

inline const char* PchzMouseOptHelp(MOUSEOPT opt)
{
	switch (opt)
	{
	case MOUSEOPT_Camera: return "Turn the camera with the mouse while playing. The cursor is hidden during gameplay and comes back whenever a menu is open.";
	case MOUSEOPT_Sensitivity: return "How fast the mouse turns the third-person camera. Separate from the controller's Camera Sensitivity. Select it to type an exact value or drag the slider.";
	case MOUSEOPT_AimSensitivity: return "How fast the mouse moves the Binoc-u-com and turrets. Slows down automatically while zoomed in.";
	case MOUSEOPT_InvertX: return "Reverses left / right mouse movement for the camera.";
	case MOUSEOPT_InvertY: return "Reverses up / down mouse movement when aiming the Binoc-u-com and turrets.";
	case MOUSEOPT_RawInput: return "Reads the mouse directly, skipping Windows pointer acceleration. Recommended on.";
	case MOUSEOPT_WheelZoom: return "The mouse wheel zooms the Binoc-u-com. Ignored if you bound the wheel to an action.";
	case MOUSEOPT_Menus: return "Hover to highlight, left click to select, right click to go back, wheel to change sliders.";
	default: return nullptr;
	}
}

inline const char* PchzRespkHelp(RESPK respk)
{
	if (respk >= RESPK_MsOpt0 && respk < RESPK_MsOpt0 + MSOPT_Max)
		return PchzMenuStyleHelp(static_cast<MSOPT>(respk - RESPK_MsOpt0));
	if (respk >= RESPK_MouseOpt0 && respk < RESPK_MouseOpt0 + MOUSEOPT_Max)
		return PchzMouseOptHelp(static_cast<MOUSEOPT>(respk - RESPK_MouseOpt0));

	switch (respk)
	{
	// Options
	case RESPK_Sound: return "Volume levels and speaker setup.";
	case RESPK_Vibration: return "Turns controller rumble on or off.";
	case RESPK_Video: return "Display, Graphics and Upscaling pages: window mode, frame rate, draw distance, DLSS, FSR and more.";
	case RESPK_GameplayMenu: return "Pause when unfocused, vibration, FPS counter, HUD style and size, and the speedrun timer.";
	case RESPK_Controls: return "Camera options, mouse settings and button mapping for keyboard and controller.";
	case RESPK_Mods: return "Save states, fast forward, the speedrun timer and their button combos.";
	case RESPK_MenuStyle: return "Change how the menus look: colors, fonts, sizes, borders and effects.";
	case RESPK_BackupsMenu: return "Save a copy of all your settings, or load a copy you saved earlier.";
	case RESPK_ExportNow: return "Saves all current settings to a dated folder in Settings Backups next to the game. Nothing changes in game.";
	case RESPK_ResetDefaults: return "Puts every setting back to how the game first started: video, sound, controls and bindings, DLSS / FSR, mods, customization and mouse. Your save files are not touched.";
	case RESPK_PauseOnFocusLoss: return "Pauses the game and mutes it when you click or Alt+Tab out of the window.";

	// Sound
	case RESPK_MusicVolume: return "Volume of the music.";
	case RESPK_SfxVolume: return "Volume of sound effects.";
	case RESPK_DialogueVolume: return "Volume of character voices.";
	case RESPK_Speakers: return "Stereo or mono sound.";

	// Video
	case RESPK_Dlss: return "NVIDIA DLSS (RTX cards). Renders at a lower resolution and rebuilds a sharp image with AI. DLAA keeps full resolution and only improves anti-aliasing.";
	case RESPK_DlssPreset: return "Which DLSS AI model to use. Newer models are sharper but slightly heavier on the GPU.";
	case RESPK_FrameGen: return "DLSS Frame Generation (RTX 40 and 50 series). Creates extra frames between real ones for much smoother motion. Works best with V-Sync off.";
	case RESPK_Fsr: return "AMD FSR upscaling. Works on any graphics card. Renders at a lower resolution and upscales for more FPS. Native AA keeps full resolution.";
	case RESPK_FsrSharpness: return "How much sharpening FSR applies after upscaling.";
	case RESPK_FpsCounter: return "Shows frames per second in the corner. Detailed adds a breakdown of where frame time goes.";
	case RESPK_Msaa: return "Classic anti-aliasing that smooths jagged edges. Turned off automatically while DLSS or FSR is on.";
	case RESPK_Anisotropy: return "Keeps textures sharp when viewed at an angle, like floors stretching into the distance. Very cheap; 16x recommended.";
	case RESPK_FrameRate: return "Maximum frames per second. Uncapped removes the limit (turn V-Sync off too). Locked to 59.94 while Retail Timing (Gameplay) is on.";
	case RESPK_RetailTiming: return "On: the game updates exactly like the PS2 (59.94 Hz), so physics, momentum and speedrun glitches behave like the console. Off: game speed follows Frame Rate, which can change how glitches play out.";
	case RESPK_CutsceneSkipControls: return "Optional binocucom dialog skipping: L1 skips one line, R1 skips the whole conversation. The vault-code call can't be skipped while this is on. Off keeps the original controls.";
	case RESPK_InternalResolution: return "Resolution the 3D scene is drawn at. Lower is faster, higher is sharper. Ignored while DLSS or FSR is on.";
	case RESPK_DrawDistance: return "How far away objects keep being drawn.";
	case RESPK_Fov: return "Widens the camera's view. Only changes what you see, not how the camera behaves.";
	case RESPK_WindowMode: return "Windowed, borderless or fullscreen.";
	case RESPK_Vsync: return "Syncs frames to your monitor to stop screen tearing. Adds a little input delay.";
	case RESPK_AspectRatio: return "Shape of the picture. Use your monitor's shape for widescreen.";
	case RESPK_Fog: return "Turns the game's distance fog on or off.";
	case RESPK_GuiStyle: return "Classic keeps the original HUD layout, Modern uses the updated one.";
	case RESPK_GuiScale: return "Size of the HUD and menus.";

	// Controls
	case RESPK_CameraSensitivity: return "How fast the right stick (and the arrow keys) turn the camera and aim. The mouse has its own sensitivity in Mouse Settings.";
	case RESPK_CameraInvert: return "Which way the camera turns when you push the right stick.";
	case RESPK_BinocInvert: return "Up / down direction when looking through the Binoc-u-com.";
	case RESPK_TurretInvert: return "Up / down direction when aiming turrets.";
	case RESPK_JetpackInvert: return "Up / down direction when flying.";
	case RESPK_DisableControllerWhenUnfocused: return "Ignores the controller while the game window is not in front.";
	case RESPK_ButtonIcons: return "Which button icons the menus show. Auto picks Xbox, PlayStation or Nintendo icons from the controller you are using.";
	case RESPK_ControllerMenu: return "Stick camera sensitivity, controller mapping and whether the controller works while the game is in the background.";
	case RESPK_MouseMenu: return "Mouse camera, sensitivity, invert and mouse menu controls.";
	case RESPK_KeyboardMapping: return "Rebind keys and mouse buttons. Any mouse button (including side buttons) and the wheel can be bound to an action.";
	case RESPK_KeyFfHold: return "Hold this to run the game faster (speed is set in Mods). Only works while Mods are on.";
	case RESPK_KeyFfToggle: return "Press to turn fast forward on or off.";
	case RESPK_KeyScreenshot: return "Saves a screenshot to the Screenshots folder next to the game. Hold Shift while pressing it to hide the HUD.";
	case RESPK_ControllerMapping: return "Rebind controller buttons.";

	// Mods
	case RESPK_ModEnable: return "Turns the mod hotkeys and button combos on or off.";
	case RESPK_ModFfSpeed: return "How fast the game runs while fast forward is active.";
	case RESPK_ModStatesMenu: return "Save your position and jump back to it instantly. Great for practicing tricky sections.";
	case RESPK_ModTimerMenu: return "An on-screen speedrun timer with level times and splits.";
	case RESPK_ModCombosMenu: return "Choose the controller combos for save states, fast forward and the timer.";
	case RESPK_ModLoadState: return "Return to the position saved in the current slot.";
	case RESPK_ModSaveState: return "Save your current position in the current slot.";
	case RESPK_ModSlot: return "Which save-state slot the save and load options use.";
	case RESPK_TimerEnable: return "Shows the speedrun timer on screen.";
	case RESPK_TimerMode: return "Real Time counts everything. Loads Removed pauses the timer during loading screens.";
	case RESPK_TimerLevel: return "Also shows the time spent in the current level.";
	case RESPK_TimerSplit: return "Shows how long the previous level took.";
	case RESPK_TimerMs: return "Shows milliseconds on the timer.";
	case RESPK_TimerPosition: return "Which corner of the screen the timer sits in.";
	case RESPK_TimerAutoStart: return "Resets and starts the timer automatically when you begin a new game.";
	case RESPK_TimerReset: return "Sets the timer back to zero.";

	// Customization
	case RESPK_MsTextMenu: return "Font, text size and text colors.";
	case RESPK_MsBoxMenu: return "Style, color and opacity of the menu background.";
	case RESPK_MsBorderMenu: return "Outline around the menu box.";
	case RESPK_MsHighlightMenu: return "How the selected option looks.";
	case RESPK_MsEffectsMenu: return "Menu position, animations, screen dimming, sounds, brightness and contrast.";
	case RESPK_MsReset: return "Puts every Customization setting back to the original game look.";

	default: return nullptr;
	}
}
