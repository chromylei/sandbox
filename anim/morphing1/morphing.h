#pragma once

#include "base/basictypes.h"
#include "sbox/base/base.h"


class MorphingMesh {
 public:
  MorphingMesh(sbox::SkinnedMesh* skinned)
      : mesh_(skinned) {
  }

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  void Init(azer::RenderSystem* rs);
  void Render(azer::Renderer* renderer, const azer::Matrix4& world,
              const azer::Matrix4& pv);
 private:
  sbox::SkinnedMesh* mesh_;
  std::vector<RenderGroup> rgroups_;
  std::shared_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(MorphingMesh);
};
