#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"

class LineRenderer {
 public:
  LineRenderer();
  void Init(azer::RenderSystem* rs);
  void Render(azer::Renderer* renderer, const azer::Matrix4& pv);

  void SetPosition(const azer::Vector3& p1, const azer::Vector3& p2);

  static const azer::VertexDesc::Desc kVertexDesc[];
 private:
  azer::VertexBufferPtr vb_;
  std::unique_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(LineRenderer);
};
