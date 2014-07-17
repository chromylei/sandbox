#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include <vector>

class Mesh {
 public:
  Mesh() {}

  struct Material {
    azer::TexturePtr tex;
  };

  struct Vertex {
    azer::Vector3 position;
    azer::Vector2 coordtex;
    azer::Vector3 normal;
    Vertex(const azer::Vector3 p0, const azer::Vector2 p1, const azer::Vector3 p2)
      : position(p0)
      , coordtex(p1)
      , normal(p2)
      {}
  };

  struct Group {
    std::vector<Vertex> vertices;
    std::vector<int32> indices;
    int mtrl_idx;
  };

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  const std::vector<Group>& groups() const { return groups_;}
  std::vector<Group>* mutable_groups() { return &groups_;}

  const std::vector<RenderGroup>& rgroups() const { return rgroups_;}

  const std::vector<Material>& materials() const { return materials_;}
  std::vector<Material>* mutable_materials() { return &materials_;}

  void Init(azer::RenderSystem* rs);
 private:
  std::vector<Group> groups_;
  std::vector<Material> materials_;
  std::vector<RenderGroup> rgroups_;
  DISALLOW_COPY_AND_ASSIGN(Mesh);
};

struct BoneAttached {
  int index;
  float weight;

  BoneAttached()
      : index(-1)
      , weight(0.0f) {
  }
};
typedef std::vector<BoneAttached> BoneWeigthVec;

class SkinnedMesh : public Mesh {
 public:
  SkinnedMesh() {}
  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);

  const Skeleton& GetSkeleton() const { return skeleton_;}
 private:
  Skeleton skeleton_;
  std::vector<BoneWeigthVec> group_weights_;
  DISALLOW_COPY_AND_ASSIGN(SkinnedMesh);
};
