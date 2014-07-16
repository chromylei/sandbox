#pragma once

#include "azer/math/math.h"
#include "base/files/file_path.h"

struct HeightMapInfo {
  int width;
  int height;
  azer::vec3* map;
};

bool HeightMapLoad(base::FilePath filename, HeightMapInfo *info);
