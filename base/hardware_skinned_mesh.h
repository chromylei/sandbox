#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/animation.h"
#include "sbox/base/mesh.h"

struct aiMesh;
struct aiScene;

class HardwareSkinnedMesh {
 public:
  HardwareSkinnedMesh()
      : anim_set_(&skeleton_) {
  }

  struct Material {
    azer::TexturePtr tex;
  };

  struct Vertex {
    azer::Vector4 position;
    azer::Vector2 coordtex;
    azer::Vector4 normal;
    azer::Vector4 weights;
    int index[4];
    Vertex(const azer::Vector4 p0, const azer::Vector2 p1, const azer::Vector4 p2)
      : position(p0)
      , coordtex(p1)
      , normal(p2) {
      for (int i = 0; i < arraysize(index); ++i) {
        index[i] = -1;
      }
    }
  };

  typedef std::vector<std::pair<int, azer::Matrix4> > OffsetType;
  struct Group {
    std::vector<Vertex> vertices;
    std::vector<int32> indices;
    OffsetType offset;
    int mtrl_idx;
    Bone* bone;

    Group() : bone(NULL) {
    }
  };

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  void Init(azer::RenderSystem* rs);
  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);

  Skeleton& GetSkeleton() { return skeleton_;}

  const std::vector<Group>& groups() const { return groups_;}
  std::vector<Group>* mutable_groups() { return &groups_;}

  const std::vector<RenderGroup>& rgroups() const { return rgroups_;}

  const std::vector<Material>& materials() const { return materials_;}
  std::vector<Material>* mutable_materials() { return &materials_;}
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
  std::vector<RenderGroup> rgroups_;
  Skeleton skeleton_;
  AnimationSet anim_set_;
  DISALLOW_COPY_AND_ASSIGN(HardwareSkinnedMesh);
};
