#include "sbox/base/util.h"

#include "line.afx.h"

const azer::VertexDesc::Desc LineRenderer::kVertexDesc[] = {
  {"POSITION", 0, azer::kVec3},
};

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "line.afx"

LineRenderer::LineRenderer() {
}

void LineRenderer::Init(azer::RenderSystem* rs) {
  azer::VertexDescPtr vertex_desc_ptr(
      new azer::VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, 2));
  azer::Vector3* ptr = (azer::Vector3*)vdata->pointer();
  ptr[0] = azer::Vector3(0.0f, 0.0f, 0.0f);
  ptr[1] = azer::Vector3(1.0f, 1.0f, 1.0f);
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new LineEffect(shaders.GetShaderVec(), rs));
}

void LineRenderer::SetPosition(const azer::Vector3& p1, const azer::Vector3& p2) {
  pos1_ = azer::Vector4(p1, 1.0f);
  pos2_ = azer::Vector4(p2, 1.0f);
}

void LineRenderer::Render(azer::Renderer* renderer, const azer::Matrix4& pv) {
  LineEffect* effect = (LineEffect*)effect_.get();
  effect->SetPVW(pv);
  effect->SetPoint1(pos1_);
  effect->SetPoint2(pos2_);
  effect->SetDiffuse(azer::Vector4(0.9f, 0.9f, 0.9f, 1.0f));
  effect->Use(renderer);
  renderer->Draw(vb_.get(), azer::kLineList, 2, 0);
}
