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

#include <algorithm>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG

#include <fstream>
#include <iomanip>
#include <sstream>

#include "AllegroOperations.h"
#include "ColorSegments.h"
#include "Depth.h"
#include "ShapeFill.h"
#include "Utils.h"

/// <summary>
/// Draws an arrow to selected coordinates with bordering and filling color.
/// </summary>
/// <param name="start">First coordinate</param>
/// <param name="end">Destination coordinate</param>
/// <param name="cb">Bordering color</param>
/// <param name="cf">Filling color</param>
void draw_arrow(vec2<int>& start, vec2<int>& end, ALLEGRO_COLOR& cb,
                ALLEGRO_COLOR& cf) {
  vec2<float> dir = vec2<float>(end.x, end.y) - vec2<float>(start.x, start.y);
  vec2<float> normD = dir / dir.norm();
  // three points crossing the line to create arrow
  vec2<float> l, m, r;
  m = vec2<float>(end.x, end.y) - normD * ARROW_HEAD_SIZE;
  vec2<float> perp = vec2<float>(normD.y, -normD.x) * ARROW_HEAD_SIZE;
  l = m - perp;
  r = m + perp;

  al_draw_line(start.x, start.y, m.x, m.y, cb, ARROW_THICKNESS);
  al_draw_line(start.x, start.y, m.x, m.y, cf, ARROW_THICKNESS - 2);
  al_draw_filled_triangle(l.x, l.y, r.x, r.y, end.x, end.y, cf);
  al_draw_triangle(l.x, l.y, r.x, r.y, end.x, end.y, cb, 1);
}

/// <summary>
/// Uses allegro library to draw arrows to the screen.
/// </summary>
/// <param name="depth"></param>
void draw_edges(Depth& depth) {
  float size = ARROW_THICKNESS * 2.0f;
  ALLEGRO_COLOR cf = {ARROW_COLOR_R, ARROW_COLOR_G, ARROW_COLOR_G, 1},
                cb = {ARROW_COLOR_R * 0.5f, ARROW_COLOR_G * 0.5f,
                      ARROW_COLOR_B * 0.5f, 1},
                cnf = {ARROW2_COLOR_R, ARROW2_COLOR_G, ARROW2_COLOR_B, 1},
                cnb = {ARROW2_COLOR_R * 0.5f, ARROW2_COLOR_G * 0.5f,
                       ARROW2_COLOR_B * 0.5f, 1};
  for (const Coordinates& ed : depth.graphicData) {
    int x1 = ed.x1 & 1023;
    int flag = ed.x1 & 1024;
    vec2<int> s = {x1, ed.y1}, e = {ed.x2, ed.y2};
    ALLEGRO_COLOR b, f;
    if (flag == 0) {
      b = cb;
      f = cf;
    } else {
      b = cnb;
      f = cnf;
    }

    draw_arrow(s, e, b, f);
  }
}

/// <summary>
/// Uses allegro library to draw error arrow to the screen.
/// </summary>
/// <param name="coords">Arrow coordinates</param>
/// <param name="interp">Color interpolation</param>
/// <param name="incr">Increment to interpolation</param>
/// <param name="iter">Number of iterations</param>
void draw_err_edge(vec2<int>* coords, float& interp, float& incr, BYTE& iter) {
  RGB red = RGB(1, 0, 0) * interp, green = RGB(0, 1, 0) * (1.0f - interp);
  RGB col = red + green;
  ALLEGRO_COLOR cf = {col.r, col.g, col.b, 1},
                cb = {col.r * 0.5f, col.g * 0.5f, col.b * 0.5f, 1};

  draw_arrow(coords[0], coords[1], cb, cf);

  // tresholds met, go the other direction
  interp += incr;
  if (interp <= 0.0f) {
    incr = -incr;
    interp = 0.0f;
  }
  if (interp >= 1.0f) {
    incr = -incr;
    interp = 1.0f;
    iter += 1;
  }
}

