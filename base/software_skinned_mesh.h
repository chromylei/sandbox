#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/animation.h"
#include "sbox/base/mesh.h"
#include "sbox/base/skinned_mesh.h"

struct aiMesh;
struct aiScene;

struct BoneAttached {
  int index;
  float weight;

  BoneAttached()
      : index(-1)
      , weight(0.0f) {
  }

  BoneAttached(int idx, float w)
      : index(idx)
      , weight(w) {
  }
};


typedef std::vector<BoneAttached> BoneWeights;
typedef std::vector<BoneWeights> BoneWeightsVec;


typedef std::vector<azer::Matrix4> MeshOffsetMat;
class SoftwareSkinnedMesh {
 public:
  SoftwareSkinnedMesh(sbox::SkinnedMesh* skinned)
      : mesh_(skinned) {
  }

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  void Init(azer::RenderSystem* rs);
  void UpdateVertex(const azer::Matrix4& world);
  void Render(azer::Renderer* renderer, const azer::Matrix4& world,
              const azer::Matrix4& pv);
 private:
  azer::Matrix4 CalcPosition(const azer::Matrix4& world, const BoneWeights& weights,
                             const MeshOffsetMat& offsets);
  sbox::SkinnedMesh* mesh_;
  std::vector<BoneWeightsVec> group_weights_;
  std::vector<MeshOffsetMat> group_offset_;
  std::vector<azer::Matrix4> bone_mat_;
  std::vector<azer::Matrix4> temp_mat_;
  std::vector<RenderGroup> rgroups_;
  std::shared_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(SoftwareSkinnedMesh);
};
