/*
 * Y2KBox Windows XP WGL / pixel-format diagnostic.
 *
 * Enumerates every pixel format exposed by the current display driver,
 * reproduces the game's id Tech 3 GLW_ChoosePFD selection algorithm,
 * compares it with Windows ChoosePixelFormat, and attempts WGL context
 * creation for selected and enumerated formats.
 *
 * The probe attempts SetPixelFormat + wglCreateContext + wglMakeCurrent for each
 * enumerated format on a fresh window/DC.
 *
 * Build with Visual C++:
 *   cl /nologo /W4 /O2 glprobe.c /link opengl32.lib gdi32.lib user32.lib
 *
 * Run from Command Prompt:
 *   glprobe.exe > glprobe.txt 2>&1
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <GL/gl.h>

static const char g_class_name[] = "Y2KBoxGLProbeWindow";

static LRESULT CALLBACK probe_wndproc(HWND hwnd, UINT message,
                                      WPARAM wparam, LPARAM lparam)
{
    (void)hwnd;
    (void)message;
    (void)wparam;
    (void)lparam;
    return DefWindowProcA(hwnd, message, wparam, lparam);
}

static int register_probe_class(void)
{
    WNDCLASSA wc;
    HINSTANCE instance = GetModuleHandleA(NULL);

    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = probe_wndproc;
    wc.hInstance = instance;
    wc.lpszClassName = g_class_name;

    if (RegisterClassA(&wc) != 0) {
        return 1;
    }

    return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

static HWND create_probe_window(void)
{
    return CreateWindowA(g_class_name, "Y2KBox GL probe",
                         WS_POPUP, 0, 0, 32, 32,
                         NULL, NULL, GetModuleHandleA(NULL), NULL);
}

static void print_flags(DWORD flags)
{
    printf(" flags=0x%08lX [", (unsigned long)flags);
    if (flags & PFD_DRAW_TO_WINDOW) printf(" DRAW_TO_WINDOW");
    if (flags & PFD_DRAW_TO_BITMAP) printf(" DRAW_TO_BITMAP");
    if (flags & PFD_SUPPORT_GDI) printf(" SUPPORT_GDI");
    if (flags & PFD_SUPPORT_OPENGL) printf(" SUPPORT_OPENGL");
    if (flags & PFD_GENERIC_FORMAT) printf(" GENERIC_FORMAT");
    if (flags & PFD_GENERIC_ACCELERATED) printf(" GENERIC_ACCELERATED");
    if (flags & PFD_DOUBLEBUFFER) printf(" DOUBLEBUFFER");
    if (flags & PFD_STEREO) printf(" STEREO");
    if (flags & PFD_SWAP_EXCHANGE) printf(" SWAP_EXCHANGE");
    if (flags & PFD_SWAP_COPY) printf(" SWAP_COPY");
    if (flags & PFD_NEED_PALETTE) printf(" NEED_PALETTE");
    if (flags & PFD_NEED_SYSTEM_PALETTE) printf(" NEED_SYSTEM_PALETTE");
    if (flags & PFD_DEPTH_DONTCARE) printf(" DEPTH_DONTCARE");
    if (flags & PFD_DOUBLEBUFFER_DONTCARE) printf(" DOUBLEBUFFER_DONTCARE");
    printf(" ]");

    if (flags & PFD_GENERIC_FORMAT) {
        if (flags & PFD_GENERIC_ACCELERATED) {
            printf(" [generic format, accelerated]");
        } else {
            printf(" [generic software format]");
        }
    } else if (flags & PFD_SUPPORT_OPENGL) {
        printf(" [non-generic OpenGL format; typically ICD-provided]");
    }
}

static void describe_format(HDC dc, int index, PIXELFORMATDESCRIPTOR *pfd)
{
    if (!DescribePixelFormat(dc, index, sizeof(*pfd), pfd)) {
        printf("PFD %d: DescribePixelFormat failed, GetLastError=%lu\n",
               index, (unsigned long)GetLastError());
        return;
    }

    printf("PFD %d: pixelType=%u layerType=%u version=%u",
           index, (unsigned int)pfd->iPixelType,
           (unsigned int)pfd->iLayerType, (unsigned int)pfd->nVersion);
    print_flags(pfd->dwFlags);
    printf("\n"
           "  color=%u (R%u@%u G%u@%u B%u@%u A%u@%u)"
           " depth=%u stencil=%u"
           " accum=%u (R%u G%u B%u A%u)"
           " aux=%u\n",
           (unsigned int)pfd->cColorBits,
           (unsigned int)pfd->cRedBits, (unsigned int)pfd->cRedShift,
           (unsigned int)pfd->cGreenBits, (unsigned int)pfd->cGreenShift,
           (unsigned int)pfd->cBlueBits, (unsigned int)pfd->cBlueShift,
           (unsigned int)pfd->cAlphaBits, (unsigned int)pfd->cAlphaShift,
           (unsigned int)pfd->cDepthBits, (unsigned int)pfd->cStencilBits,
           (unsigned int)pfd->cAccumBits,
           (unsigned int)pfd->cAccumRedBits,
           (unsigned int)pfd->cAccumGreenBits,
           (unsigned int)pfd->cAccumBlueBits,
           (unsigned int)pfd->cAccumAlphaBits,
           (unsigned int)pfd->cAuxBuffers);
}

static int try_format(int index, const char *label)
{
    HWND hwnd;
    HDC dc;
    HGLRC context;
    PIXELFORMATDESCRIPTOR pfd;
    int ok;

    printf("TRY %s: PFD %d\n", label, index);
    if (index <= 0) {
        printf("  no pixel format selected\n");
        return 0;
    }

    hwnd = create_probe_window();
    if (!hwnd) {
        printf("  CreateWindow failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        return 0;
    }

    dc = GetDC(hwnd);
    if (!dc) {
        printf("  GetDC failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        DestroyWindow(hwnd);
        return 0;
    }

    ZeroMemory(&pfd, sizeof(pfd));
    if (!DescribePixelFormat(dc, index, sizeof(pfd), &pfd)) {
        printf("  DescribePixelFormat failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 0;
    }

    SetLastError(ERROR_SUCCESS);
    ok = SetPixelFormat(dc, index, &pfd);
    printf("  SetPixelFormat=%d GetLastError=%lu\n",
           ok, (unsigned long)GetLastError());
    if (!ok) {
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 0;
    }

    SetLastError(ERROR_SUCCESS);
    context = wglCreateContext(dc);
    printf("  wglCreateContext=%s GetLastError=%lu\n",
           context ? "success" : "FAILED",
           (unsigned long)GetLastError());
    if (!context) {
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 0;
    }

    SetLastError(ERROR_SUCCESS);
    ok = wglMakeCurrent(dc, context);
    printf("  wglMakeCurrent=%d GetLastError=%lu\n",
           ok, (unsigned long)GetLastError());
    if (ok) {
        const GLubyte *vendor = glGetString(GL_VENDOR);
        const GLubyte *renderer = glGetString(GL_RENDERER);
        const GLubyte *version = glGetString(GL_VERSION);

        printf("  GL_VENDOR=%s\n", vendor ? (const char *)vendor : "(null)");
        printf("  GL_RENDERER=%s\n",
               renderer ? (const char *)renderer : "(null)");
        printf("  GL_VERSION=%s\n",
               version ? (const char *)version : "(null)");
        wglMakeCurrent(NULL, NULL);
    }

    wglDeleteContext(context);
    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);
    return ok != 0;
}


/*
 * Reproduce the Quake III / id Tech 3 GLW_ChoosePFD filtering and
 * best-match rules. The game enumerates PFDs itself; it does not use
 * ChoosePixelFormat for this selection.
 */