/// <summary>
/// Draws area of selection to block merging segments.
/// </summary>
/// <param name="fromToCoords">Minimal and maximal coordinats</param>
/// <param name="x">Current x coordinate</param>
/// <param name="y">Current y coordinate</param>
void draw_block_area(vec2<int>* fromToCoords, int x, int y) {
  ALLEGRO_COLOR col;
  col.b = 0.5f;
  col.g = col.r = 0.0f;
  col.a = 0.8f;
  vec2<int> minc = {std::min(fromToCoords[0].x, x),
                    std::min(fromToCoords[0].y, y)};
  vec2<int> maxc = {std::max(fromToCoords[0].x, x),
                    std::max(fromToCoords[0].y, y)};
  al_draw_filled_rectangle(minc.x, minc.y, maxc.x, maxc.y, col);
}

/// <summary>
/// Calls the function to segment the image depending on scribbles. Then updates
/// the screen and changes scribbles to the default one, allowing
/// multi-labeling.
/// </summary>
/// <param name="screen">Screen bitmap</param>
/// <param name="c_map">Color map</param>
/// <param name="intensityImg">Original image</param>
/// <param name="scribbles">User input</param>
void graphCut(ALLEGRO_BITMAP* screen, ColorMap& c_map,
              Image<float>& intensityImg, short* scribbles) {
  std::cout << "Start Segmentation\n";
  ColorSegments::applyScribbles(intensityImg, c_map, scribbles);
  std::cout << "Finished\n";
}

/// <summary>
/// Checks for any scribbles that may have been overlapped by the user and
/// switches their indices with the last usable indices.
/// </summary>
/// <param name="c_map">Color map</param>
/// <param name="depth">Depth data</param>
/// <param name="scribbles">Scribble data map</param>
void checkScribbles(ColorMap& c_map, Depth& depth, short* scribbles) {
  std::set<BYTE> marks[2];
  std::map<BYTE, BYTE> changes;
  for (int h = 0; h < c_map.getHeight(); h++) {
    for (int w = 0; w < c_map.getWidth(); w++) {
      if (scribbles[w + h * c_map.getWidth()] >= 0)
        if (scribbles[w + h * c_map.getWidth()] < 128)
          marks[0].insert(scribbles[w + h * c_map.getWidth()]);
        else
          marks[1].insert(scribbles[w + h * c_map.getWidth()]);
    }
  }
  bool reset = false;
  if (marks[0].size() != c_map.getScribbleCount()[0]) {
    reset = true;
    c_map.consolidate(marks[0], changes, 0);
  }
  if (marks[1].size() != c_map.getScribbleCount()[1]) {
    c_map.consolidate(marks[1], changes, 1);
    reset = true;
  }
  if (reset) {
    depth.reset(c_map.getScribbleCount());
    for (int i = 0; i < c_map.getWidth() * c_map.getHeight(); i++)
      if (changes.find(scribbles[i]) != changes.end()) {
        scribbles[i] = changes[scribbles[i]];
      }
  }
}

