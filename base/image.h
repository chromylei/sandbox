#pragam once

#include <memory>
#include "base/basictypes.h"
#include "base/files/file_path.h"

namespace sbox {
class Image {
 public:
  Image();
  int height() const { return height_;}
  int width() const { return width_;}

  uint32 GetData(int x, int y);
  bool Load(const ::base::FilePath& path);
 private:
  std::unique_ptr<uint8[]> data_;
  DISALLOW_COPY_AND_ASSIGN(Image);
};
}
