#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/mesh.h"

struct aiMesh;

class HardwareSkinnedMesh {
 public:
  HardwareSkinnedMesh() {}
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

  struct Group {
    std::vector<Vertex> vertices;
    std::vector<int32> indices;
    std::vector<azer::Matrix4> offset;
    int mtrl_idx;
  };

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  void Init(azer::RenderSystem* rs);
  bool Load(const ::base::FilePath& filepath, azer::RenderSystem* rs);
 private:
  void LoadVertex(const aiMesh* paiMesh, Group* group);
  void LoadBoneWeights(const aiMesh* paiMesh, std::vector<Vertex>* vertex,
                       std::vector<azer::Matrix4>* offsets);
  std::vector<Group> groups_;
  std::vector<Material> materials_;
  std::vector<RenderGroup> rgroups_;
  Skeleton skeleton_;
  DISALLOW_COPY_AND_ASSIGN(HardwareSkinnedMesh);
};