static int glw_choose_pfd(HDC dc, BYTE color_bits, BYTE depth_bits,
                          BYTE stencil_bits, int allow_software,
                          const char *label)
{
    PIXELFORMATDESCRIPTOR pfds[257];
    DWORD required_flags;
    int max_pfd;
    int index;
    int best = 0;

    ZeroMemory(pfds, sizeof(pfds));
    max_pfd = DescribePixelFormat(dc, 1, sizeof(pfds[0]), &pfds[0]);
    printf("\nGLW_ChoosePFD model %s: color=%u depth=%u stencil=%u,"
           " software_allowed=%d, reported_formats=%d\n",
           label, (unsigned int)color_bits, (unsigned int)depth_bits,
           (unsigned int)stencil_bits, allow_software, max_pfd);

    if (max_pfd <= 0) {
        printf("  DescribePixelFormat returned no formats\n");
        return 0;
    }
    if (max_pfd > 256) {
        printf("  limiting enumeration to 256 formats (id Tech 3 limit)\n");
        max_pfd = 256;
    }

    required_flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                     PFD_DOUBLEBUFFER;

    for (index = 1; index <= max_pfd; ++index) {
        PIXELFORMATDESCRIPTOR *candidate = &pfds[index];

        if (!DescribePixelFormat(dc, index, sizeof(*candidate), candidate)) {
            printf("  PFD %d rejected: DescribePixelFormat failed\n", index);
            continue;
        }

        if ((candidate->dwFlags & PFD_GENERIC_FORMAT) && !allow_software) {
            printf("  PFD %d rejected: PFD_GENERIC_FORMAT"
                   " (software OpenGL not allowed)\n", index);
            continue;
        }
        if (candidate->iPixelType != PFD_TYPE_RGBA) {
            printf("  PFD %d rejected: not RGBA (pixelType=%u)\n",
                   index, (unsigned int)candidate->iPixelType);
            continue;
        }
        if ((candidate->dwFlags & required_flags) != required_flags) {
            printf("  PFD %d rejected: required flags missing"
                   " (have=0x%08lX need=0x%08lX)\n",
                   index, (unsigned long)candidate->dwFlags,
                   (unsigned long)required_flags);
            continue;
        }
        if (candidate->cDepthBits < 15) {
            printf("  PFD %d rejected: depth bits %u < 15\n",
                   index, (unsigned int)candidate->cDepthBits);
            continue;
        }
        if (candidate->cStencilBits < 4 && stencil_bits > 0) {
            printf("  PFD %d rejected: stencil bits %u < 4"
                   " (requested %u)\n",
                   index, (unsigned int)candidate->cStencilBits,
                   (unsigned int)stencil_bits);
            continue;
        }

        printf("  PFD %d eligible: color=%u depth=%u stencil=%u\n",
               index, (unsigned int)candidate->cColorBits,
               (unsigned int)candidate->cDepthBits,
               (unsigned int)candidate->cStencilBits);

        if (best) {
            if (pfds[best].cColorBits != color_bits) {
                if (candidate->cColorBits == color_bits) {
                    best = index;
                    continue;
                } else if (candidate->cColorBits > pfds[best].cColorBits) {
                    best = index;
                    continue;
                }
            }

            if (pfds[best].cDepthBits != depth_bits) {
                if (candidate->cDepthBits == depth_bits) {
                    best = index;
                    continue;
                } else if (candidate->cDepthBits > pfds[best].cDepthBits) {
                    best = index;
                    continue;
                }
            }

            if (pfds[best].cStencilBits != stencil_bits) {
                if (candidate->cStencilBits == stencil_bits) {
                    best = index;
                    continue;
                } else if (candidate->cStencilBits > pfds[best].cStencilBits &&
                           stencil_bits > 0) {
                    best = index;
                    continue;
                }
            }
        } else {
            best = index;
        }
    }

    if (!best) {
        printf("  GLW_ChoosePFD result: FAILED (no eligible format)\n");
        return 0;
    }

    printf("  GLW_ChoosePFD result: PFD %d selected\n", best);
    describe_format(dc, best, &pfds[best]);
    if (pfds[best].dwFlags & PFD_GENERIC_FORMAT) {
        printf("  selected format is generic; game accepts it only when"
               " r_allowSoftwareGL is enabled\n");
    } else if (pfds[best].dwFlags & PFD_GENERIC_ACCELERATED) {
        printf("  selected format reports generic acceleration (MCD)\n");
    } else {
        printf("  selected format is non-generic (driver/hardware path)\n");
    }

    return best;
}

