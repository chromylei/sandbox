#include "sbox/base/animation.h"
#include "sbox/base/assimp.h"
#include "sbox/base/skeleton.h"

void Animation::Load(aiAnimation* anim) {
  for (uint32 i = 0; i < anim->mNumChannels; ++i) {
    aiNodeAnim* anim_node = anim->mChannels[i];
    std::string node_name = std::string(anim_node->mNodeName.data);
    for (int j = 0; j < anim_node->mNumPositionKeys;  ++j) {
      aiVectorKey& key = anim_node->mPositionKeys[j];
      positions_.Add(azer::Vector4(
          key.mValue.x, key.mValue.y, key.mValue.z, 1.0), key.mTime);
    }

    for (int j = 0; j < anim_node->mNumScalingKeys;  ++j) {
      aiVectorKey& key = anim_node->mScalingKeys[j];
      scales_.Add(azer::Vector4(
          key.mValue.x, key.mValue.y, key.mValue.z, 1.0), key.mTime);
    }

    for (int j = 0; j < anim_node->mNumRotationKeys;  ++j) {
      aiQuatKey& key = anim_node->mRotationKeys[j];
      rotate_.Add(azer::Quaternion(
          key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z), key.mTime);
    }
  }
}

AnimationSet::~AnimationSet() {
  for (auto iter = animations_.begin(); iter != animations_.end(); ++iter) {
    delete iter->second;
  }
}

void AnimationSet::Load(aiScene* scene) {
  for (uint32 i = 0; i < scene->mNumAnimations; ++i) {
    aiAnimation* ai_anim = scene->mAnimations[i];
    std::string name(ai_anim->mName.data);
    int bone_index = skeleton_->GetBoneIndex(name);
    Bone* bone = skeleton_->GetBone(bone_index);
    Animation* anim = new Animation(bone);
    anim->Load(ai_anim);
    animations_.insert(std::make_pair(name, anim));
  }
}
