#include "sbox/base/coord.h"
#include "base/files/file_path.h"

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "coord.afx"
#include "coord.afx.h"
using base::FilePath;

#define kMeshPath "sbox/res/coord/coord.obj"

void CoordMesh::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));

  effect_.reset(new CoordEffect(shaders.GetShaderVec(), rs));
  LoadMesh(::base::FilePath(::base::UTF8ToWide(kMeshPath)), &mesh_, rs);
}

void CoordMesh::Render(azer::Renderer* renderer, const azer::Matrix4& world,
                       const azer::Matrix4& pv) {
  CoordEffect::PointLight light;
  light.direction = azer::Vector4(0.5f, -0.5f, 0.0f, 1.0f);
  light.diffuse = azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f);
  light.ambient = azer::Vector4(0.2f, 0.2f, 0.2f, 1.0f);
  CoordEffect* effect = (CoordEffect*)effect_.get();
  azer::Matrix4 pvw = std::move(pv * world);
  effect->SetPVW(pvw);
  effect->SetWorld(world);
  effect->SetPointLight(light);
  for (uint32 i = 0; i < mesh_.rgroups().size(); ++i) {
    Mesh::RenderGroup rg = mesh_.rgroups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    if (rg.mtrl_idx >= 0) {
      Mesh::Material mtrl = mesh_.materials()[rg.mtrl_idx];
      effect->SetDiffuseTex(mtrl.tex);
      effect->SetAmbientTex(mtrl.tex);
    }
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}
