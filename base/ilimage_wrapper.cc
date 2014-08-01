#include "sbox/base/ilimage_wrapper.h"
#include "base/file_util.h"
#include "base/logging.h"

#include "IL/ilu.h"

namespace sbox {
bool ilImageWrapper::Load(const ::base::FilePath& path) {
  int64 filesize = 0;
  if (!::base::GetFileSize(path, &filesize)) {
    return false;
  }

  std::unique_ptr<uint8[]> data(new uint8[filesize]);
  if (filesize != ::base::ReadFile(path, (char*)data.get(), (int32)filesize)) {
    return false;
  }

  return Load(data.get(), filesize);
}

bool ilImageWrapper::Create(int width, int height) {
  data_.reset(new uint8[width * height * 4]);
  ilGenImages(1, &image_id_);
  ilBindImage(image_id_);
  ilTexImage(width, height,
             1,  // OpenIL supports 3d textures!  but we don't want it to be 3d.  so
    // we just set this to be 1
             4,  // 4 channels:  one for R , one for G, one for B, one for A
             IL_RGBA,  
             IL_UNSIGNED_BYTE, // the type of data the imData array contains
             data_.get());
  ILenum ilerr = ilGetError();
  if (ilerr != IL_NO_ERROR) {
    LOG(ERROR) << (const char*)iluErrorString(ilerr);
    return false;
  }

  width_ = width;
  height_ = height_;
  bytes_per_pixel_ = 4;
  return true;
}

bool ilImageWrapper::Load(uint8* data, int size) {
  ilGenImages(1, &image_id_);
  ilBindImage(image_id_);

  ilLoadL(IL_BMP, data, size);
  ILenum ilerr = ilGetError();
  if (ilerr != IL_NO_ERROR) {
    LOG(ERROR) << (const char*)iluErrorString(ilerr);
    return false;
  }

  width_ = (int)ilGetInteger(IL_IMAGE_WIDTH);
  height_ = (int)ilGetInteger(IL_IMAGE_HEIGHT);
  bytes_per_pixel_ = (int)ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
  return true;
}

int ilImageWrapper::GetData(int x, int y) {
  DCHECK(image_id_ != (ILuint)-1);
  ilBindImage(image_id_);

  ILubyte* data = ilGetData();
  
  switch (bytes_per_pixel_) {
    case 1: {
      uint8* ptr = (uint8*)(data) + (y * width_ + x);
      return *ptr;
    }
    case 2: {
      uint16* ptr = (uint16*)(data) + (y * width_ + x);
      return *ptr;
    }
    case 4: {
      uint32* ptr = (uint32*)(data) + (y * width_ + x);
      return *ptr;
    }
    default:
      NOTREACHED();
      return 0;
  }
}
}  // namespace sbox
