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

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <filesystem>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include "AllegroOperations.h"
#define ALLEGRO_UNSTABLE

#include "Utils.h"

void put_pixel(int x, int y, ALLEGRO_BITMAP*& bitmap, const RGB& color) {
  if (x >= 0 && x < al_get_bitmap_width(bitmap) && y >= 0 &&
      y < al_get_bitmap_height(bitmap)) {
    ALLEGRO_COLOR col = al_get_pixel(bitmap, x, y);
    al_put_pixel(
        x, y, al_map_rgb_f(color.r * col.r, color.g * col.g, color.b * col.b));
  }
}

void circleFillAllegro(const unsigned int mouseX, const unsigned int mouseY,
                       ALLEGRO_BITMAP*& bitmap, short*& scribbles,
                       const int radius, const RGB& color, short idx) {
  al_set_target_bitmap(bitmap);
  al_lock_bitmap(bitmap, al_get_bitmap_format(bitmap), ALLEGRO_LOCK_READWRITE);
  al_set_clipping_rectangle(mouseX - RADIUS, mouseY - RADIUS, RADIUS * 2,
                            RADIUS * 2);
  int r_2 = radius * radius;
  for (int dy = -radius; dy <= radius; dy += 1) {
    for (int dx = -radius; dx <= radius; dx += 1) {
      if (dx * dx + dy * dy <= r_2 &&
          dx + mouseX < al_get_bitmap_width(bitmap) && dx + mouseX >= 0 &&
          dy + mouseY < al_get_bitmap_height(bitmap) && dy + mouseY >= 0) {
        put_pixel(dx + mouseX, dy + mouseY, bitmap, color);
        scribbles[dx + mouseX + al_get_bitmap_width(bitmap) * (dy + mouseY)] =
            idx;
      }
    }
  }
  al_unlock_bitmap(bitmap);
}

void reset(ALLEGRO_BITMAP*& screen) {
  // unlock locked bitmap
  if (screen != nullptr && al_is_bitmap_locked(screen)) {
    try {
      al_unlock_bitmap(screen);
    } catch (const std::exception& e) {
      std::cout << "ERROR: Allegro bitmap unlock. Exception: " << e.what()
                << std::endl;
    }
  }
  if (screen == nullptr) {
    screen = al_create_bitmap(MM_WIDTH, MM_HEIGHT);
  }
  al_set_target_bitmap(screen);
  al_reset_clipping_rectangle();
  al_clear_to_color(al_map_rgb(255, 255, 255));
  al_draw_bitmap(screen, 0, 0, 0);
}

