# Vacuum Robot Controller

Desktop application for monitoring and controlling a vacuum robot. Built with Qt 6 and C++17, it combines a multi-source video workspace with on-screen robot controls.

## Features

- **Video workspace** — View one or more video feeds in a tiled layout with per-source toolbars.
- **Multiple input sources**
  - Live camera capture
  - Local video files (MP4, MKV, AVI, MOV, WebM)
  - Network streams (HTTP and other Qt-supported URLs)
- **Robot controls** — Start, stop, forward, back, home, and a speed slider overlaid on the workspace.
- **Source management** — Add, remove, or stop all video sources from the menu bar.

On launch, the app automatically connects to the preferred system camera when one is available.

## Requirements

- CMake 3.16 or newer
- A C++17 compiler (GCC, Clang, or MSVC)
- Qt 6 with these modules:
  - Widgets
  - Multimedia
  - MultimediaWidgets

On Linux, install the build tools, Qt 6 development packages, and media runtime libraries. On Ubuntu/Debian:

```bash
# Build toolchain
sudo apt install build-essential cmake ninja-build pkg-config

# Qt 6 development (Widgets, Multimedia, MultimediaWidgets)
sudo apt install qt6-base-dev qt6-multimedia-dev

# Runtime support for camera capture and video playback
sudo apt install \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-libav \
  gstreamer1.0-gl \
  libv4l-0 \
  v4l-utils

# Wayland session support (skip on X11-only setups)
sudo apt install qt6-wayland
```

`qt6-multimedia-dev` pulls in `qt6-base-dev` as a dependency, but listing both makes the required modules explicit. The GStreamer and V4L packages are needed at runtime for live camera feeds and file/stream playback; without them the app may build but fail to open a camera or play video.

For additional codecs (e.g. H.264 in some formats), you may also need `libavcodec-extra` or the `ubuntu-restricted-extras` meta-package.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

To install:

```bash
cmake --install build
```

## Run

```bash
./build/vacuum-robot-controller
```

## Usage

### Video menu

| Action | Shortcut |
|--------|----------|
| Add Camera | Ctrl+Shift+C |
| Open File | Ctrl+O |
| Open Stream | Ctrl+Shift+O |
| Stop All | Ctrl+T |
| Quit | Ctrl+Q |

### Workspace controls

The floating toolbar on the right provides robot controls:

- **▶** Start
- **■** Stop (also stops all video sources)
- **▲ / ▼** Forward / Back
- **⌂** Home
- **Slider** Speed (0–100)

Control events are shown in the status bar. Robot hardware integration is handled separately from the UI layer.

## Project structure

```
vacuum-robot-controller/
├── CMakeLists.txt
├── src/
│   ├── main.cpp              # Application entry point
│   ├── MainWindow.*          # Menus, source management, status bar
│   ├── WorkspaceView.*       # Video layout and robot control toolbar
│   ├── AbstractVideo.*       # Base class for video sources
│   ├── CameraVideo.*         # Live camera input
│   ├── MediaVideo.*          # File and stream playback
│   ├── VideoPlayer.*         # Graphics-scene video widget
│   └── VideoToolbar.*        # Floating overlay toolbar helper
└── build/                    # CMake build output (ignored by git)
```

## Version

0.1.0