static int choose_and_try(HDC dc, BYTE stencil_bits, const char *label)
{
    PIXELFORMATDESCRIPTOR request;
    int index;

    ZeroMemory(&request, sizeof(request));
    request.nSize = sizeof(request);
    request.nVersion = 1;
    request.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                      PFD_DOUBLEBUFFER;
    request.iPixelType = PFD_TYPE_RGBA;
    request.cColorBits = 32;
    request.cDepthBits = 24;
    request.cStencilBits = stencil_bits;
    request.iLayerType = PFD_MAIN_PLANE;

    SetLastError(ERROR_SUCCESS);
    index = ChoosePixelFormat(dc, &request);
    printf("\nChoosePixelFormat %s (color=32, depth=24, stencil=%u)"
           " -> %d, GetLastError=%lu\n",
           label, (unsigned int)stencil_bits, index,
           (unsigned long)GetLastError());

    if (index > 0) {
        PIXELFORMATDESCRIPTOR actual;
        ZeroMemory(&actual, sizeof(actual));
        if (DescribePixelFormat(dc, index, sizeof(actual), &actual)) {
            printf("  selected descriptor:\n");
            describe_format(dc, index, &actual);
        }
    }

    return try_format(index, label);
}

int main(void)
{
    HWND hwnd;
    HDC dc;
    PIXELFORMATDESCRIPTOR pfd;
    int format_count;
    int index;
    int contexts_ok = 0;
    DWORD version;

    printf("Y2KBox WGL / pixel-format probe\n");
    version = GetVersion();
    printf("Windows version: %lu.%lu build %lu\n",
           (unsigned long)LOBYTE(LOWORD(version)),
           (unsigned long)HIBYTE(LOWORD(version)),
           (unsigned long)HIWORD(version));

    if (!register_probe_class()) {
        printf("RegisterClass failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        return 1;
    }

    hwnd = create_probe_window();
    if (!hwnd) {
        printf("CreateWindow failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        return 1;
    }

    dc = GetDC(hwnd);
    if (!dc) {
        printf("GetDC failed, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        DestroyWindow(hwnd);
        return 1;
    }

    format_count = DescribePixelFormat(dc, 1, sizeof(pfd), &pfd);
    printf("DescribePixelFormat reports %d formats\n", format_count);
    if (format_count <= 0) {
        printf("No pixel formats available, GetLastError=%lu\n",
               (unsigned long)GetLastError());
        ReleaseDC(hwnd, dc);
        DestroyWindow(hwnd);
        return 2;
    }

    printf("\n=== Complete pixel-format table ===\n");
    for (index = 1; index <= format_count; ++index) {
        describe_format(dc, index, &pfd);
    }

    printf("\n=== Reproduce COD1 / id Tech 3 GLW_ChoosePFD ===\n");
    index = glw_choose_pfd(dc, 32, 24, 8, 0,
                           "32/24/8, software disallowed");
    if (index > 0) {
        contexts_ok += try_format(index, "GLW strict 32/24/8");
    }
    index = glw_choose_pfd(dc, 32, 24, 0, 0,
                           "32/24/0, software disallowed");
    if (index > 0) {
        contexts_ok += try_format(index, "GLW strict 32/24/0");
    }

    printf("\n=== Diagnostic comparison: allow generic software formats ===\n");
    index = glw_choose_pfd(dc, 32, 24, 8, 1,
                           "32/24/8, software allowed");
    if (index > 0) {
        contexts_ok += try_format(index, "GLW software-allowed 32/24/8");
    }
    index = glw_choose_pfd(dc, 32, 24, 0, 1,
                           "32/24/0, software allowed");
    if (index > 0) {
        contexts_ok += try_format(index, "GLW software-allowed 32/24/0");
    }

    printf("\n=== Windows ChoosePixelFormat comparison (not used by COD1) ===\n");
    contexts_ok += choose_and_try(dc, 8, "Windows ChoosePixelFormat stencil=8");
    contexts_ok += choose_and_try(dc, 0, "Windows ChoosePixelFormat stencil=0");

    ReleaseDC(hwnd, dc);
    DestroyWindow(hwnd);

    printf("\n=== Try every enumerated pixel format ===\n");
    for (index = 1; index <= format_count; ++index) {
        contexts_ok += try_format(index, "enumerated");
    }

    printf("\nSummary: %d pixel-format context test(s) succeeded"
           " across %d enumerated formats.\n",
           contexts_ok, format_count);
    printf("A successful context is not proof of hardware acceleration;"
           " inspect the PFD generic flags and GL strings above.\n");

    return contexts_ok ? 0 : 2;
}
