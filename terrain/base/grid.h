#pragma once

#include <string>
#include <vector>

#include "base/files/file_path.h"
#include "azer/render/render.h"
#include "sbox/terrain/base/heightmap.h"

class Grid {
 public:
  Grid() {}

  struct Vertex {
    azer::Vector3 position;
    azer::Vector2 tex;
    azer::Vector3 normal;
  };

  void Init(const base::FilePath& filepath);
  const std::vector<Vertex>& vertices() { return vertices_;}
  const std::vector<int32>& indices() { return indices_;}
 private:
  void InitVertex();
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::vector<Vertex> vertices_;
  std::vector<int32> indices_;
  HeightMapInfo height_map_;
};

typedef std::shared_ptr<Grid> GridPtr;