Image<float> firstInit(ALLEGRO_DISPLAY*& display, ALLEGRO_EVENT_QUEUE*& queue,
                       ALLEGRO_FONT*& font, ALLEGRO_BITMAP*& hue,
                       ALLEGRO_TIMER*& timer, ALLEGRO_BITMAP*& screen,
                       std::string& filename, std::string& name) {
  // find folders for saves and pictures and create them, if they do not exist
  if (!std::filesystem::is_directory("saves") ||
      !std::filesystem::exists("saves")) {
    std::filesystem::create_directory("saves");
  }
  if (!std::filesystem::is_directory("pictures") ||
      !std::filesystem::exists("pictures")) {
    std::filesystem::create_directory("pictures");
  }

  std::cout
      << "Type the image name. The image must be in the ./pictures folder:\n";
  if (!(std::cin >> filename)) {
    std::cerr << "ERROR: Invalid name." << std::endl;
    std::getchar();  // Empty stdin
    std::getchar();
    exit(1);
  }

  // parse names
  std::string delim = ".";
  size_t pos = filename.rfind(delim);
  if (pos == 0) {
    std::cout << "ERROR: Invalid name" << std::endl;
    std::getchar();  // empty stdin
    std::getchar();
    exit(1);
  }
  name = filename.substr(0, pos);
  std::string ext = filename.substr(pos + 1, filename.size() - pos - 1);
  for (int i = 0; i < ext.size(); i++) ext[i] = std::tolower(ext[i]);
  if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
    std::cout
        << "ERROR: Wrong format. Supported formats are png, jpg, bmp and tga.\n\
See the stbi library reference at https://www.cs.unh.edu/~cs770/lwjgl-javadoc/lwjgl-stb/org/lwjgl/stb/STBImage.html";
    std::getchar();  // empty stdin
    std::getchar();
    exit(1);
  }

  // initialize allegro addons
  al_init();
  if (!al_init_image_addon() || !al_init_primitives_addon() ||
      !al_init_font_addon() || !al_init_ttf_addon()) {
    std::cout << "ERROR: Allegro addons not loaded!" << std::endl;
    std::getchar();
    std::getchar();
    exit(1);
  }

  // read the image
  Image<float> img = imread<float>(FOLDER + filename);
  if (img.width() == 0 || img.height() == 0) {
    std::cout << "ERROR: Image not loaded.\n\
For supported formats see the stbi library reference at https://www.cs.unh.edu/~cs770/lwjgl-javadoc/lwjgl-stb/org/lwjgl/stb/STBImage.html";
    std::getchar();  // empty stdin
    std::getchar();
    exit(1);
  }

  // install and loar resources
  al_install_keyboard();
  al_install_mouse();
  Utils::scaleAndPad(img);
  display = al_create_display(img.width(), img.height());
  if (!display) {
    std::cout << "ERROR: Display not created!" << std::endl;
    std::getchar();
    std::getchar();
    exit(1);
  }
  queue = al_create_event_queue();
  timer = al_create_timer(1.0f / 60.0f);
  font = al_load_font(FONT_LOC, FONT_S, 0);
  hue = al_load_bitmap(HUE_LOC);
  if (!queue || !timer) {
    std::cout << "ERROR: Resources not created!" << std::endl;
    std::getchar();
    std::getchar();
    exit(1);
  }

  // register events
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_mouse_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer));

  reset(screen);

  return img;
}

void init_modal_window(ALLEGRO_BITMAP*& modal) {
  modal = al_create_bitmap(MW_W, MW_H);
  if (modal == nullptr) return;
  al_set_target_bitmap(modal);
  al_clear_to_color(al_map_rgb(200, 200, 200));
  al_draw_rectangle(0, 0, MW_W, MW_H, al_map_rgb(0, 0, 0), 3);
}

void cleanup(ALLEGRO_DISPLAY*& display, ALLEGRO_TIMER*& timer,
             ALLEGRO_BITMAP*& screen, ALLEGRO_FONT*& font,
             ALLEGRO_BITMAP*& hue) {
  if (hue) al_destroy_bitmap(hue);
  al_uninstall_keyboard();
  al_uninstall_mouse();
  al_destroy_timer(timer);
  if (screen) al_destroy_bitmap(screen);
  al_destroy_display(display);
  if (font) al_destroy_font(font);
}

BYTE* cloneImage(ALLEGRO_BITMAP* bitmap) {
  al_set_target_bitmap(bitmap);
  al_lock_bitmap(bitmap, al_get_bitmap_format(bitmap), ALLEGRO_LOCK_READWRITE);
  std::cout << "cloning start" << std::endl;
  int height = al_get_bitmap_height(bitmap);
  int width = al_get_bitmap_width(bitmap);
  int idx;
  BYTE* image = new BYTE[(size_t)width * (size_t)height * (size_t)CHANNELS];
  for (int h = 0; h < height; h++) {
    for (int w = 0; w < width; w++) {
      ALLEGRO_COLOR col = al_get_pixel(bitmap, w, h);
      idx = w * CHANNELS + width * h * CHANNELS;

      image[idx] = (BYTE)(col.r * 255);
      image[idx + 1] = (BYTE)(col.g * 255);
      image[idx + 2] = (BYTE)(col.b * 255);
    }
  }
  std::cout << "cloning end" << std::endl;
  al_unlock_bitmap(bitmap);
  return image;
}

