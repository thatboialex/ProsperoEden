#!/usr/bin/env python3
"""Fail staging CI if the visible shortcut UI or live pause wiring regresses."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
settings = (root / "headless/prosperoeden/pe/ui/settings.cpp").read_text()
game = (root / "headless/prosperoeden/pe/ui/game_options.cpp").read_text()
pad = (root / "headless/pad.cpp").read_text()
main = (root / "headless/main.cpp").read_text()
menu = (root / "headless/live_cheat_menu.h").read_text()
graphics = (root / "headless/live_graphics_menu.h").read_text()
chords = (root / "headless/shortcut_chords.h").read_text()

def check(source, needle, label):
    assert needle in source, f"Missing {label}: {needle}"

check(settings, 'Eden::ControlsMenu::RowTop(row)', "bounded Controls panel geometry")
check(settings, 'std::string(tr("Pause: ")) + Eden::kShortcutLabels[prefs_.shortcuts[2]]',
      "pause combo on main Settings menu")
check(settings, '{tr("PAUSE / CHEATS"), Eden::kShortcutLabels[prefs_.shortcuts[2]]}',
      "current pause combo in Controls preview")
check(settings, '{tr("LIVE GRAPHICS"), Eden::kShortcutLabels[prefs_.shortcuts[3]]}',
      "graphics action in Controls")
check(settings, 'TR("Live graphics")', "fourth selectable shortcut action")
check(chords, 'kShortcutActionCount', "all shortcut bindings have a shared count")
check(settings, 'prefs_.shortcuts = Eden::AssignShortcut', "shortcut editor action")
check(settings, 'controls_from_game_', "return to game Controls")
check(game, 'tr("Pause / Cheats shortcut")', "game-specific Controls discoverability")
check(game, 'tr("Live graphics shortcut")', "game-specific graphics control")
check(game, 'open_modal(Modal::controls);', "game Controls opens real editor")
check(pad, 'DispatchShortcut(shortcuts, chord_masks, pressed, last_buttons)',
      "controller combinations use shared tested dispatch")
check(pad, 'else if (shortcut.action == 2) pause_cheats = true;', "pause event fired")
check(pad, 'else if (shortcut.action == 3) graphics_menu = true;', "graphics menu event fired")
check(pad, 'sample.buttons &= ~shortcut.consumed;', "shortcut buttons blocked from guest")
# A simultaneous button press must remain flagged after touch-start initializes it.
start = pad.index('if (touch && !touched)')
flag = pad.index('if (shortcut.action >= 0 ||', start)
assert flag > start, "Touchpad chord flag is erased by touch-start reset"
check(main, 'pad->SetShortcuts(shortcuts);', "loaded shortcuts applied at boot")
check(main, 'Eden::ShowPausedCheats(system, *pad, game_title, stop_cheat_dialog);',
      "pause event invokes cheat dialog")
check(main, 'Eden::ShowPausedGraphics(system, *pad, backend, stop_cheat_dialog);',
      "graphics event invokes live settings selector")
check(graphics, 'Settings::values.scaling_filter.SetValue(LiveFilterValue(index));',
      "live Vulkan presentation filter mutation")
check(graphics, 'system.Pause();', "graphics menu pauses guest")
check(graphics, 'system.Run();', "graphics menu resumes guest")
check(menu, 'system.Pause();', "guest pause")
check(menu, 'system.Run();', "guest resume")
check(menu, 'system.RegisterCheatList(Cheats::Rebuild()', "live cheat VM update")
print("PASS: visible four-action shortcuts, controller dispatch, live Vulkan filters and live cheats")
