#include "sbox/base/software_skinned_mesh.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "base/logging.h"
#include "azer/render/render.h"

#include "software_skinned.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "softdware_skinned.afx"


void SoftwareSkinnedMesh::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  SoftwareSkinnedEffect* effect = new SoftwareSkinnedEffect(
      shaders.GetShaderVec(), azer::RenderSystem::Current());
  effect_.reset(effect);

  azer::VertexBuffer::Options vbopt;
  vbopt.usage = azer::GraphicBuffer::kDynamic;
  vbopt.cpu_access = azer::kCPUWrite;
  azer::VertexDescPtr vertex_desc_ptr = effect->GetVertexDesc();

  using SoftwareSkinnedEffect::Vertex;
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


  for (uint32 i = 0; i < mesh_->groups().size(); ++i) {
    BoneWeightsVec weights;
    MeshOffsetMat offsets;

    offsets.resize(mesh_->GetSkeleton().GetBoneNum());
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];
    for (auto iter = group.offset.begin(); iter != group.offset.end(); ++iter) {
      offsets[iter->first] = iter->second;
    }

    group_weights_.push_back(weights);
    group_offset_.push_back(offsets);
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
                                      iter->normal)));
  }
  return vertex;
}

azer::Matrix4 SoftSkinnedMesh::CalcPosition(const azer::Matrix4& world,
                                            const BoneWeights& weights,
                                            const MeshOffsetMat& offsets) {
  azer::Matrix4 matrix = azer::Matrix4::kZero;
  for (auto iter = weights.begin(); iter != weights.end(); ++iter) {
    Bone* bone = skeleton_.GetBone(iter->index);
    const azer::Matrix4* offset = offsets[iter->index];
    if (offset == NULL) {
      matrix += bone->combined() * iter->weight;
    } else {
      matrix += std::move(bone->combined() * *offset) * iter->weight;
    }
  }
  azer::Matrix4 ret = std::move(world * matrix);
  return ret;
}

void SoftSkinnedMesh::UpdateVertex(const azer::Matrix4& world) {
  for (size_t i = 0; i < rgroups_.size(); ++i) {
    azer::LockDataPtr dataptr(rgroups_[i].vb->map(azer::kWriteDiscard));
    Vertex* vertex = (Vertex*)dataptr->data_ptr();
    sbos::SkinnedMesh& Group& group = mesh_->groups_[i];
    BoneWeightsVec weights = group_weights_[i];
    for (int j = 0; j < group.vertices.size(); ++j) {
      azer::Matrix4 new_world = std::move(CalcPosition(world, weights[j],
                                                       group_offset_[i]));
      vertex[j].position =  std::move(new_world* group.vertices[j].position);
      vertex[j].coordtex = group.vertices[j].coordtex;
      vertex[j].normal = new_world * group.vertices[j].normal;
    }
    rgroups_[i].vb->unmap();
  }
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
  SoftwareSkinnedEffect* effect = (SoftwareSkinnedEffect*)effect_.get();
  for (uint32 i = 0; i < rgroups_.size(); ++i) {
    const RenderGroup& rg = rgroups_[i];
    const sbox::SkinnedMesh::Group& group = mesh_->groups()[i];
    DCHECK(group.bone != NULL);
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    sbox::SkinnedMesh::Material mtrl = mesh_->materials()[rg.mtrl_idx];
    effect->SetProjView(pv);
    effect->SetWorld(world);
    effect->SetDiffuseTex(mtrl.tex);
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}
