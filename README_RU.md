# YOKUTON AI VOCAL — исходный проект

Это исходный проект VST3/Standalone-плагина на JUCE для Windows 10/11 и FL Studio 2025/2026.

## Что уже заложено
- VST3-эффект с параметрами, доступными для автоматизации в DAW.
- Четыре переключаемых характера: Kawaii, Hyperpop, Idol, Cyber Anime.
- Настройка тональности, лада, скорости коррекции, Amount, Pitch, Formant, Sustain и Mix.
- Сохранение состояния проекта/пресета через механизм JUCE.
- Розово-фиолетовый интерфейс с неоновым ядром и круговыми регуляторами.

## Важное ограничение первой версии
Это **рабочая стартовая DSP-реализация**, но пока не коммерческий вокальный процессор уровня Antares/Auto-Tune. Детектор использует автокорреляцию для одного ведущего голоса, а питч-сдвиг — облегчённый granular delay. При сильной коррекции могут появляться артефакты. Параметр Formant сейчас реализован как мягкая спектральная окраска, а не как полноценное независимое формантное преобразование. Sustain оставлен как параметр интерфейса/сессии для следующего DSP-этапа и пока не меняет огибающую.

Для точного «аниме-вокала» следующими этапами будут улучшение pitch tracker, качественный pitch shifter с сохранением формант и отдельный формантный процессор.

## Сборка на Windows
1. Установи Visual Studio 2022 с компонентом **Desktop development with C++**.
2. Установи CMake 3.22+ и Git.
3. Открой PowerShell в папке проекта.
4. Запусти `.\build_windows.ps1`.
5. После успешной сборки VST3 появится в `build\YOKUTON_artefacts\Release\VST3\YOKUTON AI VOCAL.vst3`.

Сборка скачивает JUCE 8.0.6 через CMake FetchContent, поэтому при первом запуске нужен интернет.

## Установка в FL Studio
Скопируй папку `YOKUTON AI VOCAL.vst3` в:
`C:\Program Files\Common Files\VST3\`

В FL Studio открой **Options → Manage plugins → Find installed plugins**. Затем добавь YOKUTON AI VOCAL как эффект на Mixer Insert.

## Скрипт сборки
`build_windows.ps1` создаёт Release-сборку VST3 и Standalone. Если сборка падает, сохрани полный текст ошибки из PowerShell — по нему можно исправить конкретную проблему.

## Лицензия и сторонние компоненты
JUCE подключается как отдельная зависимость с официального репозитория. Ознакомься с лицензией JUCE перед распространением бинарной версии.


# Cloud build (no local Visual Studio needed)

1. Create/sign in to a GitHub account.
2. Create a new repository (public or private) named `YOKUTON_AI_VOCAL`.
3. Upload the contents of this ZIP to the repository root (not the containing folder).
4. Open the repository's **Actions** tab. Select **Build YOKUTON VST3 (Windows)**.
5. Press **Run workflow** and wait for the Windows build to finish.
6. On the successful run, open the run page and download the artifact named
   `YOKUTON-AI-VOCAL-VST3-Windows`.
7. Extract the artifact. Copy the `.vst3` bundle to:
   `C:\\Program Files\\Common Files\\VST3\\`
8. In FL Studio, use **Options → Manage plugins → Find installed plugins**.

The workflow creates a Windows build using GitHub's Windows runner. It does not prove
the plugin has been audio-tested in FL Studio. The first build may fail due to source
compile issues; if it does, download/copy the Actions log and send it back for fixes.
