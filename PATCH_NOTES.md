# YOKUTON AI VOCAL — visual/audio patch

This package is a source-project update for the JUCE/CMake project, not a prebuilt VST3.
It includes an original pink/purple anime recording-studio background embedded into the plugin.

## Changes
- Adds `Resources/yokuton_anime_bg.png` and embeds it with `juce_add_binary_data`.
- Draws the anime artwork behind the editor controls with a dark translucent overlay.
- Sets a consistent rotary angle range for all six knobs.
- Fixes pitch-window autocorrelation indexing so the circular buffer is read in chronological order.
- Connects Sustain to pitch-tracker smoothing/stability.
- Removes the hidden preset pitch offset; pitch transposition is now controlled by the Pitch parameter.
- Presets alter the spectral-tilt colour only. This is still a lightweight tone-shaping effect, not a true formant-preserving vocal processor.

## Build
Use Windows 10/11 with Visual Studio 2022 C++ Desktop workload and CMake:
1. Replace the repository contents with this project (or copy the included files, preserving paths).
2. Run `build_windows.ps1` from the project root.
3. Find the VST3 in `build/YOKUTON_artefacts/Release/VST3/`.

This source package has not been compiled in this environment. Test it with GitHub Actions or Visual Studio before installing.
