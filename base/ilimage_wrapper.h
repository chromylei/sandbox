#pragma once

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "IL/il.h"

namespace sbox {
class ilImageWrapper {
 public:
  ilImageWrapper()
      : width_(-1)
      , height_(-1)
      , image_id_((ILuint)-1) {
  }

  ~ilImageWrapper() {
    if (image_id_ != (ILuint)-1) {
      ilDeleteImages(1, &image_id_);
    }
  }

  bool Create(int width, int height);
  bool Load(const ::base::FilePath& path);
  bool Load(uint8* data, int size);
  int GetData(int x, int y);

  int height() const { return height_;}
  int width() const { return width_;}
 private:
  ILuint image_id_;
  int width_, height_;
  int bytes_per_pixel_;
  std::unique_ptr<uint8[]> data_;
  DISALLOW_COPY_AND_ASSIGN(ilImageWrapper);
};

}  // namespace sbox
