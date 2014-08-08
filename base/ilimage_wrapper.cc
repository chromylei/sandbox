#include "sbox/base/ilimage_wrapper.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
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

  const ::base::FilePath::StringType ext = path.Extension();
  int type;
  if (ext == FILE_PATH_LITERAL(".bmp")) {
    type = IL_BMP;
  } else if (ext == FILE_PATH_LITERAL(".dds")) {
    type = IL_DDS;
  } else {
    NOTREACHED();
  }

  return Load(data.get(), filesize, type);
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

bool ilImageWrapper::Load(uint8* data, int size, int type) {
  ilGenImages(1, &image_id_);
  ilBindImage(image_id_);

  ilLoadL(type, data, size);
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

uint8* ilImageWrapper::GetDataPtr() {
  DCHECK(image_id_ != (ILuint)-1);
  ilBindImage(image_id_);
  return (uint8*)ilGetData();
}

uint32 ilImageWrapper::GetData(int x, int y) {
  DCHECK(image_id_ != (ILuint)-1);
  ilBindImage(image_id_);

  ILubyte* data = ilGetData();
  
  switch (bytes_per_pixel_) {
    case 8: {
      uint8* ptr = data + y * width_ + x;
      return *ptr;
    }
    case 16: {
      uint8* ptr = data + (y * width_ + x) * 2;
      // 5, 6, 5
      return ptr[1] << 8 | ptr[0];
    }
    case 24: {
      uint8* ptr = data + (y * width_ + x) * 3;
      uint32 value = ptr[2] << 24;
      value |= ptr[1] << 16;
      value |= ptr[0] << 8;
      return  value;
    }
    case 32: {
      uint8* ptr = data + (y * width_ + x) * 3;
      uint32 value = ptr[0] << 24;
      value |= ptr[1] << 16;
      value |= ptr[2] << 8;
      value |= ptr[3];
      return  value;
    }
    default:
      NOTREACHED();
      return 0;
  }
}
}  // namespace sbox