/// <summary>
/// Saves necessary data to binary form.
/// </summary>
/// <param name="c_map">Color map</param>
/// <param name="scribbleData">User input</param>
/// <param name="depth">Depth data</param>
/// <param name="operations">Blur</param>
/// <param name="name">Image name without extention</param>
void save(ColorMap& c_map, short* scribbleData, BYTE* block, Depth& depth,
          const std::string name) {
  std::ofstream wfs;
  wfs.open("saves/save" + name + ".bin", std::ios::out | std::ios::binary);
  if (!wfs.good()) {
    std::cout << "File creation failed\n";
    return;
  }
  {
    int i = c_map.getWidth();
    wfs.write(reinterpret_cast<const char*>(&i), sizeof(int));
    i = c_map.getHeight();
    wfs.write(reinterpret_cast<const char*>(&i), sizeof(int));
    wfs.write(reinterpret_cast<const char*>(scribbleData),
              c_map.getWidth() * c_map.getHeight() * sizeof(short));
    wfs.write(reinterpret_cast<const char*>(c_map.data()),
              c_map.getWidth() * c_map.getHeight() * sizeof(short));
    wfs.write(reinterpret_cast<const char*>(block),
              c_map.getWidth() * c_map.getHeight() * sizeof(BYTE));
  }
  wfs.write((char*)c_map.getScribbleCount(), 2 * sizeof(int));
  for (size_t i = 0; i < c_map.getScribbleCount()[0]; i++) {
    float col[3] = {c_map.getColors()[i].r, c_map.getColors()[i].g,
                    c_map.getColors()[i].b};
    wfs.write(reinterpret_cast<const char*>(col), 3 * sizeof(float));
  }
  for (size_t i = 0; i < c_map.getScribbleCount()[1]; i++) {
    float col[3] = {c_map.getColors()[i + 128].r, c_map.getColors()[i + 128].g,
                    c_map.getColors()[i + 128].b};
    wfs.write(reinterpret_cast<const char*>(col), 3 * sizeof(float));
  }

  {
    size_t i = depth.graphicData.size();
    wfs.write(reinterpret_cast<const char*>(&i), sizeof(size_t));
  }
  if (!depth.graphicData.empty()) {
    int* coordinates = (int*)malloc(depth.graphicData.size() * 4 * sizeof(int));
    int i = 0;
    for (std::list<Coordinates>::iterator it = depth.graphicData.begin();
         it != depth.graphicData.end(); it++, i++) {
      coordinates[4 * i] = it->x1;
      coordinates[4 * i + 1] = it->y1;
      coordinates[4 * i + 2] = it->x2;
      coordinates[4 * i + 3] = it->y2;
    }
    wfs.write(reinterpret_cast<const char*>(coordinates),
              depth.graphicData.size() * (size_t)4 * sizeof(int));
    free(coordinates);
  }
  wfs.close();
}

/// <summary>
/// Loads all saved information from binary file.
/// </summary>
/// <param name="c_map">Color map</param>
/// <param name="scribbleData">User input</param>
/// <param name="block"> Mask blocking/forcing merge</param>
/// <param name="intensityImg">Original image</param>
/// <param name="depth">Depth data</param>
/// <param name="operations">Blur</param>
/// <param name="filename">Image name with extention</param>
/// <param name="name">Image name without extention</param>
/// <returns>Success</returns>
bool load(ColorMap& c_map, short* scribbleData, BYTE* block,
          Image<float>& intensityImg, Depth& depth, std::string filename,
          std::string name) {
  // open and check file
  std::ifstream rfs;
  rfs.open("saves/save" + name + ".bin", std::ios::in | std::ios::binary);
  if (!rfs.good()) {
    std::cout << "File loading failed\n";
    return false;
  }

  // read size
  // not neccessary now, but for custom sizes this can check correctness of the
  // input
  int h, w;
  rfs.read((char*)(&w), sizeof(int));
  rfs.read((char*)(&h), sizeof(int));
  if (w != c_map.getWidth() || h != c_map.getHeight()) {
    std::cout << "This save file has different size compared to the image!\n";
    rfs.close();
    return false;
  }
  // read scribbles
  rfs.read((char*)(scribbleData),
           c_map.getWidth() * c_map.getHeight() * sizeof(short));

  // color map loading sequance, load colors to the color palette and their
  // count
  c_map.reset();
  int scrCnt[2];
  rfs.read((char*)(c_map.data()),
           c_map.getWidth() * c_map.getHeight() * sizeof(short));
  // read merge blocking data and continue with the color map
  rfs.read((char*)(block), c_map.getWidth() * c_map.getHeight() * sizeof(BYTE));
  // read number of scribbles both soft and hard ones
  rfs.read((char*)scrCnt, 2 * sizeof(int));
  c_map.setScribbleCount(scrCnt);
  // set their colors
  for (int i = 0; i < scrCnt[0]; i++) {
    float col[3];
    rfs.read((char*)(col), 3 * sizeof(float));
    c_map.setColors(i, {col[0], col[1], col[2]});
  }
  for (int i = 0; i < scrCnt[1]; i++) {
    float col[3];
    rfs.read((char*)(col), 3 * sizeof(float));
    c_map.setColors(i + 128, {col[0], col[1], col[2]});
  }

  // load depth data
  depth.reset(c_map.getScribbleCount());
  if (scrCnt[0] + scrCnt[1] > 1) {
    // it is not possible to create graph without segments
    // load depth values
    size_t _size;
    rfs.read((char*)(&_size), sizeof(size_t));
    if (_size > 0) {
      int cds[4];
      for (int i = 0; i < _size; i++) {
        rfs.read((char*)(cds), 4 * sizeof(int));
        vec2<int> coords[2] = {{cds[0], cds[1]}, {cds[2], cds[3]}};
        // do not forget that there is a flag hidden in the first x coordinate
        vec2<int> unfl = {coords[0].x & 1023, coords[0].y};
        int flag = (coords[0].x & 1024) == 0 ? 0 : 1;
        depth.addEdge(c_map, c_map.getMaskAt(unfl), c_map.getMaskAt(coords[1]),
                      coords, flag);
      }
    }
  }
  rfs.close();
  return true;
}

