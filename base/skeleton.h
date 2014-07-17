#pragma once

#include <string>
#include <map>
#include <memory>

#include "base/files/file_path.h"
#include "base/basictypes.h"
#include "azer/base/tree_node.h"
#include "azer/base/string.h"
#include "azer/render/render.h"

struct aiNode;  // for assimp

class Bone : public azer::TreeNode<Bone> {
 public:
  explicit Bone(const std::string& name, Bone* parent);
  const std::string&  bone_name() const { return bone_name_;}
 private:
  std::string bone_name_;
  azer::Matrix4 transform_;
  azer::Matrix4 local_transform_;

  friend class Skeleton;
  DISALLOW_COPY_AND_ASSIGN(Bone);
};

class Skeleton {
 public:
  Skeleton();
  void Render(azer::Renderer* renderer);

  int GetBoneIndex(const std::string& name);
 private:
  void HierarchyBone(aiNode* node, Bone* bone);
  void AddNewBone(Bone* bone);
  std::map<azer::StringType, int> bone_map_;
  std::vector<Bone*> bone_;

  friend class SkinnedMesh;
  DISALLOW_COPY_AND_ASSIGN(Skeleton);
};
