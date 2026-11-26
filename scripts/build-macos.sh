#!/usr/bin/env bash
# Builds a universal (arm64 + x86_64) MathiSynth VST3 + AU and installs them for the current user.
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -B build -G Xcode
cmake --build build --config Release --target MathiSynth_VST3 MathiSynth_AU MathiSynth_Standalone
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
rm -rf ~/Library/Audio/Plug-Ins/VST3/MathiSynth.vst3 ~/Library/Audio/Plug-Ins/Components/MathiSynth.component
cp -R build/MathiSynth_artefacts/Release/VST3/MathiSynth.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build/MathiSynth_artefacts/Release/AU/MathiSynth.component ~/Library/Audio/Plug-Ins/Components/
# Unsigned local builds: clear the quarantine flag so hosts will load them.
xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/MathiSynth.vst3 ~/Library/Audio/Plug-Ins/Components/MathiSynth.component 2>/dev/null || true
echo "Installed VST3 + AU to ~/Library/Audio/Plug-Ins"
