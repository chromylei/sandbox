#include "sbox/base/software_skinned_mesh.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "base/logging.h"
#include "azer/render/render.h"

#include "software_skinned.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "haredware_skinned.afx"

SoftwareSkinnedMesh::Vertex::Vertex(const azer::Vector4 p0, const azer::Vector2 p1,
                                    const azer::Vector4 p2,
                                    const std::vector<int>& p3,
                                    const std::vector<float>& p4)
    : position(p0)
    , coordtex(p1)
    , normal(p2) {
  DCHECK_EQ(p3.size(), p4.size());
  for (int i = 0; i < 4; ++i) {
    if (i < p3.size()) {
      index[i] = p3[i];
      weights[i] = p4[i];
    } else {
      index[i] = -1;
      weights[i] = 0.0f;
    }
  }
}

void SoftwareSkinnedMesh::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  HaredwareSkinnedMeshEffect* effect = new HaredwareSkinnedMeshEffect(
      shaders.GetShaderVec(), azer::RenderSystem::Current());
  effect_.reset(effect);

  azer::VertexBuffer::Options vbopt;
  vbopt.usage = azer::GraphicBuffer::kDynamic;
  vbopt.cpu_access = azer::kCPUWrite;
  azer::VertexDescPtr vertex_desc_ptr = effect->GetVertexDesc();
  for (uint32 i = 0; i < mesh_->groups().size(); ++i) {
    RenderGroup rgroup;
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];

    std::vector<Vertex> v = std::move(InitVertex(group.vertices));
    
    const int vertex_num = group.vertices.size();
    azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, vertex_num)); 
    memcpy(vdata->pointer(), (uint8*)&(v[0]), sizeof(Vertex) * v.size());
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

  mesh_->GetSkeleton().PrepareRender(rs);
  mesh_->GetSkeleton().UpdateHierarchy(azer::Matrix4::kIdentity);
  bone_mat_.resize(mesh_->GetSkeleton().GetBoneNum());
  temp_mat_.resize(mesh_->GetSkeleton().GetBoneNum());
}


std::vector<SoftwareSkinnedMesh::Vertex> SoftwareSkinnedMesh::InitVertex(
    const sbox::SkinnedMesh::VertexVec& vec) {
  std::vector<Vertex> vertex;
  for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
    vertex.push_back(std::move(Vertex(iter->position,
                                      iter->coordtex,
                                      iter->normal,
                                      iter->index,
                                      iter->weights)));
  }
  return vertex;
}

void SoftwareSkinnedMesh::Update(const std::string& anname, double t) {
  const Animation* anim = mesh_->GetAnimationSet().GetAnimation(anname);
  mesh_->GetSkeleton().UpdateHierarchy(t, *anim, azer::Matrix4::kIdentity);
  memcpy(&bone_mat_[0], &(mesh_->GetSkeleton().GetBoneMat()[0]),
         sizeof(azer::Matrix4) * bone_mat_.size());
}

void SoftwareSkinnedMesh::Update(double t) {
  mesh_->GetSkeleton().UpdateHierarchy(azer::Matrix4::kIdentity);
  memcpy(&bone_mat_[0], &(mesh_->GetSkeleton().GetBoneMat()[0]),
         sizeof(azer::Matrix4) * bone_mat_.size());
}

void SoftwareSkinnedMesh::Render(azer::Renderer* renderer,
                                 const azer::Matrix4& world,
                                 const azer::Matrix4& pv) {
  HaredwareSkinnedMeshEffect* effect = (HaredwareSkinnedMeshEffect*)effect_.get();
  for (uint32 i = 0; i < rgroups_.size(); ++i) {
    const RenderGroup& rg = rgroups_[i];
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];
    DCHECK(group.bone != NULL);
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    sbox::SkinnedMesh::Material mtrl = mesh_->materials()[rg.mtrl_idx];
    azer::Matrix4 w = std::move(world * group.bone->combined());
    
    memcpy(&temp_mat_[0], &(bone_mat_[0]), sizeof(azer::Matrix4) * bone_mat_.size());
    for (auto iter = group.offset.begin(); iter != group.offset.end(); ++iter) {
      temp_mat_[iter->first] = bone_mat_[iter->first] * iter->second;
    }

    effect->SetBones((azer::Matrix4*)(&temp_mat_[0]), temp_mat_.size());
    effect->SetProjView(pv);
    effect->SetWorld(w);
    effect->SetDiffuseTex(mtrl.tex);
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}
