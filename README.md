# Tape Saturation (VST3 / AU)

Analog tape saturation plugin built with [JUCE 8](https://juce.com) and CMake.

| Platform | Formats        | Arch              |
|----------|----------------|-------------------|
| Windows  | VST3           | x64               |
| macOS    | AU, VST3       | Universal (arm64 + x86_64) |

## Signal chain

```
input ─► dry tap (latency-compensated)
      └► wow/flutter (modulated delay)
         └► 4x oversampling [ pre-emphasis ► biased tanh ► de-emphasis ]
            └► DC block ► head bump ► tone ► hiss ► output ► dry/wet mix
```

## Parameters

| Parameter | Range            | Description |
|-----------|------------------|-------------|
| Drive     | 0 – 30 dB        | Level driven into the tape |
| Bias      | 0 – 100 %        | Asymmetry → even harmonics (warmth) |
| Tone      | 1 – 20 kHz       | High-frequency roll-off |
| Head Bump | 0 – 9 dB         | Low-end resonance around 60 Hz |
| Output    | -18 – +12 dB     | Output trim |
| Wow       | 0 – 100 %        | Slow pitch drift (~0.55 Hz) |
| Flutter   | 0 – 100 %        | Fast pitch wobble (6–12 Hz) |
| Hiss      | 0 – 100 %        | Tape noise (-90 … -45 dB) |
| Mix       | 0 – 100 %        | Dry / wet |

## Build with GitHub Actions (recommended)

1. Create a new GitHub repository and push the **contents of this folder** (the `.github` folder must be in the repo root).
2. Open the **Actions** tab – the *Build Tape Saturation* workflow runs automatically (or start it with *Run workflow*).
3. When finished, download the artefacts:
   * `TapeSaturation-Windows-VST3-x64` → contains `Tape Saturation.vst3`
   * `TapeSaturation-macOS-AU-VST3`    → contains `Tape Saturation.component` and `Tape Saturation.vst3`
4. Tag a commit with `v1.0.0` to also get a GitHub Release with both zips attached.

## Build locally

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

Artefacts end up in `build/TapeSaturation_artefacts/`.

## Install

* **Windows:** copy `Tape Saturation.vst3` to `C:\Program Files\Common Files\VST3\`
* **macOS AU:** copy `Tape Saturation.component` to `~/Library/Audio/Plug-Ins/Components/`
* **macOS VST3:** copy `Tape Saturation.vst3` to `~/Library/Audio/Plug-Ins/VST3/`

The macOS binaries are only ad-hoc signed. If Gatekeeper blocks them run:

```bash
xattr -cr "~/Library/Audio/Plug-Ins/Components/Tape Saturation.component"
```

For distribution to other users, sign with a Developer ID certificate and notarize.

## Customising

Change company / plugin codes / name in `CMakeLists.txt` (`COMPANY_NAME`, `PLUGIN_MANUFACTURER_CODE`, `PLUGIN_CODE`, `PRODUCT_NAME`).
The manufacturer code must contain at least one uppercase letter for AU, and both codes must be 4 characters.

## License

JUCE is licensed under AGPLv3 / commercial. Check <https://juce.com/legal/juce-8-licence/> before distributing binaries.
