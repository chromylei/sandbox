#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"


struct aiMesh;

class Mesh {
 public:
  Mesh() {}

  struct Material {
    azer::TexturePtr tex;
  };

  struct Vertex {
    azer::Vector4 position;
    azer::Vector2 coordtex;
    azer::Vector4 normal;
    Vertex(const azer::Vector4 p0, const azer::Vector2 p1, const azer::Vector4 p2)
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
 protected:
  std::vector<Group> groups_;
  std::vector<Material> materials_;
  std::vector<RenderGroup> rgroups_;
  DISALLOW_COPY_AND_ASSIGN(Mesh);
};

void LoadVertex(const aiMesh* paiMesh, Mesh::Group* group);
bool LoadMesh(const ::base::FilePath& filepath, Mesh* mesh,
              azer::RenderSystem* rs);
