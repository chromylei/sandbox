#pragma once

#include "azer/render/render.h"
#include "sbox/base/mesh.h"
#include "base/basictypes.h"

class CoordMesh {
 public:
  CoordMesh() {}

  void Init(azer::RenderSystem* rs);
  void Render(azer::Renderer* renderer, const azer::Matrix4& world,
              const azer::Matrix4& vp);
 private:
  Mesh mesh_;
  std::unique_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(CoordMesh);
};
