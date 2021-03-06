#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "azer/render/render.h"
#include "sbox/base/skeleton.h"
#include "sbox/base/animation.h"
#include "sbox/base/mesh.h"
#include "sbox/base/skinned_mesh.h"

struct aiMesh;
struct aiScene;

class HardwareSkinnedMesh {
 public:
  HardwareSkinnedMesh(sbox::SkinnedMesh* skinned)
      : mesh_(skinned) {
  }

  struct Vertex {
    azer::Vector4 position;
    azer::Vector2 coordtex;
    azer::Vector4 normal;
    azer::Vector4 weights;
    int index[4];
    Vertex(const azer::Vector4 p0, const azer::Vector2 p1, const azer::Vector4 p2,
           const std::vector<int>& p3, const std::vector<float>& p4);
  };

  struct RenderGroup {
    azer::VertexBufferPtr vb;
    azer::IndicesBufferPtr ib;
    int mtrl_idx;
  };

  void Init(azer::RenderSystem* rs);
  void Update(const std::string& name, double t);
  struct AnimBlending {
    std::string name;
    double factor;

    AnimBlending(const std::string& n, double f)
        : name(n), factor(f) {
    }
  };
  void Update(const std::vector<AnimBlending>& name, double time);
  void Update(double t);
  void Render(azer::Renderer* renderer, const azer::Matrix4& world,
              const azer::Matrix4& pv);
 private:
  std::vector<Vertex> InitVertex(const sbox::SkinnedMesh::VertexVec& vec);
  sbox::SkinnedMesh* mesh_;
  std::vector<azer::Matrix4> bone_mat_;
  std::vector<azer::Matrix4> temp_mat_;
  std::vector<RenderGroup> rgroups_;
  std::shared_ptr<azer::Effect> effect_;
  DISALLOW_COPY_AND_ASSIGN(HardwareSkinnedMesh);
};
