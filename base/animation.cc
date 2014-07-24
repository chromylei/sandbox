#include "sbox/base/animation.h"
#include "sbox/base/assimp.h"
#include "sbox/base/skeleton.h"

void AnimationNode::Load(const aiNodeAnim* anim_node) {
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

azer::Matrix4 AnimationNode::transform(double t) const {
  return azer::Matrix4::kIdentity;
}

void Animation::Load(const aiAnimation* anim) {
  for (uint32 i = 0; i < anim->mNumChannels; ++i) {
    aiNodeAnim* anim_node = anim->mChannels[i];
    std::string name(anim_node->mNodeName.data);
    int bone_index = skeleton_->GetBoneIndex(name);
    Bone* bone = skeleton_->GetBone(bone_index);
    std::shared_ptr<AnimationNode> node(new AnimationNode(bone));
    node->Load(anim_node);
    nodes_.push_back(node);
  }
}

void Animation::CalcBone(double t, std::vector<azer::Matrix4>* mat) {
  for (auto iter = nodes_.begin(); iter != nodes_.end(); ++iter) {
    AnimationNode annode = (*iter);
    azer::Matrix4 mat = std::move(annode->transform());
    mat[annode->bone()->index()] = mat;
  }
}

void AnimationSet::Load(const aiScene* scene) {
  for (uint32 i = 0; i < scene->mNumAnimations; ++i) {
    aiAnimation* ai_anim = scene->mAnimations[i];
    std::string name(ai_anim->mName.data);
    
    std::shared_ptr<Animation> anim(new Animation(name, skeleton_));
    anim->Load(ai_anim);
    animations_.insert(std::make_pair(name, anim));
  }
}


const Animation* AnimationSet::GetAnimation(const std::string& name) const {
  auto iter = animations_.find(name);
  if (iter != animations_.end()) {
    return iter->second.get();
  } else {
    return NULL;
  }
}
