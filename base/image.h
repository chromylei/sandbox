#pragma once

#include <memory>
#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "azer/render/render.h"

namespace sbox {
class ilImageWrapper;
class Image {
 public:
  Image();
  ~Image();

  int height() const;
  int width() const;

  uint32 GetData(int x, int y);
  bool Load(const ::base::FilePath& path);
 private:
  ilImageWrapper* image_;
  DISALLOW_COPY_AND_ASSIGN(Image);
};

azer::Vector4 SampleImage(float u, float v, Image* image);
}
