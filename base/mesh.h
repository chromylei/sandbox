#pragma once

#include "base/basictypes.h"
#include "azer/render/render.h"
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

  const std::vector<Group>& groups() const { return groups_;}
  std::vector<Group>* mutable_groups() { return &groups_;}

  const std::vector<Material>& materials() const { return materials_;}
  std::vector<Material>* mutable_materials() { return &materials_;}

  void Init(azer::RenderSystem* rs);
  void Render(azer::Effect* effect, azer::Renderer* renderer);
 private:
  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };
  std::vector<Group> groups_;
  std::vector<Material> materials_;
  std::vector<RenderGroup> rgroups_;
  DISALLOW_COPY_AND_ASSIGN(Mesh);
};
