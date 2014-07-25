#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/animation.h"

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

namespace sbox {
class SkinnedMesh {
 public:
  SkinnedMesh() : anim_set_(&skeleton_) {}

  struct Material {
    azer::TexturePtr tex;
  };

  struct Vertex {
    azer::Vector4 position;
    azer::Vector2 coordtex;
    azer::Vector4 normal;
    std::vector<int> index;
    std::vector<float> weights;
    Vertex(const azer::Vector4 p0, const azer::Vector2 p1, const azer::Vector4 p2)
      : position(p0)
      , coordtex(p1)
      , normal(p2) {
    }
  };

  typedef std::vector<Vertex> VertexVec;

  typedef std::vector<std::pair<int, azer::Matrix4> > OffsetType;
  struct Group {
    VertexVec vertices;
    std::vector<int32> indices;
    OffsetType offset;
    int mtrl_idx;
    Bone* bone;

    Group() : bone(NULL) {
    }
  };

  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);

  Skeleton& GetSkeleton() { return skeleton_;}

  const std::vector<Group>& groups() const { return groups_;}
  std::vector<Group>* mutable_groups() { return &groups_;}

  const std::vector<Material>& materials() const { return materials_;}
  std::vector<Material>* mutable_materials() { return &materials_;}

  const AnimationSet& GetAnimationSet() const { return anim_set_;}
 private:
  void LoadVertex(const aiMesh* paiMesh, Group* group);
  void LoadBoneWeights(const aiMesh* paiMesh, std::vector<Vertex>* vertex,
                       OffsetType* offsets);
  void LoadMaterial(const ::base::FilePath& filepath, azer::RenderSystem* rs,
                    const aiScene* scene);
  void LoadScene(const aiScene* scene);
  void LoadNode(const aiNode* node);
  std::vector<Group> groups_;
  std::vector<Material> materials_;
  Skeleton skeleton_;
  AnimationSet anim_set_;
  DISALLOW_COPY_AND_ASSIGN(SkinnedMesh);
};

}  // namespace sbox
