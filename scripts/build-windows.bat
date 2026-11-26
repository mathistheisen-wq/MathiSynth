@echo off
REM Builds MathiSynth.vst3 with Visual Studio 2022 and copies it into the system VST3 folder.
REM Run from "x64 Native Tools Command Prompt for VS 2022" (as Administrator to allow the copy).
cd /d "%~dp0\.."
cmake -B build -G "Visual Studio 17 2022" -A x64 || exit /b 1
cmake --build build --config Release --target MathiSynth_VST3 MathiSynth_Standalone || exit /b 1
xcopy /E /I /Y "build\MathiSynth_artefacts\Release\VST3\MathiSynth.vst3" "%CommonProgramFiles%\VST3\MathiSynth.vst3"
echo.
echo Installed to %CommonProgramFiles%\VST3\MathiSynth.vst3