void set_screen(Image<float>& intensity, const ColorMap& c_map,
                ALLEGRO_BITMAP*& screen, short* scribbleData, BYTE* block,
                BYTE flag) {
  al_set_target_bitmap(screen);
  al_lock_bitmap(screen, al_get_bitmap_format(screen), ALLEGRO_LOCK_READWRITE);
  al_reset_clipping_rectangle();
  for (int h = 0; h < intensity.height(); h += 1) {
    for (int w = 0; w < intensity.width(); w += 1) {
      float mul = flag != 2 ? intensity(w, h) : 1.0f;  // intensity in pixel
      RGB col = c_map.getColorAt(w, h);                   // color of segment
      // -------------------Overlap applied--------------
      if (flag & 1) {
        // case when scribble is at the pixel location
        if (flag == 1 && scribbleData[w + h * intensity.width()] >= 0) {
          if (scribbleData[w + h * intensity.width()] == c_map.getMaskAt(w, h))
            mul /= 2;
          col = c_map.getColors()[scribbleData[w + h * intensity.width()]];
          if (col == RGB(1, 1, 1)) {
            vec2<int> dirs[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
            mul = 1;
            for (int i = 0; i < ALL; i++) {
              vec2<int> coord = dirs[i] + vec2<int>(w, h);
              if (coord.x < 0 || coord.y < 0 || coord.x >= c_map.getWidth() ||
                  coord.y >= c_map.getHeight())
                continue;
              if (scribbleData[coord.x + coord.y * intensity.width()] !=
                  scribbleData[w + h * intensity.width()]) {
                col = RGB(0, 0, 0);
              }
            }
          }
        }
        if (flag == 3) {
          col = RGB(1, 1, 1);
          if (scribbleData[w + h * intensity.width()] != -1) {
            col = c_map.getColors()[scribbleData[w + h * intensity.width()]];
            mul = 1;

            vec2<int> dirs[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
            for (int i = 0; i < ALL; i++) {
              vec2<int> coord = dirs[i] + vec2<int>(w, h);
              if (coord.x < 0 || coord.y < 0 || coord.x >= c_map.getWidth() ||
                  coord.y >= c_map.getHeight())
                continue;
              if (scribbleData[coord.x + coord.y * intensity.width()] !=
                  scribbleData[w + h * intensity.width()]) {
                col = (col.r + col.g + col.b) / 3.0f < 0.5f ? RGB(1, 1, 1)
                                                            : RGB(0, 0, 0);
                break;
              }
            }
          }
        }
        // draw blocked area
        if (block[w + h * intensity.width()] == 1) {
          col = {col.r * 0.7f, col.g * 0.2f, col.b * 0.2f};
        }
        if (block[w + h * intensity.width()] == 2) {
          col = {col.r * 0.2f, col.g * 0.7f, col.b * 0.2f};
        }
      }
      // ------------------------------------------------
      RGB color = mul * col;
      al_put_pixel(w, h, al_map_rgb_f(color.r, color.g, color.b));
    }
  }
  al_unlock_bitmap(screen);
}

void circleFill(const unsigned int X, const unsigned int Y,
                ALLEGRO_BITMAP*& bitmap, const int radius, const RGB& color) {
  al_set_target_bitmap(bitmap);
  al_lock_bitmap(bitmap, al_get_bitmap_format(bitmap), ALLEGRO_LOCK_READWRITE);
  al_set_clipping_rectangle(X - radius, Y - radius, radius * 2, radius * 2);
  int r_2 = radius * radius;
  for (int dy = -radius; dy <= radius; dy += 1) {
    for (int dx = -radius; dx <= radius; dx += 1) {
      if (dx * dx + dy * dy <= r_2) {
        put_pixel(dx + X, dy + Y, bitmap, color);
      }
    }
  }
  al_unlock_bitmap(bitmap);
}

void draw_line(ALLEGRO_BITMAP*& screen, int x1, int y1, int x2, int y2) {
  al_set_target_bitmap(screen);
  al_draw_line(
      x1, y1, x2, y2,
      al_map_rgb(ARROW_COLOR_R * 255, ARROW_COLOR_G * 255, ARROW_COLOR_B * 255),
      ARROW_THICKNESS);
  circleFill(x2, y2, screen, ARROW_THICKNESS,
             {ARROW_COLOR_R, ARROW_COLOR_G, ARROW_COLOR_B});
}



// --------------------------------------------------------------------------
// ------------------------------ MODAL WINDOWS -----------------------------

/// <summary>
/// Draws to modal window
/// </summary>
/// <param name="font"> Font</param>
/// <param name="sliders">0-2 rgb, 3-5 hsl</param>
/// <param name="y"> Y coordinate</param>
/// <param name="ch"> Color component first letter</param>
void drawColorVal(ALLEGRO_FONT* font, float slider, int y, std::string ch) {
  if (font == nullptr) return;
  std::stringstream stream;
  if (slider == (int)slider && ch != "S")
    stream << std::setw(4) << (int)slider;
  else
    stream << std::setw(4) << std::fixed << std::setprecision(2) << slider;
  std::string s = stream.str();
  stream.clear();
  al_draw_text(font, al_map_rgb(0, 0, 0), FONT_X, y, ALLEGRO_ALIGN_CENTRE,
               (std::string(ch + "[" + s + "]")).c_str());
}

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
                     float val, int slY, int chY, BYTE flag) {
  // redraw text
  al_set_clipping_rectangle(M_CHX, chY, SL_X1 - M_CHX, FONT_S);
  al_clear_to_color(al_map_rgb(200, 200, 200));
  al_reset_clipping_rectangle();

  if (flag == FSL) drawColorVal(font, val, chY, ch);
  if (flag == FH) drawColorVal(font, std::floor(val * 360), chY, ch);
  if (flag == FRGB) drawColorVal(font, std::floor(val * 255), chY, ch);

  // redraw slider
  int x1 = SL_X1 - BTN_RO - 1, y1 = slY - BTN_RO - 1,
      w = SL_X2 - SL_X1 + 2 * BTN_RO + 2, h = FONT_S + 2;
  al_set_clipping_rectangle(x1, y1, w, h);
  al_clear_to_color(al_map_rgb(200, 200, 200));
  al_reset_clipping_rectangle();

  // draw hue image for hue HSL component
  if (flag == FH && hue != nullptr) {
    al_draw_bitmap(hue, SL_X1, slY - SL_W / 2 - 1, 0);
  } else {
    al_draw_line(SL_X1, slY, SL_X2, slY, al_map_rgb(100, 100, 100), SL_W);
  }

  // redraw circles
  al_draw_circle(SL_X1 + (SL_X2 - SL_X1) * val, slY, BTN_RI,
                 al_map_rgb(0, 0, 0), BTN_TH);
  al_draw_circle(SL_X1 + (SL_X2 - SL_X1) * val, slY, BTN_RO,
                 al_map_rgb(0, 0, 0), BTN_TH);
}

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
               ModalState& oldState, ModalState& newState) {
  ALLEGRO_COLOR col = {newState.vals[0], newState.vals[1], newState.vals[2], 1};
  al_set_target_bitmap(modal);
  RGB _old = {
      oldState.vals[0],
      oldState.vals[1],
      oldState.vals[2],
  };
  RGB _new = {
      newState.vals[0],
      newState.vals[1],
      newState.vals[2],
  };

  if (_new != _old) {
    // refresh the color plane and redraw it
    al_set_clipping_rectangle(M_X, M_Y, M_W, M_H);
    al_clear_to_color(al_map_rgb(200, 200, 200));
    al_draw_filled_rectangle(M_X, M_Y, M_X + M_W, M_Y + M_H, col);
    al_draw_rectangle(M_X, M_Y, M_X + M_W, M_Y + M_H, al_map_rgb(0, 0, 0), 2);
    al_reset_clipping_rectangle();

    // redraw the changed values and sliders in changed color components
    if (newState.vals[0] != oldState.vals[0]) {
      conditionalDraw(hue, font, "R", newState.vals[0], M_SL1, M_CH1, FRGB);
    }
    if (newState.vals[1] != oldState.vals[1]) {
      conditionalDraw(hue, font, "G", newState.vals[1], M_SL2, M_CH2, FRGB);
    }
    if (newState.vals[2] != oldState.vals[2]) {
      conditionalDraw(hue, font, "B", newState.vals[2], M_SL3, M_CH3, FRGB);
    }
    if (newState.vals[3] != oldState.vals[3]) {
      conditionalDraw(hue, font, "H", newState.vals[3], M_SL4, M_CH4, FH);
    }
    if (newState.vals[4] != oldState.vals[4]) {
      conditionalDraw(hue, font, "S", newState.vals[4], M_SL5, M_CH5, FSL);
    }
    if (newState.vals[5] != oldState.vals[5]) {
      conditionalDraw(hue, font, "L", newState.vals[5], M_SL6, M_CH6, FSL);
    }
    al_reset_clipping_rectangle();
    return true;
  }
  al_reset_clipping_rectangle();
  return false;
}

