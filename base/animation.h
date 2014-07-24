#pragma once

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "sbox/base/skeleton.h"
#include "azer/math/math.h"

struct aiScene;
class Animation {
 public:
  Animation() {}
  void Load(aiScene* scene);

  const std::string& name() const { return name_;}

  struct AnimationData {
    azer::Vector4 position;
    azer::Vector4 scale;
    azer::Quaternion rotate;
    Bone* bone;
  };
  typedef std::vector<AnimationData> AnimationDataVec;
  const AnimationDataVec& GetAnimationData() const { return animation_;}
  AnimationDataVec& GetAnimationData() { return animation_;}
 private:
  std::string name_;
  std::vector<AnimationData> animation_;
  DISALLOW_COPY_AND_ASSIGN(Animation);
};

class AnimationSet {
 public:
  AnimationSet() {}
  void Load(aiScene* scene);
 private:
  std::map<std::string, Animation*> animations_;
  DISALLOW_COPY_AND_ASSIGN(AnimationSet);
};
