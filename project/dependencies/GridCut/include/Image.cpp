// Basic utility code for working with images
// Written by Ondrej Jamriska at the Czech Technical University in Prague
// Modified by Tomas Cicvarek at the Czech Technical University in Prague

// This software is in the public domain. Where that dedication is not
// recognized, you are granted a perpetual, irrevocable license to copy
// and modify this file however you want.

// Definitions that originated in the Image.h file which can found in GridCut
// examples. Because of the include restrictions posed by the stb_image
// library files, the cpp file was additionally created.

#include "Image.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

RGB::RGB() : r(0), g(0), b(0) {}

RGB::RGB(float r, float g, float b) : r(r), g(g), b(b) {}

RGB operator+(const RGB& u, const RGB& v) {
  return RGB(std::min(u.r + v.r, 1.0f), std::min(u.g + v.g, 1.0f),
             std::min(u.b + v.b, 1.0f));
}

RGB operator*(const float s, const RGB& u) {
  return RGB(s * u.r, s * u.g, s * u.b);
}

RGB operator*(const RGB& u, const float s) {
  return RGB(s * u.r, s * u.g, s * u.b);
}

RGB operator/(const RGB& u, const float s) {
  return RGB(u.r / s, u.g / s, u.b / s);
}

bool operator<(const RGB& u, const RGB& v) {
  return (u.r < v.r ||
          (u.r == v.r && (u.g < v.g || (u.g == v.g && u.b < v.b))));
}

bool operator==(const RGB& u, const RGB& v) {
  return (u.r == v.r && u.g == v.g && u.b == v.b);
}

bool operator!=(const RGB& u, const RGB& v) { return !(u == v); }

std::ostream& operator<<(std::ostream& os, const RGB& rgb) {
  os << rgb.r << " " << rgb.g << " " << rgb.b;
  return os;
}

template <>
Image<RGB> imread(const std::string& fileName) {
  Image<RGB> empty;

  FILE* file = fopen(fileName.c_str(), "rb");

  if (!file) {
    return empty;
  }

  int width;
  int height;

  const unsigned char* data = stbi_load_from_file(file, &width, &height, 0, 3);

  fclose(file);

  if (data == NULL) {
    return empty;
  }
  if (width < 1 || height < 1) {
    return empty;
  }

  Image<RGB> image(width, height);

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      const int xy = x + y * width;

      image(x, y) = RGB(float(data[xy * 3 + 0]) / 255.0f,
                        float(data[xy * 3 + 1]) / 255.0f,
                        float(data[xy * 3 + 2]) / 255.0f);
    }

  stbi_image_free((void*)data);

  return image;
}

template <>
Image<float> imread(const std::string& fileName) {
  Image<RGB> rgbImage = imread<RGB>(fileName);
  if (rgbImage.width() < 1 || rgbImage.height() < 1) {
    return Image<float>();
  }

  Image<float> image(rgbImage.width(), rgbImage.height());

  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++) {
      RGB color = rgbImage(x, y);
      image(x, y) = (color.r + color.g + color.b) / 3.0f;
    }

  return image;
}

template <>
bool imwrite(const Image<unsigned char>& image, const std::string& fileName) {
  const int width = image.width();
  const int height = image.height();

  unsigned char* const data =
      new unsigned char[(size_t)width * (size_t)height * 3];

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      const int xy = x + y * width;

      const unsigned char color = image(x, y);

      data[xy * 3 + 0] = color;
      data[xy * 3 + 1] = color;
      data[xy * 3 + 2] = color;
    }

  const std::string extension = fileName.size() > 4
                                    ? fileName.substr(fileName.size() - 4, 4)
                                    : std::string();

  int ret = 0;

  if (extension == ".png") {
    ret = stbi_write_png(fileName.c_str(), width, height, 3, data, width * 3);
  } else if (extension == ".bmp") {
    ret = stbi_write_bmp(fileName.c_str(), width, height, 3, data);
  } else if (extension == ".tga") {
    ret = stbi_write_tga(fileName.c_str(), width, height, 3, data);
  }

  delete[] data;

  return (ret != 0);
}

template <>
unsigned char* memFile(const Image<unsigned char>& image, int* len) {
  if (image.width() < 1 || image.height() < 1) {
    return nullptr;
  }
  const int width = image.width();
  const int height = image.height();

  unsigned char* const data =
      new unsigned char[(size_t)width * (size_t)height * 3];

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      const int xy = x + y * width;
      unsigned char grey = image(x, y);
      data[xy * 3 + 0] = grey;
      data[xy * 3 + 1] = grey;
      data[xy * 3 + 2] = grey;
    }

  unsigned char* ret;
  ret = stbi_write_png_to_mem(data, width * 3, width, height, 3, len);

  delete[] data;

  return ret;
}

template <>
unsigned char* memFile(const Image<RGB>& image, int* len) {
  if (image.width() < 1 || image.height() < 1) {
    return nullptr;
  }
  const int width = image.width();
  const int height = image.height();

  unsigned char* const data =
      new unsigned char[(size_t)width * (size_t)height * 3];

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      const int xy = x + y * width;

      const RGB color = image(x, y);

      data[xy * 3 + 0] = color.r * 255.0f;
      data[xy * 3 + 1] = color.g * 255.0f;
      data[xy * 3 + 2] = color.b * 255.0f;
    }

  unsigned char* ret;
  ret = stbi_write_png_to_mem(data, width * 3, width, height, 3, len);

  delete[] data;

  return ret;
}

template <>
unsigned char* memFile(const Image<float>& image, int* len) {
  if (image.width() < 1 || image.height() < 1) {
    return nullptr;
  }

  Image<RGB> rgbImage(image.width(), image.height());

  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++) {
      const float grey = image(x, y);
      rgbImage(x, y) = RGB(grey, grey, grey);
    }

  return memFile(rgbImage, len);
}

template <>
bool imwrite(const Image<RGB>& image, const std::string& fileName) {
  const int width = image.width();
  const int height = image.height();

  unsigned char* const data =
      new unsigned char[(size_t)width * (size_t)height * 3];

  for (int y = 0; y < height; y++)
    for (int x = 0; x < width; x++) {
      const int xy = x + y * width;

      const RGB color = image(x, y);

      data[xy * 3 + 0] = color.r * 255.0f;
      data[xy * 3 + 1] = color.g * 255.0f;
      data[xy * 3 + 2] = color.b * 255.0f;
    }

  const std::string extension = fileName.size() > 4
                                    ? fileName.substr(fileName.size() - 4, 4)
                                    : std::string();

  int ret = 0;

  if (extension == ".png") {
    ret = stbi_write_png(fileName.c_str(), width, height, 3, data, width * 3);
  } else if (extension == ".bmp") {
    ret = stbi_write_bmp(fileName.c_str(), width, height, 3, data);
  } else if (extension == ".tga") {
    ret = stbi_write_tga(fileName.c_str(), width, height, 3, data);
  }

  delete[] data;

  return (ret != 0);
}

template <>
bool imwrite(const Image<float>& image, const std::string& fileName) {
  if (image.width() < 1 || image.height() < 1) {
    return false;
  }

  Image<RGB> rgbImage(image.width(), image.height());

  for (int y = 0; y < image.height(); y++)
    for (int x = 0; x < image.width(); x++) {
      const float grey = image(x, y);
      rgbImage(x, y) = RGB(grey, grey, grey);
    }

  return imwrite(rgbImage, fileName);
}
