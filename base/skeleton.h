#pragma once

#include <string>
#include <map>
#include <memory>

#include "base/files/file_path.h"
#include "base/basictypes.h"
#include "azer/base/tree_node.h"
#include "azer/base/string.h"
#include "sbox/base/mesh.h"

class Bone : public azer::TreeNode<Bone> {
 public:
  explicit Bone(const std::string& name);
 private:
  DISALLOW_COPY_AND_ASSIGN(Bone);
};

class Skeleton {
 public:
  Skeleton();
  void Render(azer::Renderer* renderer);

  int GetBoneIndex(const std::string& name);
 private:
  std::map<azer::StringType, int> bone_map_;
  std::vector<Bone*> bone_;
  DISALLOW_COPY_AND_ASSIGN(Skeleton);
};
