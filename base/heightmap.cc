#include "sbox/base/heightmap.h"

#include "base/file_util.h"

bool HeightMapLoad(base::FilePath filename, HeightMapInfo *info) {
  int64 filesize = 0;
  if (!base::GetFileSize(filename, &filesize)) {
    return false;
  }

  std::unique_ptr<uint8[]> data(new uint8[filesize]);
  int ret = base::ReadFile(filename, (char*)data.get(), filesize);
  DCHECK_EQ(ret, filesize);


  BITMAPFILEHEADER bitmapFileHeader;
  BITMAPINFOHEADER bitmapInfoHeader;
  uint8* current = data.get();
  memcpy(&bitmapFileHeader, current, sizeof(bitmapFileHeader));
  current += sizeof(bitmapFileHeader);
  memcpy(&bitmapInfoHeader, current, sizeof(bitmapInfoHeader));
  current += sizeof(bitmapInfoHeader);

  info->width = bitmapInfoHeader.biWidth;
  info->height = bitmapInfoHeader.biHeight;
  const int datasize = info->height * info->width * 3;
  current += bitmapFileHeader.bfOffBits;
  uint8* imagedata = current;
  info->map = new azer::vec3[info->height * info->width];

  float factor = 10.0f;
  int k = 0;
  for (int j = 0; j < info->height; ++j) {
    for (int i = 0; i < info->width; ++i) {
      int height = imagedata[k];
      const int idx = j * info->width + i;
      info->map[idx] = azer::vec3((float)i, (float)height / factor, (float)j);
      k+=3;
    }
  }
  return true;
}
