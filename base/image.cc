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
  DCHECK(image_ != NULL);
  return image_->GetData(x, y);
}

bool Image::Load(const ::base::FilePath& path) {
  DCHECK(image_ == NULL);
  image_ = new ilImageWrapper;
  return image_->Load(path);
}

int Image::height() const {
  DCHECK(image_ != NULL);
  return image_->height();
}
int Image::width() const {
  DCHECK(image_ != NULL);
  return image_->width();
}

azer::Vector4 SampleImage(float u, float v, Image* image) {
  int x = u * image->width();
  int y = v * image->height();
  int32 rgba = image->GetData(x, y);
  return azer::Vector4((float)((rgba & 0xFF000000) >> 24) / 255.0f,
                       (float)((rgba & 0x00FF0000) >> 16) / 255.0f,
                       (float)((rgba & 0x0000FF00) >> 8) / 255.0f,
                       (float)(rgba & 0x000000FF) / 255.0f);
}
}  // namespace sbox
