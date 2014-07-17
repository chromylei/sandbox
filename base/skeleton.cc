#include "sbox/base/skeleton.h"
#include "sbox/base/assimp.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"

Bone::Bone(const std::string& name, Bone* parent)
    : TreeNode<Bone>(::base::UTF8ToWide(name), parent)
    , bone_name_(name) {
}

void Skeleton::HierarchyBone(aiNode* node, Bone* bone) {
  for (uint32 i = 0; i < node->mNumChildren; ++i) {
    aiNode* cnode = node->mChildren[i];
    Bone* new_bone = new Bone(node->mName.data, bone);
    new_bone->transform_ = azer::Matrix4(cnode->mTransformation[0][0],
                                         cnode->mTransformation[0][1],
                                         cnode->mTransformation[0][2],
                                         cnode->mTransformation[0][3],
                                         cnode->mTransformation[1][0],
                                         cnode->mTransformation[1][1],
                                         cnode->mTransformation[1][2],
                                         cnode->mTransformation[1][3],
                                         cnode->mTransformation[2][0],
                                         cnode->mTransformation[2][1],
                                         cnode->mTransformation[2][2],
                                         cnode->mTransformation[2][3],
                                         cnode->mTransformation[3][0],
                                         cnode->mTransformation[3][1],
                                         cnode->mTransformation[3][2],
                                         cnode->mTransformation[3][3]);
    AddNewBone(new_bone);
    bone->AddChild(new_bone);
  }
}

void Skeleton::AddNewBone(Bone* bone) {
  bone_.push_back(bone);
  DCHECK(bone_map_.find(bone->node_name()) == bone_map_.end());
  bone_map_[bone->node_name()] = bone_.size() - 1;
}

int Skeleton::GetBoneIndex(const std::string& index) {
  return 0;
}
