# sokol-samples

Sample code for https://github.com/floooh/sokol

asm.js/wasm live demos: https://floooh.github.io/sokol-html5/index.html

Minimal 8-bit emulators using the sokol headers: https://floooh.github.io/tiny8bit

*Work In Progress*

### Build Status:

|Platform|Build Status|
|--------|------|
|GH Actions (OSX/Linux/Win+VS2019/iOS/WASM)|[![Build Status](https://github.com/floooh/sokol-samples/workflows/build_and_test/badge.svg)](https://github.com/floooh/sokol-samples/actions)|
|Travis: (OSX/Linux/Win,VS2017)|[![Build Status](https://travis-ci.org/floooh/sokol-samples.svg?branch=master)](https://travis-ci.org/floooh/sokol-samples)|
|AppVeyor (Windows,VS2015)|[![Build status](https://ci.appveyor.com/api/projects/status/3jxh6gi272i5jd84/branch/master?svg=true)](https://ci.appveyor.com/project/floooh/sokol-samples/branch/master)|

## What's New:

- **26-Aug-2019**: New sample: [fontstash-sapp](https://floooh.github.io/sokol-html5/fontstash-sapp.html)

- **06-Jul-2019**: Two new samples for the new [sokol_fetch.h header](https://github.com/floooh/sokol/blob/master/sokol_fetch.h):
    - [loadpng-sapp](https://floooh.github.io/sokol-html5/loadpng-sapp.html): load an image file into a sokol-gfx texture
    - [plmpeg-sapp](https://floooh.github.io/sokol-html5/plmpeg-sapp.html): MPEG1 streaming via [pl_mpeg](https://github.com/phoboslab/pl_mpeg)

- **04-Jun-2019**: New sample on how to compile and use the sokol headers as
DLL (currently only on Windows). This demonstrates the new SOKOL_DLL
configuration define which annotates public function declarations with
__declspec(dllexport) or __declspec(dllimport). [See
here](https://github.com/floooh/sokol-samples/tree/master/libs/sokol) for the
DLL, [and here](https://github.com/floooh/sokol-samples/blob/master/sapp/noentry-dll-sapp.c)
for the example code using the DLL.

- **15-May-2019**: the sokol-app samples in the ```sapp``` directory have
been "ported" to the new shader-cross-compiler solution ([see here for
details](https://github.com/floooh/sokol-tools/blob/master/docs/sokol-shdc.md)).
Shaders are written as 'annotated GLSL', and cross-compiled to various
GLSL dialects, HLSL and MSL through a custom-build job which invokes the
```sokol-shdc``` command line tool.

- **01-Apr-2019**: sample code for the new sokol_gl.h header:
    - [sapp/sgl-sapp.c](https://github.com/floooh/sokol-samples/blob/master/sapp/sgl-sapp.c): triangles, quads, texturing and the matrix stack
    - [sapp/sgl-lines-sapp.c](https://github.com/floooh/sokol-samples/blob/master/sapp/sgl-lines-sapp.c): lines and line strips
    - [sapp/sgl-microui-sapp.c](https://github.com/floooh/sokol-samples/blob/master/sapp/sgl-microui-sapp.c): example [microui](https://github.com/rxi/microui) integration
    - [glfw/sgl-test-glfw.c](https://github.com/floooh/sokol-samples/blob/master/glfw/sgl-test-glfw.c): a pure GLFW/OpenGL 1.2 program to check whether sokol_gl.h behaves the same as OpenGL 1.2

- **05-Mar-2019**: the sokol-app samples (in the sapp directory) now come with optional
debugging UIs implemented via the new Dear ImGui based debug-inspection headers, these
are compiled as separate executables, so the executable-versions without UI are still as
small as possible.

- **19-Feb-2019**: a new sokol_app.h sample has been added to demonstrate the
new SOKOL_NO_ENTRY feature (in which sokol_app.h doesn't hijack the main function):
```sapp/noentry-sapp.c```

- **26-Jan-2019**: The sokol_app.h samples now also work on Android. See below for build instructions.

- **12-Apr-2018**: New samples have been added to demonstrate the new optional vertex-buffer-
and index-buffer-offsets in the sg\_draw\_state struct. Also the location of fips build-system
files have changed, please update fips with a 'git pull' from the fips directory.

- **27-Mar-2018**: The Dear Imgui fips wrapper has recently been moved to a new repository at
https://github.com/fips-libs/fips-imgui and updated to the latest ImGui version which
required some code changes. If you already had checked out sokol-samples, perform the following
steps to udpate:
    1. delete the fips-imgui directory
    2. in the sokol-samples directory, run **./fips fetch**

## How to build

Make sure that the following tools are in the path. Exact versions shouldn't
matter:
```
> python --version
Python 2.7.10
> cmake --version
cmake version 3.8.2
# make is only needed for building through emscripten
> make --version
GNU Make 3.81
# on OSX (on Windows you just need a recent VS)
> xcodebuild -version
Xcode 9.0
```

### Building the platform-specific samples

There are two types of samples, platform-specific samples in the
folders ```d3d11```, ```glfw```, ```html5``` and ```metal```, and
platform-agnostic samples using the ```sokol_app.h``` application-wrapper
header in the folder ```sapp```.

### To build the GLFW samples on Linux, MacOS and Windows:

```
> mkdir ~/scratch
> cd ~/scratch
> git clone git@github.com:floooh/sokol-samples
> cd sokol-samples
> ./fips build
...
> ./fips list targets
...
> ./fips run triangle-glfw
...
```

On Linux you'll need to install a couple of development packages for
GLFW: http://www.glfw.org/docs/latest/compile.html#compile_deps_x11

### To build for Metal on OSX:

```
> cd ~/scratch/sokol-samples
> ./fips set config metal-osx-xcode-debug
> ./fips build
...
> ./fips list targets
...
> ./fips run triangle-metal
```

### To build for Metal on iOS:

```
> cd ~/scratch/sokol-samples
> ./fips set config metal-ios-xcode-debug
> ./fips set iosteam [YOUR-TEAM-ID]
> ./fips gen
> ./fips open
# Xcode should open now, where you can build and run the iOS code as usual
```
The \[YOUR-TEAM-ID\] must be replaced with your Apple Developer Team ID, this
is a 10-character string which you can look up on
https://developer.apple.com/account/#/membership. If you get build errors
about 32-bit targets, exit Xcode, run ```./fips clean```, ```./fips gen```
and ```./fips open``` again. This is a known but unsolved issue which I need
to investigate.

Another known issue: The arraytex-metal sample currently has a weird rendering artefact at least on my iPad Mini4 which looks like Z-fighting.

### To build for D3D11 on Windows:

```
> cd /scratch/sokol-samples
> fips set config d3d11-win64-vstudio-debug
> fips build
...
> fips list targets
...
> fips run triangle-d3d11
```

### To build for emscripten:

```
> cd ~/scratch/sokol-samples
> ./fips setup emscripten
[...this will take a while]
> ./fips set config webgl2-emsc-make-release
> ./fips build
...
> ./fips list targets
...
> ./fips run triangle-emsc
...
```

### To build for Android:

Plug an Android device into your computer, and then:

```
> cd ~/scratch/sokol-samples
> ./fips setup android
[...this will install a local Android SDK/NDK under ../fips-sdks/android]
> ./fips set config sapp-android-make-debug
> ./fips build
...
> ./fips list targets
...
> ./fips run triangle-sapp
...
```
The last command should install and run the ```triangle-sapp``` sample on the
connected Android device.

To debug Android applications I recommend using Android Studio with
"Profile or debug APK". You can find the compiled APK files under
```../fips-deploy/[project]/[config]```.

## To build the platform-agnostic sokol_app.h samples:

Building the sokol_app.h samples is currently supported for MacOS, Windows,
Linux, iOS, HTML5 and Android (RaspberryPi is planned).

Use any of the following custom build configs starting with ```sapp-```
which matches your platform and build system:

```bash
> ./fips list configs | grep sapp-
  sapp-android-make-debug
  ...
  sapp-d3d11-win64-vs2017-debug
  sapp-d3d11-win64-vs2017-release
  sapp-d3d11-win64-vscode-debug
  sapp-d3d11-win64-vstudio-debug
  sapp-d3d11-win64-vstudio-release
  sapp-ios-xcode-debug
  ...
  sapp-win64-vstudio-debug
  sapp-win64-vstudio-release
> ./fips set config sapp-...
> ./fips build
> ./fips list targets
> ./fips run cube-sapp
```

Note the following caveats:
- for HTML5, first install the emscripten SDK as described above in the
native HTML5 sample section
- for iOS, set the developer team id, as described above in the iOS section
- OpenGL is currently not supported on MacOS because NSOpenGLView and
friends are broken on the MacOS Mojave beta, instead use the
```sapp-metal-*``` build configs (GLES on iOS is supported though)

## Thanks to:

- GLFW: https://github.com/glfw/glfw
- flextGL: https://github.com/ginkgo/flextGL
- Handmade-Math: https://github.com/StrangeZak/Handmade-Math
- Dear Imgui: https://github.com/ocornut/imgui
- cimgui: https://github.com/cimgui/cimgui
- utest.h: https://github.com/sheredom/utest.h

Enjoy!
