//-----------------------------------------------------------------
//                        ExactStep IAISS
//                             V0.5
//               github.com/ultraembedded/exactstep
//                     Copyright 2014-2019
//                    License: BSD 3-Clause
//-----------------------------------------------------------------
#include "display.h"

#ifdef INCLUDE_SCREEN
#include <SDL/SDL.h>
#endif

//-----------------------------------------------------------------
// Defines
//-----------------------------------------------------------------
// RGB565 pixel format -> components
#define RGB16_R(value) ((((value) >> 11) & 0x1F) << 3)
#define RGB16_G(value) ((((value) >> 5)  & 0x3F) << 2)
#define RGB16_B(value) ((((value) >> 0)  & 0x1F) << 3)

//-----------------------------------------------------------------
// init: Create SDL based screen buffer
//-----------------------------------------------------------------
bool display::init(int width, int height)
{
#ifdef INCLUDE_SCREEN
    m_width  = width;
    m_height = height;

    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE))
        return false;

    int flags = SDL_HWSURFACE | SDL_ASYNCBLIT | SDL_HWACCEL;
    SDL_Surface *screen = SDL_SetVideoMode(m_width, m_height, 0, flags);
    if (!screen || !screen->pixels)
        return false;

    m_screen = screen;
    SDL_WM_SetCaption("ExactStep", "ExactStep");

    // Disable cursor
    uint8_t data = 0;
    SDL_Cursor *cursor = SDL_CreateCursor(&data, &data, 8, 1, 0, 0);
    SDL_ShowCursor(1);
    SDL_SetCursor(cursor);
#endif

    return true;
}
//-----------------------------------------------------------------
// update: Redraw entire screen from frame buffer
//-----------------------------------------------------------------
bool display::update(uint8_t *memory)
{
#ifdef INCLUDE_SCREEN
    SDL_Surface *screen = (SDL_Surface *)m_screen;
    uint16_t    *frame  = (uint16_t *)memory;

    SDL_LockSurface(screen);
    for (int y=0;y<m_height;y++)
    {
        for (int x=0;x<m_width;x++)
        {
            uint16_t v = frame[(y*m_width)+x];
            uint32_t r = RGB16_R(v);
            uint32_t g = RGB16_G(v);
            uint32_t b = RGB16_B(v);

            ((uint32_t*)screen->pixels)[(y * (screen->pitch / 4)) + x] = 
            (
                r << screen->format->Rshift |
                g << screen->format->Gshift |
                b << screen->format->Bshift
            );
        }
    }
    SDL_UnlockSurface(screen);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
#endif

    return true;
}


