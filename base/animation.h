#pragma once

#include <memory>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "sbox/base/skeleton.h"
#include "azer/math/math.h"
#include "base/logging.h"


template <class T>
class AnimationData {
 public:
  AnimationData() : duration_(0) {}
  double duration() const { return duration_; }
  const T& data(double t) const {
    CHECK(data_.size() > 0);
    CHECK(duration_ > 0);
    return data_[FindIndex(t)];
  }

  void Add(const T& t, double duration) {
    duration_ += duration_;
    data_.push_back(t);
  }
 private:
  int FindIndex(double t) const {
    while (t > duration_) {
      t -= duration_;
    }

    for (size_t i = 0; i < time_.size(); ++i) {
      if (t > *iter) {
        return i;
      }
    }
    NOTREACHED();
    return -1;
  }
  std::vector<double> time_;
  std::vector<T> data_;
  double duration_;

  friend class Animation;
  DISALLOW_COPY_AND_ASSIGN(AnimationData);
};

struct aiScene;
struct aiAnimation;
struct aiVectorKey;
struct aiQuatKey;
struct aiNodeAnim;

class AnimationNode {
 public:
  explicit AnimationNode(Bone* bone)
      : bone_(bone) {
  }

  void Load(const aiNodeAnim* animnode);

  const std::string& name() const { return name_;}
 private:
  double duration_;
  std::string name_;
  AnimationData<azer::Vector4> positions_;
  AnimationData<azer::Vector4> scales_;
  AnimationData<azer::Quaternion> rotate_;
  Bone* bone_;
  DISALLOW_COPY_AND_ASSIGN(AnimationNode);
};

class Animation {
 public:
  Animation(const std::string& name, Skeleton* skeleton)
      : name_(name)
      , skeleton_(skeleton) {
  }
  void Load(const aiAnimation* anim);
 private:
  std::string name_;
  Skeleton* skeleton_;
  std::vector<std::shared_ptr<AnimationNode> > nodes_;
  DISALLOW_COPY_AND_ASSIGN(Animation);
};

class AnimationSet {
 public:
  explicit AnimationSet(Skeleton* skeleton) : skeleton_(skeleton) {}
  ~AnimationSet() {}
  void Load(const aiScene* scene);
 private:
  Skeleton* skeleton_;
  std::map<std::string, std::shared_ptr<Animation> > animations_;
  DISALLOW_COPY_AND_ASSIGN(AnimationSet);
};
