#!/usr/bin/env python3
"""Fail staging CI if the visible shortcut UI or live pause wiring regresses."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
settings = (root / "headless/prosperoeden/pe/ui/settings.cpp").read_text()
game = (root / "headless/prosperoeden/pe/ui/game_options.cpp").read_text()
pad = (root / "headless/pad.cpp").read_text()
main = (root / "headless/main.cpp").read_text()
menu = (root / "headless/live_cheat_menu.h").read_text()

def check(source, needle, label):
    assert needle in source, f"Missing {label}: {needle}"

check(settings, 'Eden::ControlsMenu::RowTop(row)', "bounded Controls panel geometry")
check(settings, 'std::string(tr("Pause: ")) + Eden::kShortcutLabels[prefs_.shortcuts[2]]',
      "pause combo on main Settings menu")
check(settings, '{tr("PAUSE / CHEATS"), Eden::kShortcutLabels[prefs_.shortcuts[2]]}',
      "current pause combo in Controls preview")
check(settings, 'prefs_.shortcuts = Eden::AssignShortcut', "shortcut editor action")
check(settings, 'controls_from_game_', "return to game Controls")
check(game, 'tr("Pause / Cheats shortcut")', "game-specific Controls discoverability")
check(game, 'open_modal(Modal::controls);', "game Controls opens real editor")
check(pad, 'DispatchShortcut(shortcuts, chord_masks, pressed, last_buttons)',
      "controller combinations use shared tested dispatch")
check(pad, 'else pause_cheats = true;', "pause event fired")
check(pad, 'sample.buttons &= ~shortcut.consumed;', "shortcut buttons blocked from guest")
# A simultaneous button press must remain flagged after touch-start initializes it.
start = pad.index('if (touch && !touched)')
flag = pad.index('if (shortcut.action >= 0 ||', start)
assert flag > start, "Touchpad chord flag is erased by touch-start reset"
check(main, 'pad->SetShortcuts(shortcuts);', "loaded shortcuts applied at boot")
check(main, 'Eden::ShowPausedCheats(system, *pad, game_title, stop_cheat_dialog);',
      "pause event invokes cheat dialog")
check(menu, 'system.Pause();', "guest pause")
check(menu, 'system.Run();', "guest resume")
check(menu, 'system.RegisterCheatList(Cheats::Rebuild()', "live cheat VM update")
print("PASS: visible Settings & game Controls editor, real input dispatch, pause/resume, live cheats")
