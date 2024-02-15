// Basic utility code for working with images
// Written by Ondrej Jamriska at the Czech Technical University in Prague
// Modified by Tomas Cicvarek at the Czech Technical University in Prague

// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy
// and modify this file however you want.

// Original file to this one can be found in GridCut examples.
// Because of the include restrictions posed by the stb_image
// library files, the cpp file was additionally created.

#ifndef IMAGE_H_
#define IMAGE_H_

#include <assert.h>

#include <cstdlib>
#include <iostream>
#include <string>

/// <summary>
/// Struct from GridCut library. Represents RGB color in [0-1] range.
/// </summary>
struct RGB {
  RGB();
  RGB(float r, float g, float b);

  float r, g, b;
};

RGB operator+(const RGB& u, const RGB& v);
RGB operator*(const float s, const RGB& u);
RGB operator*(const RGB& u, const float s);
RGB operator/(const RGB& u, const float s);
bool operator<(const RGB& u, const RGB& v);
bool operator==(const RGB& u, const RGB& v);
bool operator!=(const RGB& u, const RGB& v);
std::ostream& operator<<(std::ostream& os, const RGB& rgb);

/// <summary>
/// Struct from GridCut library. Represents the image.
/// </summary>
/// <typeparam name="T"> Color data</typeparam>
template <typename T>
class Image {
 public:
  Image() : _width(0), _height(0), _data(0) {}
  Image(int width, int height) {
    assert(width > 0 && height > 0);
    _width = width;
    _height = height;
    _data = new T[(size_t)_width * (size_t)_height];
  }
  Image(const Image<T>& image) {
    _width = image._width;
    _height = image._height;

    if (_width > 0 && _height > 0) {
      _data = new T[_width * _height];
      for (int i = 0; i < _width * _height; i++) {
        _data[i] = image._data[i];
      }
    } else {
      _data = 0;
    }
  }
  ~Image() { delete[] _data; }

  Image<T>& operator=(const Image<T>& image) {
    if (this != &image) {
      if (_width == image._width && _height == image._height) {
        for (int i = 0; i < _width * _height; i++) {
          _data[i] = image._data[i];
        }
      } else {
        delete[] _data;
        _width = image._width;
        _height = image._height;

        if (image._width > 0 && image._height > 0) {
          _data = new T[_width * _height];
          for (int i = 0; i < _width * _height; i++) {
            _data[i] = image._data[i];
          }
        } else {
          _data = 0;
        }
      }
    }
    return *this;
  }

  inline T& operator()(int x, int y) {
    assert(_data != 0);
    assert(x >= 0 && x < _width && y >= 0 && y < _height);

    return _data[x + y * _width];
  }

  inline const T& operator()(int x, int y) const {
    assert(_data != 0);
    assert(x >= 0 && x < _width && y >= 0 && y < _height);

    return _data[x + y * _width];
  }

  int width() const { return _width; }
  int height() const { return _height; }

  T* data() { return _data; }

  const T* data() const { return _data; }

  void reset() {
    delete[] _data;
    _data = new RGB[_width * _height];
  }

 private:
  int _width;
  int _height;
  T* _data;
};

/// <summary>
/// Function used to load an image from memory.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="fileName"></param>
/// <returns></returns>
template <typename T>
Image<T> imread(const std::string& fileName);

/// <summary>
/// Function used to store the image in memory.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="image"></param>
/// <param name="fileName"></param>
/// <returns></returns>
template <typename T>
bool imwrite(const Image<T>& image, const std::string& fileName);

/// <summary>
/// Converts image structure to PNG file in memory.
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="image"></param>
/// <param name="len"></param>
/// <returns></returns>
template <typename T>
unsigned char* memFile(const Image<T>& image, int* len);

#endif
