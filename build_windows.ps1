$ErrorActionPreference = "Stop"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target YOKUTON_VST3 YOKUTON_Standalone
Write-Host ""
Write-Host "Build complete. Check build\YOKUTON_artefacts\Release"