/// <summary>
/// Saves current screen.
/// </summary>
/// <param name="screen"> Allegro bitmap</param>
void saveScreen(ALLEGRO_BITMAP* screen, std::string append = "") {
  if (!al_save_bitmap(std::string("pictures/screen" + append + ".png").c_str(),
                      screen))
    std::cerr << "Error while saving image: error " << al_get_errno()
              << std::endl;
}

int main(int argc, char* argv[]) {
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  {
    ALLEGRO_DISPLAY* display = nullptr;
    ALLEGRO_EVENT_QUEUE* queue = nullptr;
    ALLEGRO_TIMER* timer = nullptr;
    ALLEGRO_BITMAP* screen = nullptr;
    ALLEGRO_FONT* font = nullptr;

    ALLEGRO_BITMAP* modal = nullptr;
    ALLEGRO_BITMAP* hue = nullptr;
    ModalState current = {-1, -1, -1, -1, -1, -1};

    short* scribbleData = nullptr;
    std::string name;
    std::string filename;

    Image<float> intensityImg =
        firstInit(display, queue, font, hue, timer, screen, filename, name);

    Image<RGB> arrows(al_get_bitmap_width(screen),
                      al_get_bitmap_height(screen));
    ColorMap c_map(al_get_bitmap_width(screen), al_get_bitmap_height(screen));
    Depth depth(c_map.getScribbleCount());
    ColorSegments::createBackgroundScribbles(scribbleData,
                                             al_get_bitmap_width(screen),
                                             al_get_bitmap_height(screen));
    ShapeFill sf;
    RGB oldCol;
    vec2<int> xy = {0, 0}, xy_old = {0, 0};  // X and Y mouse coordinates
    float interp = 0, incr = INIT_INCR;
    bool inProcess = false, shiftDown = false;
    BYTE mode = 0, iter = 0, lastActive = 0, displayFlags = 1, mouse_bs = REL,
         drag = 6;
    char scrFlags = 0;
    vec2<int> fromToCoords[2] = {{-1, -1}, {-1, -1}};
    unsigned int key = 0;

    BYTE* block = new BYTE[intensityImg.width() * intensityImg.height()];
    for (int i = 0; i < intensityImg.width() * intensityImg.height(); i++)
      block[i] = 0;

    std::cout << (scrFlags & 1 ? "Soft scribbles" : "Hard scribbles") << std::endl;
    std::cout << (scrFlags & 2 ? "Colors locked" : "Colors unlocked") << std::endl;
    std::cout << "Color: " << ColorSegments::foreground.r << ' '
              << ColorSegments::foreground.g << ' '
              << ColorSegments::foreground.b << ' ' << std::endl
              << "Draw mode" << std::endl;
    al_start_timer(timer);

    set_screen(intensityImg, c_map, screen, scribbleData, block, displayFlags);

    // Main loop
    while (true) {
      ALLEGRO_EVENT event;

      al_wait_for_event(queue, &event);
      if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
        break;
      }
      // key handles
      ALLEGRO_KEYBOARD_STATE keyState;
      al_get_keyboard_state(&keyState);

      // common mouse event handle
      if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
        xy = {event.mouse.x, event.mouse.y};
      }

      // handle modal window
      if ((mode & COL) != 0) {
        handleModal(event, hue, modal, font, current, mouse_bs, xy, drag);
        if (al_key_down(&keyState, ALLEGRO_KEY_ENTER) ||
            al_key_down(&keyState, ALLEGRO_KEY_PAD_ENTER)) {
          mode = (mode & (COL - 1));
          ColorSegments::foreground = {current.vals[0], current.vals[1],
                                       current.vals[2]};
          key = 0;
          mouse_bs = REL;
          al_destroy_bitmap(modal);
        }
      }

      if ((mode & ANIM) == 0 && (mode & COL) == 0) {
        // additional key
        shiftDown = al_key_down(&keyState, ALLEGRO_KEY_LSHIFT);

        // mouse events
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            (event.mouse.button & 1) && mouse_bs != RMB && key == 0) {
          mouse_bs = LMB;
          if (mode == DRAW) {
            if (shiftDown && lastActive != 0) {
              c_map.setActive(lastActive);
            } else {
              c_map.newSegment(ColorSegments::foreground, scrFlags);
              lastActive = c_map.getActive();
            }
          }
        }
        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            (event.mouse.button & 2) && mouse_bs != LMB && key == 0) {
          mouse_bs = RMB;
          if (mode == DRAW) {
            c_map.setActive(0);
          }
        }

        if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && key == 0) {
          if (event.mouse.button == mouse_bs) {
            if (mode == DRAW) {
              al_reset_clipping_rectangle();
              checkScribbles(c_map, depth, scribbleData);
              depth.update(
                  (scrFlags & MASK_SCRIBBLE_TYPE) * 128 +
                  c_map.getScribbleCount()[scrFlags & MASK_SCRIBBLE_TYPE] - 1);
              std::cout
                  << "Counter : "
                  << ((scrFlags & MASK_SCRIBBLE_TYPE) * 128 +
                      c_map.getScribbleCount()[scrFlags & MASK_SCRIBBLE_TYPE] - 1)
                  << "\nMask: " << (int)lastActive << " "
                  << (int)c_map.getActive() << std::endl;
            }
            if (mode == DEPTH) {
              if (fromToCoords[0].x == -1) {
                fromToCoords[0] = xy;
                inProcess = true;
              } else {
                fromToCoords[1] = xy;
                if (c_map.getMaskAt(fromToCoords[0]) != -1) {
                  BYTE flag = 0;
                  if (shiftDown) flag = 1;
                  bool status =
                      depth.addEdge(c_map,
                                    (BYTE)c_map.getMaskAt(fromToCoords[0].x,
                                                          fromToCoords[0].y),
                                    c_map.getMaskAt(xy), fromToCoords, flag);
                  if (status)
                    fromToCoords[0].x = -1;
                  else
                    mode = mode | ANIM;
                } else {
                  inProcess = false;
                  mode = mode | ANIM;
                }
                inProcess = false;
              }
            }
            if (mode == BLOCK) {
              if (fromToCoords[0].x == -1) {
                fromToCoords[0] = xy;
                inProcess = true;
              } else {
                fromToCoords[1] = xy;
                vec2<int> minc = {
                    std::min(fromToCoords[0].x, fromToCoords[1].x),
                    std::min(fromToCoords[0].y, fromToCoords[1].y)};
                vec2<int> maxc = {
                    std::max(fromToCoords[0].x, fromToCoords[1].x),
                    std::max(fromToCoords[0].y, fromToCoords[1].y)};
                for (int h = minc.y; h <= maxc.y; h++)
                  for (int w = minc.x; w <= maxc.x; w++)
                    block[w + h * intensityImg.width()] = mouse_bs;
                fromToCoords[0].x = -1;
                inProcess = false;
              }
            }
            mouse_bs = REL;
          }
          set_screen(intensityImg, c_map, screen, scribbleData, block, displayFlags);
        }
        if (mouse_bs != REL && mode == DRAW && key == 0) {
          if (xy_old != xy) {
            circleFillAllegro(xy.x, xy.y, screen, scribbleData, RADIUS,
                              c_map.getColors()[c_map.getActive()],
                              c_map.getActive());
            xy_old = xy;
          }
        }
        if (mode == PICK && mouse_bs == LMB && key == 0) {
          if (c_map.getMaskAt(xy) > 0) {
            lastActive = c_map.getMaskAt(xy);
            c_map.setActive(lastActive);
          }
        }

        if (key == 0 && mouse_bs == REL && !inProcess) {
          // quit
          if (al_key_down(&keyState, ALLEGRO_KEY_ESCAPE) ||
              al_key_down(&keyState, ALLEGRO_KEY_Q)) {
            break;
          }

          // map scribbles to the regions, run graph cut
          if (al_key_down(&keyState, ALLEGRO_KEY_M)) {
            graphCut(screen, c_map, intensityImg, scribbleData);
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            key = ALLEGRO_KEY_M;
          }

          // complete overlapped parts
          if (al_key_down(&keyState, ALLEGRO_KEY_O) && !depth.order.empty()) {
            std::cout << "Shape filling start\n";
            depth.computeDepths();
            sf.shapeFill(depth, c_map, intensityImg.data(), filename, block,
                         name);
            key = ALLEGRO_KEY_O;
            std::cout << "Done\n";
          }

          // reset application
          if (al_key_down(&keyState, ALLEGRO_KEY_R)) {
            if (shiftDown) {
              intensityImg = imread<float>(FOLDER + filename);
              Utils::scaleAndPad(intensityImg);
            } else {
              reset(screen);
              for (int i = 0; i < intensityImg.width() * intensityImg.height();
                   i++)
                block[i] = 0;
              // reset intensity image
              intensityImg = imread<float>(FOLDER + filename);
              Utils::scaleAndPad(intensityImg);
              c_map.reset();
              depth.reset(c_map.getScribbleCount());
              ColorSegments::createBackgroundScribbles(
                  scribbleData, c_map.getWidth(), c_map.getHeight());
              mode = DRAW;
              std::cout << "Draw mode\n";
            }
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            key = ALLEGRO_KEY_R;
          }

          // save current screen
          if (al_key_down(&keyState, ALLEGRO_KEY_S)) {
            std::cout << "Saving data, wait please.\n";
            ColorSegments::printScribbles(c_map, scribbleData, intensityImg, 1);
            saveScreen(al_get_backbuffer(display));
            depth.printDepth(c_map);
            ColorSegments::printScribbleData(scribbleData, c_map);
            c_map.printSegments();
            save(c_map, scribbleData, block, depth, name);
            imwrite(intensityImg, FOLDER + name + "_mod.png");
            std::cout << "Saving done\n";
            key = ALLEGRO_KEY_S;
          }

          // switch to depth mode and back to draw mode
          if (al_key_down(&keyState, ALLEGRO_KEY_D)) {
            if (shiftDown) {
              // reset
              depth.reset(c_map.getScribbleCount());
            } else {
              mode = mode == DEPTH ? DRAW : DEPTH;
              std::cout << (mode == DRAW ? "Draw mode" : "Depth mode")
                        << std::endl;
            }
            key = ALLEGRO_KEY_D;
          }

          if (al_key_down(&keyState, ALLEGRO_KEY_P)) {
            key = ALLEGRO_KEY_P;
            mode = mode == PICK ? DRAW : PICK;
            std::cout << (mode == DRAW ? "Draw mode" : "Picking mode")
                      << std::endl;
          }

          if (al_key_down(&keyState, ALLEGRO_KEY_V)) {
            key = ALLEGRO_KEY_V;
            if (shiftDown) {
              // reset
              for (int i = 0; i < intensityImg.width() * intensityImg.height();
                   i++)
                block[i] = 0;
              set_screen(intensityImg, c_map, screen, scribbleData, block,
                         displayFlags);
            } else {
              mode = mode == BLOCK ? DRAW : BLOCK;
              std::cout << (mode == DRAW ? "Draw mode" : "Merge blocking mode")
                        << std::endl;
            }
          }

          // change soft/hard scribbles
          if (al_key_down(&keyState, ALLEGRO_KEY_H) && mode == DRAW) {
            scrFlags = ((scrFlags + 1) & MASK_SCRIBBLE_TYPE) + (scrFlags & MASK_OTHER_1);
            key = ALLEGRO_KEY_H;
            std::cout << (scrFlags & MASK_SCRIBBLE_TYPE ? "Soft scribbles"
                                                     : "Hard scribbles")
                      << std::endl;
          }

          if (al_key_down(&keyState, ALLEGRO_KEY_A)) {
            displayFlags = (displayFlags + 1) % 4;
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            key = ALLEGRO_KEY_A;
          }

          // load from binary file
          if (al_key_down(&keyState, ALLEGRO_KEY_X)) {
            load(c_map, scribbleData, block, intensityImg, depth, filename,
                 name);
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            std::cout << "Done\n";
            key = ALLEGRO_KEY_X;
          }

          // change color
          if (al_key_down(&keyState, ALLEGRO_KEY_C) && mode == DRAW) {
            init_modal_window(modal);
            if (modal == nullptr) {
              std::cout << "Type RGB color [0 - 1]: ";
              std::cin >> current.vals[0] >> current.vals[1] >> current.vals[2];
              current.vals[0] = std::max(0.0f, std::min(current.vals[0], 1.0f));
              current.vals[1] = std::max(0.0f, std::min(current.vals[1], 1.0f));
              current.vals[2] = std::max(0.0f, std::min(current.vals[2], 1.0f));

              ColorSegments::foreground = {current.vals[0], current.vals[1],
                                           current.vals[2]};
              std::cout << std::endl;
              key = ALLEGRO_KEY_C;
            } else {
              ModalState ms;
              RGB rgb = ColorSegments::foreground;
              HSL hsl = Utils::RGB2HSL(rgb);
              ms.vals[0] = rgb.r;
              ms.vals[1] = rgb.g;
              ms.vals[2] = rgb.b;
              ms.vals[3] = hsl.H / 360.f;
              ms.vals[4] = hsl.S;
              ms.vals[5] = hsl.L;
              for (int i = 0; i < 6; i++) {
                current.vals[i] = -1;
              }
              al_set_target_backbuffer(display);
              drawModal(modal, hue, font, current, ms);
              current = ms;
              mode = mode | COL;
            }
          }

          // add contrast for computations
          if (al_key_down(&keyState, ALLEGRO_KEY_K) && mode == DRAW) {
            Utils::gammaCorrection(intensityImg, 2);
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            key = ALLEGRO_KEY_K;
          }

          // blurAndTreshold image
          if (al_key_down(&keyState, ALLEGRO_KEY_B) && mode == DRAW) {
            Utils::blur(intensityImg, 1);
            set_screen(intensityImg, c_map, screen, scribbleData, block,
                       displayFlags);
            key = ALLEGRO_KEY_B;
          }
        }
        // enable keys
        if (event.keyboard.type == ALLEGRO_EVENT_KEY_UP) {
          unsigned int released = event.keyboard.keycode;
          if (released == key) key = 0;
        }
      }
      if (event.type == ALLEGRO_EVENT_TIMER) {
        al_set_target_backbuffer(display);
        al_reset_clipping_rectangle();
        al_clear_to_color(al_map_rgba(0, 0, 0, 0));
        al_draw_bitmap(screen, 0, 0, 0);
        if (mode == BLOCK && fromToCoords[0].x != -1) {
          draw_block_area(fromToCoords, xy.x, xy.y);
        }
        if (displayFlags == 1 || displayFlags == 3) draw_edges(depth);
        if ((mode & ANIM) > 0) {
          draw_err_edge(fromToCoords, interp, incr, iter);
          if (iter >= 2) {
            iter = 0;
            interp = 0;
            incr = INIT_INCR;
            fromToCoords[0].x = -1;
            mode = mode & 3;
          }
        }
        if ((mode & COL) != 0) {
          al_draw_bitmap(modal, MW_X, MW_Y, 0);
        }
        al_flip_display();
      }
    }

    cleanup(display, timer, screen, font, hue);
    delete[] scribbleData;
    delete[] block;
  }

  _CrtDumpMemoryLeaks();
  return 0;
}
