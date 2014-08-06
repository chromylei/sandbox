#pragma once

#include <vector>
#include <string>

#include "base/basictypes.h"
#include "azer/render/render.h"

class TerrainTexMap {
 public:
  TerrainTexMap() : kCellWidth(100.0f) {}
  bool GetTexture(float x, float z, azer::TexturePtr* ptrarr);
  bool GetTexture(float x, float z, int* index);
  void SetCell(int x, int z, azer::TexturePtr ptr) {
    map_[x][z] = ptr;
  }

  azer::Vector2 CalcTexViewpos(float x, float z);
 private:
  float MapCoordX(float x);
  float MapCoordZ(float z);
  azer::TexturePtr map_[11][11];
  const float kCellWidth;
  DISALLOW_COPY_AND_ASSIGN(TerrainTexMap);
};

/**
 *
 */
class TerrainTile {
 public:
  TerrainTile(int width, int height)
      : kWidth(width)
      , kHeight(height) {
  }

  bool Init(azer::RenderSystem* rs);
  void SetTexture(int index, azer::TexturePtr ptr);

  void OnUpdate(float x, float z, azer::Renderer* render);
 private:
  const int kWidth;
  const int kHeight;

  azer::TexturePtr heightmap_;
  azer::VertexBufferPtr vb_;
  azer::EffectPtr effect_;
  std::vector<azer::TexturePtr> texvec_;
  std::unique_ptr<azer::Renderer> heightmap_renderer_;
  DISALLOW_COPY_AND_ASSIGN(TerrainTile);
};
