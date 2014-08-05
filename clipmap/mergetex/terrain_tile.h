#pragma once

#include <vector>
#include <string>

#include "base/basictypes.h"
#include "azer/render/render.h"

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
