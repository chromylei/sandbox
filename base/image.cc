#include "sbox/base/image.h"

#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"
#include "sbox/base/ilimage_wrapper.h"

namespace sbox {

Image::Image()
    : image_(NULL) {
}

Image::~Image() {
  if (image_) {
    delete image_;
  }
}

uint32 Image::GetData(int x, int y) {
  return 0;
}

bool Image::Load(const ::base::FilePath& path) {
  DCHECK(image_ == NULL);
  image_ = new ilImageWrapper;
  return image_->Load(path);
}
}  // namespace sbox
