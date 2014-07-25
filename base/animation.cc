#include "sbox/base/animation.h"
#include "sbox/base/assimp.h"
#include "sbox/base/skeleton.h"

void AnimationNode::Load(const aiNodeAnim* anim_node) {
  std::string node_name = std::string(anim_node->mNodeName.data);
  for (int j = 0; j < anim_node->mNumPositionKeys;  ++j) {
    aiVectorKey& key = anim_node->mPositionKeys[j];
    positions_.Add(azer::Vector4(
        key.mValue.x, key.mValue.y, key.mValue.z, 1.0), key.mTime / 1000.0f);
  }

  for (int j = 0; j < anim_node->mNumScalingKeys;  ++j) {
    aiVectorKey& key = anim_node->mScalingKeys[j];
    scales_.Add(azer::Vector4(
        key.mValue.x, key.mValue.y, key.mValue.z, 1.0), key.mTime / 1000.0f);
  }

  for (int j = 0; j < anim_node->mNumRotationKeys;  ++j) {
    aiQuatKey& key = anim_node->mRotationKeys[j];
    rotate_.Add(azer::Quaternion(
        key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z), key.mTime / 1000.0f);
  }
}

azer::Matrix4 AnimationNode::transform(double t) const {
  azer::Matrix4 mat = azer::Matrix4::kIdentity;
  if (positions_.has_data()) {
    azer::Matrix4 trans = std::move(positions_.Interpolate(t));
    mat = std::move(mat * trans);
  }

  if (rotate_.has_data()) {
    azer::Matrix4 rotate = std::move(rotate_.Interpolate(t));
    mat = std::move(mat * rotate);
  }
  return mat;
}

void Animation::Load(const aiAnimation* anim) {
  nodes_.resize(skeleton_->GetBoneNum());
  for (uint32 i = 0; i < anim->mNumChannels; ++i) {
    aiNodeAnim* anim_node = anim->mChannels[i];
    std::string name(anim_node->mNodeName.data);
    int bone_index = skeleton_->GetBoneIndex(name);
    DCHECK_GE(bone_index, 0u);
    std::shared_ptr<AnimationNode> node(new AnimationNode(bone_index, name));
    node->Load(anim_node);
    nodes_[bone_index] = node;
  }
}

azer::Matrix4 Animation::CalcBone(double t, int index) const {
  DCHECK_LT(index, nodes_.size());
  std::shared_ptr<AnimationNode> annode = nodes_[index];
  if (annode.get()) {
    return std::move(annode->transform(t));
  } else {
    return azer::Matrix4::kIdentity;
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
