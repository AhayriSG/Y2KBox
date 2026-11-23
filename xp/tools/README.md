# Windows XP WGL diagnostic

glprobe.c is a standalone diagnostic for investigating Windows XP OpenGL
initialization failures such as Call of Duty 1's GLW_ChoosePFD failure.

It does not modify the guest, install a driver, or change Y2KBox. It reports:

- Every pixel format returned by DescribePixelFormat, including flags,
  RGBA component sizes/shifts, depth, stencil, accumulation, auxiliary buffers,
  pixel type, and layer type.
- The selected formats for 32-bit color / 24-bit depth requests with 8-bit and
  0-bit stencil, matching the two requests in the supplied COD1 log.
- Whether SetPixelFormat, wglCreateContext, and wglMakeCurrent succeed
  for those selected formats and for every enumerated format.
- GL_VENDOR, GL_RENDERER, and GL_VERSION for every context that becomes
  current.

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
- PFD_GENERIC_ACCELERATED indicates generic-format acceleration.
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