/// <summary>
/// Chech whether point is in circle
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="r"></param>
/// <param name="w"></param>
/// <param name="h"></param>
/// <returns></returns>
bool inRadius(int x, int y, int r, int w, int h) {
  return (x - w) * (x - w) + (y - h) * (y - h) <= r * r;
}

/// <summary>
/// Check whether mouse is out of the modal window
/// </summary>
/// <param name="xy"> Mouse location</param>
/// <returns></returns>
bool outOfModal(vec2<int>& xy) {
  int sw = MW_X, sh = MW_Y, ew = MM_WIDTH / 2 + MW_W / 2,
      eh = MM_HEIGHT / 2 + MW_H / 2;
  // out of bounds
  if (xy.x < sw || xy.y < sh || xy.x > ew || xy.y > eh) return true;
  return false;
}

/// <summary>
/// Mouse drag event initialization function.
/// </summary>
/// <param name="xy"> Mouse location</param>
/// <param name="ms"> Color state</param>
/// <returns></returns>
BYTE dragStartHandle(vec2<int>& xy, ModalState& ms) {
  // out of bounds
  if (outOfModal(xy)) return 6;
  int x = xy.x - MW_X, y = xy.y - MW_Y;

  // check whether slider button is being pressed
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[0], M_SL1)) {
    return 0;
  }
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[1], M_SL2)) {
    return 1;
  }
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[2], M_SL3)) {
    return 2;
  }
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[3], M_SL4)) {
    return 3;
  }
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[4], M_SL5)) {
    return 4;
  }
  if (inRadius(x, y, BTN_RO, SL_X1 + (SL_X2 - SL_X1) * ms.vals[5], M_SL6)) {
    return 5;
  }
  return 6;
}

