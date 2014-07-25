#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"

struct aiScene;

class ModelLoader {
 public:
  struct BaseVertex {
    azer::Vector4 position;
    azer::Vector2 texcoord;    
    azer::Vector4 normal;

    BaseVertex(const Vertex4& p1, const Vertex2& p2, const Vertex4& p3)
        : position(p1)
        , texcoord(p2)
        , normal(p3) {
    }
  };

  struct Material {
  };

  struct Bone {
    azer::Matrix4 offset;
    std::vector<double> weight;
    std::vector<int> index;
  };

  class Delegate {
   public:
    virtual ~Delegate() {}
    virtual void OnMesh(int meshid, int vertex_num, int indices_num) = 0;
    virtual void OnVertex(int meshid, int vid, const BaseVertex& vertex,
                          const Bone& bone) = 0;
    virtual void OnIndices(int meshid, int faceid, int p1, int p2, int p3) = 0;
    virtual void OnMaterial(int material, Material* material) = 0;
    virtual void OnMeshEnd(int meshid) = 0;
  };

  ModelLoader(Delegate* delegate, RenderSystem* rs)
      : delegate_(delegate)
      , scene_(NULL)
      , render_system_(rs) {
  }

  bool Load(const std::string& path);
  const aiScene* GetScene() { return scene_;}
 private:
  Delegate* delegate_;
  const aiScene* scene_;
  RenderSystem* render_system_;
  DISALLOW_COPY_AND_ASSIGN(ModelLoader);
};
