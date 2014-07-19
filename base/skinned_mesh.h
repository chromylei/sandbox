#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/mesh.h"

struct BoneAttached {
  int index;
  float weight;

  BoneAttached()
      : index(-1)
      , weight(0.0f) {
  }
};
typedef std::vector<BoneAttached> BoneWeights;
typedef std::vector<BoneWeights> BoneWeightsVec;

class SkinnedMesh : public Mesh {
 public:
  SkinnedMesh() {}
  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);

  const Skeleton& GetSkeleton() const { return skeleton_;}
  Skeleton& GetSkeleton() { return skeleton_;}
 private:
  BoneWeightsVec group_weights_;
  Skeleton skeleton_;
  DISALLOW_COPY_AND_ASSIGN(SkinnedMesh);
};
