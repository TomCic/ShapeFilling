// Copyright (c) 2022 - 2023 Tomáš Cicvárek, CTU in Prague, FEE
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#ifndef __OPERATIONS
#define __OPERATIONS

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <assert.h>

#include <iostream>
#include <string>

#include "ColorMap.h"
#include "defines.h"

/// <summary>
/// This functon adds pixel colod on defined position.
/// </summary>
/// <param name="x"> x position</param>
/// <param name="y"> y position</param>
/// <param name="bitmap"> allegro image / bitmap</param>
/// <param name="color"> color</param>
void put_pixel(int x, int y, ALLEGRO_BITMAP*& bitmap, const RGB& color);

/// <summary>
/// This function draws a circle od defined radius around selected location.
/// </summary>
/// <param name="mouseX"> x position</param>
/// <param name="mouseY"> y position</param>
/// <param name="bitmap"> allegro image</param>
/// <param name="scribbles"> image containing scribbles</param>
/// <param name="radius"> circle radius</param>
/// <param name="color"> color</param>
/// <param name="idx"> scribble index</param>
void circleFillAllegro(const unsigned int mouseX, const unsigned int mouseY,
                       ALLEGRO_BITMAP*& bitmap, short*& scribbles,
                       const int radius, const RGB& color, short idx);

/// <summary>
/// Resets screen parameters and reloads the original bitmap.
/// </summary>
/// <param name="screen">Allegro bitmap</param>
void reset(ALLEGRO_BITMAP*& screen);

/// <summary>
/// Initializes the allegro variables.
/// </summary>
/// <param name="display"></param>
/// <param name="queue"></param>
/// <param name="font"></param>
/// <param name="hue"></param>
/// <param name="timer"></param>
/// <param name="screen"></param>
/// <param name="filename"> Image name</param>
/// <param name="name"> Image name without ext</param>
/// <returns>Scaled and padded original image</returns>
Image<float> firstInit(ALLEGRO_DISPLAY*& display, ALLEGRO_EVENT_QUEUE*& queue,
                       ALLEGRO_FONT*& font, ALLEGRO_BITMAP*& hue,
                       ALLEGRO_TIMER*& timer, ALLEGRO_BITMAP*& screen,
                       std::string& filename, std::string& name);

/// <summary>
/// Initializes the modal window.
/// </summary>
/// <param name="modal"> modal window</param>
void init_modal_window(ALLEGRO_BITMAP*& modal);

/// <summary>
/// Deallocates allegro variables.
/// </summary>
/// <param name="display"></param>
/// <param name="timer"></param>
/// <param name="screen"></param>
/// <param name="font"></param>
/// <param name="hue"></param>
void cleanup(ALLEGRO_DISPLAY*& display, ALLEGRO_TIMER*& timer,
             ALLEGRO_BITMAP*& screen, ALLEGRO_FONT*& font,
             ALLEGRO_BITMAP*& hue);

/// <summary>
/// Duplicates an allegro image.
/// </summary>
/// <param name="bitmap"> Bitmap to be duplicated</param>
/// <returns> Bitmap duplicate</returns>
BYTE* cloneImage(ALLEGRO_BITMAP* bitmap);

/// <summary>
/// Redraws the screen according the intensities and the color map.
/// </summary>
/// <param name="intensity"></param>
/// <param name="c_map"></param>
/// <param name="screen"></param>
/// <param name="scribbleData"> color</param>
/// <param name="block"> color</param>
/// <param name="flag"> display settings</param>
void set_screen(Image<float>& intensity, const ColorMap& c_map,
                ALLEGRO_BITMAP*& screen, short* scribbleData, BYTE* block,
                BYTE flag);

/// <summary>
/// Fills area in the screen with a color in a circle of area depending on its
/// radius
/// </summary>
/// <param name="X">X coordinate</param>
/// <param name="Y">Y coordinate</param>
/// <param name="bitmap">Allegro bitmap representing screen</param>
/// <param name="radius">Circle radius</param>
/// <param name="color">Scribble color</param>
void circleFill(const unsigned int X, const unsigned int Y,
                ALLEGRO_BITMAP*& bitmap, const int radius, const RGB& color);

// --------------------------------------------------------------------------
// ------------------------------ MODAL WINDOWS -----------------------------

/// <summary>
/// Draws to modal window
/// </summary>
/// <param name="font"> Font</param>
/// <param name="sliders">0-2 rgb, 3-5 hsl</param>
/// <param name="y"> Y coordinate</param>
/// <param name="ch"> Color component first letter</param>
void drawColorVal(ALLEGRO_FONT* font, float slider, int y, std::string ch);

/// <summary>
/// Redraw a line in the modal window
/// </summary>
/// <param name="Hue"> Hue bitmap</param>
/// <param name="font"> Font</param>
/// <param name="ch"> Component first letter</param>
/// <param name="val"> Component value</param>
/// <param name="slY"> Y cordinate of slider</param>
/// <param name="chY"> Y coordinate of text</param>
/// <param name="flag"> Component related flag</param>
void conditionalDraw(ALLEGRO_BITMAP* hue, ALLEGRO_FONT* font, std::string ch,
                     float val, int slY, int chY, BYTE flag);

/// <summary>
/// Draws to modal window
/// </summary>
/// <param name="modal"> Modal window</param>
/// <param name="hue"> Hue image</param>
/// <param name="font"> Font</param>
/// <param name="oldState"> Old color state</param>
/// <param name="newState"> New color state</param>
/// <returns></returns>
bool drawModal(ALLEGRO_BITMAP* modal, ALLEGRO_BITMAP* hue, ALLEGRO_FONT* font,
               ModalState& oldState, ModalState& newState);

/// <summary>
/// Chech whether point is in circle
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="r"></param>
/// <param name="w"></param>
/// <param name="h"></param>
/// <returns></returns>
bool inRadius(int x, int y, int r, int w, int h);

/// <summary>
/// Check whether mouse is out of the modal window
/// </summary>
/// <param name="xy"> Mouse location</param>
/// <returns></returns>
bool outOfModal(vec2<int>& xy);

/// <summary>
/// Mouse drag event initialization function.
/// </summary>
/// <param name="xy"> Mouse location</param>
/// <param name="ms"> Color state</param>
/// <returns></returns>
BYTE dragStartHandle(vec2<int>& xy, ModalState& ms);

/// <summary>
/// Mouse drag handler
/// </summary>
/// <param name="x"> x location of the mouse</param>
/// <param name="idx"> Index of color component</param>
/// <param name="ms"> Color state</param>
void dragSlider(int x, int idx, ModalState& ms);

/// <summary>
/// Color selecting event handler
/// </summary>
/// <param name="event"> Allegro event</param>
/// <param name="hue"> Hue bitmap</param>
/// <param name="modal"> Modal window</param>
/// <param name="font"> Font</param>
/// <param name="old"> Old color state</param>
/// <param name="mouse_bs"> Mouse state</param>
/// <param name="xy"> Mouse location</param>
/// <param name="drag"> Mouse drag switch</param>
void handleModal(ALLEGRO_EVENT& event, ALLEGRO_BITMAP* hue,
                 ALLEGRO_BITMAP* modal, ALLEGRO_FONT* font, ModalState& old,
                 BYTE& mouse_bs, vec2<int>& xy, BYTE& drag);

//----------------------------------------------------------------






#endif  // !__OPERATIONS