/// <summary>
/// Mouse drag handler
/// </summary>
/// <param name="x"> x location of the mouse</param>
/// <param name="idx"> Index of color component</param>
/// <param name="ms"> Color state</param>
void dragSlider(int x, int idx, ModalState& ms) {
  float val;
  int minx = SL_X1, maxx = SL_X2, d = SL_X2 - SL_X1;

  // select percentage
  if (x < minx) {
    if (ms.vals[idx] == 0.0f) return;
    val = 0.0f;
  } else if (x > maxx) {
    if (ms.vals[idx] == 1.0f) return;
    val = 1.0f;
  } else {
    float t = (float)(maxx - x) / (float)d;
    val = 1.0f - t;
  }

  // save change and change color
  ms.vals[idx] = val;
  if (idx < 3) {
    // RGB slider is being dragged
    RGB rgb = {ms.vals[0], ms.vals[1], ms.vals[2]};
    HSL hsl = Utils::RGB2HSL(rgb);
    ms.vals[3] = hsl.H / 360.0f;
    ms.vals[4] = hsl.S;
    ms.vals[5] = hsl.L;
  } else {
    // HSL slider is being dragged
    HSL hsl = {ms.vals[3] * 360.0f, ms.vals[4], ms.vals[5]};
    RGB rgb = Utils::HSL2RGB(hsl);
    ms.vals[0] = rgb.r;
    ms.vals[1] = rgb.g;
    ms.vals[2] = rgb.b;
  }
}

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
                 BYTE& mouse_bs, vec2<int>& xy, BYTE& drag) {
  ModalState current;

  // one of the sliders is being dragged
  if (drag < 6) {
    int x = xy.x - MW_X;
    current = old;
    dragSlider(x, drag, current);
    if (drawModal(modal, hue, font, old, current)) old = current;
  }

  if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
      (event.mouse.button & 1)) {
    mouse_bs = LMB;
    drag = dragStartHandle(xy, old);
  }

  if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
    mouse_bs = REL;
    drag = 6;
  }
}

//----------------------------------------------------------------
