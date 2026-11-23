# Windows XP WGL diagnostic

glprobe.c is a standalone diagnostic for investigating Windows XP OpenGL
initialization failures such as Call of Duty 1's GLW_ChoosePFD failure.

It does not modify the guest, install a driver, or change Y2KBox. It reports:

- Every pixel format returned by DescribePixelFormat, including flags,
  RGBA component sizes/shifts, depth, stencil, accumulation, auxiliary buffers,
  pixel type, and layer type.
- A reproduction of the Quake III / id Tech 3 GLW_ChoosePFD selection logic
  for 32-bit color / 24-bit depth requests with 8-bit and 0-bit stencil.
  It reports why each format is rejected or eligible, applies the original
  best-match ordering, and runs both with generic formats disallowed and
  allowed for comparison.
- The result of Windows ChoosePixelFormat for the same numeric requests as
  a separate comparison. COD1's GLW_ChoosePFD routine enumerates and filters
  PFDs itself; it does not use ChoosePixelFormat for that selection.
- Whether SetPixelFormat, wglCreateContext, and wglMakeCurrent succeed
  for selected formats and for every enumerated format.
- GL_VENDOR, GL_RENDERER, and GL_VERSION for every context that becomes
  current.

The strict GLW_ChoosePFD reproduction uses the game's required window,
OpenGL, and double-buffer flags; RGBA pixel type; minimum 15-bit depth;
minimum 4-bit stencil when stencil is requested; and the game's color/depth/
stencil best-match ordering. It models software OpenGL as disallowed in the
strict pass, then runs an allow-software pass to show whether generic formats
are the only available candidates. The game's runtime setting
r_allowSoftwareGL can affect that distinction.

## Quick COD1 software-rendering test

Before compiling the probe, you can test whether COD1 will accept a generic
software OpenGL format. Add this startup argument to the game's launch command:

```text
+set r_allowSoftwareGL 1
```

The id Tech 3 reference renderer defaults this cvar to 0. Setting it to 1
allows PFD_GENERIC_FORMAT candidates through pixel-format selection. If COD1
then creates a context, check its GL vendor/renderer/version output: this is a
software-compatibility test, not evidence of SVGA3D acceleration or OpenGL 2.1.
Remove the argument to return to the previous behavior.

## Build in Windows XP

Using Visual C++ with the Windows SDK:

```bat
cl /nologo /W4 /O2 glprobe.c /link opengl32.lib gdi32.lib user32.lib
```

Run from Command Prompt:

```bat
glprobe.exe > glprobe.txt 2>&1
```

Send back the complete glprobe.txt. The probe creates a fresh hidden
window/DC for each format because Windows permits SetPixelFormat only once
per window. It does not draw or change display resolution.

## Interpreting the output

- PFD_GENERIC_FORMAT identifies a generic OpenGL format.
- PFD_GENERIC_ACCELERATED indicates that a device driver accelerates the
  generic implementation. The id Tech 3 selection code rejects any
  PFD_GENERIC_FORMAT when software OpenGL is disallowed, even if the
  generic format is marked accelerated.
- A format with PFD_SUPPORT_OPENGL and neither generic flag is typically
  supplied by an Installable Client Driver (ICD), but the GL vendor/renderer
  strings are the better confirmation of the active implementation.
- SetPixelFormat succeeding does not guarantee context creation; the three
  WGL stages are reported separately.
- A successful software context can still be useful diagnostically. It does
  not demonstrate SVGA3D acceleration.

The program is intended for Windows XP and uses only the Win32/WGL APIs
available there. No third-party runtime or graphics SDK is required beyond
the compiler's Windows headers and import libraries.
