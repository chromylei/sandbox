#include "sbox/base/skeleton.h"
#include "sbox/base/assimp.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"

Bone::Bone(const std::string& name, Bone* parent)
    : TreeNode<Bone>(::base::UTF8ToWide(name), parent)
    , bone_name_(name) {
}

bool Skeleton::Load(aiNode* root) {
  Bone* new_bone = InitBone(root);
  HierarchyBone(root, new_bone);
  return true;
}

Bone* Skeleton::InitBone(aiNode* node) {
  Bone* new_bone = new Bone(node->mName.data, NULL);
  new_bone->transform_ = azer::Matrix4(node->mTransformation[0][0],
                                       node->mTransformation[0][1],
                                       node->mTransformation[0][2],
                                       node->mTransformation[0][3],
                                       node->mTransformation[1][0],
                                       node->mTransformation[1][1],
                                       node->mTransformation[1][2],
                                       node->mTransformation[1][3],
                                       node->mTransformation[2][0],
                                       node->mTransformation[2][1],
                                       node->mTransformation[2][2],
                                       node->mTransformation[2][3],
                                       node->mTransformation[3][0],
                                       node->mTransformation[3][1],
                                       node->mTransformation[3][2],
                                       node->mTransformation[3][3]);
  AddNewBone(new_bone);
  return new_bone;
}

void Skeleton::HierarchyBone(aiNode* pnode, Bone* bone) {
  for (uint32 i = 0; i < pnode->mNumChildren; ++i) {
    aiNode* cnode = pnode->mChildren[i];
    Bone* new_bone = InitBone(cnode);
    bone->AddChild(new_bone);
  }
}

void Skeleton::AddNewBone(Bone* bone) {
  bone_.push_back(bone);
  DCHECK(bone_map_.find(bone->node_name()) == bone_map_.end())
      << "cannot find bone " << bone->node_name();
  bone_map_[bone->node_name()] = bone_.size() - 1;
}

int Skeleton::GetBoneIndex(const std::string& name) {
  auto iter = bone_map_.find(::base::UTF8ToWide(name));
  DCHECK(iter != bone_map_.end());
  return iter->second;
}

Bone* Skeleton::root() {
  DCHECK_GT(bone_.size(), 0);
  return bone_[0];
}

Bone* Skeleton::root() const {
  DCHECK_GT(bone_.size(), 0);
  return bone_[0];
}

namespace {
class SkeletonHierarchyTraverser : public azer::TreeNode<Bone>::Traverser {
 public:
  SkeletonHierarchyTraverser() : depth_(0) {}
  virtual bool OnVisitBegin(Bone* bone) {
    std::string ident(depth_ * 2, ' ');
    std::stringstream ss;
    ss << ident << bone->bone_name() << "\n";
    depth_++;
    dump_.append(ss.str());
    return true;
  }

  virtual void OnVisitEnd(Bone* bone) {
    depth_--;
  }

  const std::string& str() const { return dump_;}
 private:
  int depth_;
  std::string dump_;
  DISALLOW_COPY_AND_ASSIGN(SkeletonHierarchyTraverser);
};
}  // namespace

std::string Skeleton::DumpHierarchy() const {
  SkeletonHierarchyTraverser traverser;
  root()->traverse(&traverser);
  return traverser.str();
}
