#pragma once

#include <string>
#include <map>
#include <memory>

#include "base/files/file_path.h"
#include "base/basictypes.h"
#include "azer/base/tree_node.h"
#include "azer/base/string.h"
#include "azer/render/render.h"
#include "sbox/base/util.h"

struct aiNode;  // for assimp
class Mesh;
class Animation;
class Bone : public azer::TreeNode<Bone> {
 public:
  explicit Bone(const std::string& name, Bone* parent);
  const std::string&  bone_name() const { return bone_name_;}

  azer::Vector3 position() const;
  const azer::Matrix4& combined() const { return combined_transform_;}
  const azer::Matrix4& local() const { return transform_;}
  int index() const { return index_;}
 private:
  int index_;
  std::string bone_name_;
  azer::Matrix4 transform_;
  azer::Matrix4 combined_transform_;

  friend class Skeleton;
  friend class SoftSkinnedMesh;
  DISALLOW_COPY_AND_ASSIGN(Bone);
};

class Skeleton {
 public:
  Skeleton();
  ~Skeleton();
  void PrepareRender(azer::RenderSystem* rs);
  void Render(azer::Renderer* renderer, const azer::Matrix4& pvw);
  
  int GetBoneIndex(const std::string& name) const;
  Bone* GetBone(int idx) { return bone_[idx];}
  bool Load(aiNode* root, azer::RenderSystem* rs);
  int GetBoneNum() const { return bone_.size();}
  std::vector<Bone*>& GetBoneVec() { return bone_;}
  const std::vector<Bone*>& GetBoneVec() const { return bone_;}

  std::string DumpHierarchy() const;
  void UpdateHierarchy(const azer::Matrix4& world);
  void UpdateHierarchy(double t, const Animation& animation,
                       const azer::Matrix4& world);
  const std::vector<azer::Matrix4>& GetBoneMat() const { return bone_mat_;}
  Bone* root();
  Bone* root() const;
 private:
  void UpdateHierarchy(Bone* bone, const azer::Matrix4& mat);
  void UpdateHierarchy(double t, Bone* bone, const Animation& animation,
                       const azer::Matrix4& world);
  void Render(Bone* node, azer::Renderer* renderer, const azer::Matrix4& pvw);
  void HierarchyBone(aiNode* node, Bone* bone);
  Bone* InitBone(aiNode* node);
  void AddNewBone(Bone* bone);
  std::map<azer::StringType, int> bone_map_;
  std::vector<Bone*> bone_;
  std::vector<azer::Matrix4> bone_mat_;

  // for render skeleton
  Mesh* sphere_;
  std::unique_ptr<azer::Effect> effect_;
  azer::VertexBufferPtr vb_;
  LineRenderer line_;
  int empty_skeleton_;

  azer::Matrix4 global_inverse_;
  friend class SkinnedMesh;
  DISALLOW_COPY_AND_ASSIGN(Skeleton);
};
