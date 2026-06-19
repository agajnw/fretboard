# Fretboard

Guitar fretboard practice app (JUCE, macOS + iOS).

## Setup

Clone with submodules:

```bash
git clone --recursive git@github.com-personal:agajnw/fretboard.git
```

If you already cloned without `--recursive`:

```bash
git submodule update --init --recursive
```

## Build

Open `fretboard/Fretboard.jucer` in Projucer (use the one from the submodule: `JUCE/extras/Projucer/`) and save to regenerate Xcode projects, then build:

- **macOS:** `fretboard/Builds/MacOSX/Fretboard.xcodeproj`
- **iOS:** `fretboard/Builds/iOS/Fretboard.xcodeproj`

JUCE is included as a git submodule at `JUCE/` (tracking `master`).

To update JUCE:

```bash
git submodule update --remote JUCE
```

Then re-save the `.jucer` file and rebuild.
