#include "sbox/morphing1/morphing.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "base/logging.h"
#include "azer/render/render.h"

#include "morphing.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/morphing1/"
#define SHADER_NAME "morphing.afx"

namespace {
std::vector<MorphingEffect::Vertex> InitVertex(
    const sbox::SkinnedMesh::VertexVec& vec) {
  std::vector<MorphingEffect::Vertex> vertex;
  for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
    vertex.push_back(std::move(MorphingEffect::Vertex(iter->position,
                                                             iter->coordtex,
                                                             iter->normal)));
  }
  return vertex;
}
}  // namespace


void Morphing::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  MorphingEffect* effect = new MorphingEffect(
      shaders.GetShaderVec(), azer::RenderSystem::Current());
  effect_.reset(effect);

  azer::VertexBuffer::Options vbopt;
  vbopt.usage = azer::GraphicBuffer::kDynamic;
  vbopt.cpu_access = azer::kCPUWrite;
  azer::VertexDescPtr vertex_desc_ptr = effect->GetVertexDesc();
  for (uint32 i = 0; i < mesh_->groups().size(); ++i) {
    RenderGroup rgroup;
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];

    std::vector<MorphingEffect::Vertex> v
        = std::move(InitVertex(group.vertices));
    
    const int vertex_num = group.vertices.size();
    azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, vertex_num)); 
    memcpy(vdata->pointer(), (uint8*)&(v[0]),
           sizeof(MorphingEffect::Vertex) * v.size());
    rgroup.vb.reset(rs->CreateVertexBuffer(vbopt, vdata));

    azer::IndicesDataPtr idata_ptr(
        new azer::IndicesData(vertex_num, azer::IndicesData::kUint32,
                              azer::IndicesData::kMainMemory));
    memcpy(idata_ptr->pointer(), &(group.indices[0]),
           group.indices.size() * sizeof(uint32));
    rgroup.ib.reset(
        rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
    rgroup.mtrl_idx = group.mtrl_idx;
    rgroups_.push_back(rgroup);
  }
}

void Morphing::Render(azer::Renderer* renderer,
                      const azer::Matrix4& world,
                      const azer::Matrix4& pv) {
  MorphingEffect* effect = (MorphingEffect*)effect_.get();
  for (uint32 i = 0; i < rgroups_.size(); ++i) {
    const RenderGroup& rg = rgroups_[i];
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];
    DCHECK(group.bone != NULL);
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    sbox::SkinnedMesh::Material mtrl = mesh_->materials()[rg.mtrl_idx];
    effect->SetProjView(pv);
    effect->SetDiffuseTex(mtrl.tex);
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}
