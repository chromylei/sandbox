#include "sbox/base/skeleton.h"
#include "base/strings/utf_string_conversions.h"

Bone::Bone(const std::string& name)
    : TreeNode<Bone>(::base::UTF8ToWide(name), NULL) {
}

int Skeleton::GetBoneIndex(const std::string& index) {
  return 0;
}
