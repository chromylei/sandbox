#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/mesh.h"

class SoftSkinnedMesh : public Mesh {
 public:
  SoftSkinnedMesh() {}
  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);
  
  const Skeleton& GetSkeleton() const { return skeleton_;}
  Skeleton& GetSkeleton() { return skeleton_;}
  void UpdateVertex(const azer::Matrix4& world);
 private:
  struct BoneWeight {
  };
  struct BonedMesh {
    azer::Matrix4* boneOffset;
    std::vector<Bone*> bone;
    std::vector<float> weight;
  };

  typedef std::vector<azer::Matrix4* > MeshOffsetMat;
  azer::Matrix4 CalcPosition(const azer::Matrix4& pv, const BoneWeights& weights,
                             const MeshOffsetMat& offsets);
  void LoadBoneWeights(const aiMesh* paiMesh, BoneWeightsVec* vec,
                       MeshOffsetMat* offset);
  std::vector<BoneWeightsVec> group_weights_;
  std::vector<MeshOffsetMat> group_offset_;
  Skeleton skeleton_;
  DISALLOW_COPY_AND_ASSIGN(SoftSkinnedMesh);
};

