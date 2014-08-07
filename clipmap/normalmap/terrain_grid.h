#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"

class GridTileEffect;

class GridTile {
 public:
  GridTile(float cell, int cellnum, float x = 0.0f, float z = 0.0f);

  bool Init(azer::RenderSystem* rs);
  void Render(azer::Renderer* renderer, const azer::Camera& camera);
 private:
  void CreateVertex(azer::RenderSystem* rs, GridTileEffect* effect);

  std::unique_ptr<azer::Effect> effect_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  azer::TexturePtr texptr_;
  const float kCellWidth;
  const float orgx_, orgz_;
  const int kCellNum;
  DISALLOW_COPY_AND_ASSIGN(GridTile);
};
