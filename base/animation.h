#pragma once

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
class Animation {
 public:
  explicit Animation(Bone* bone) : bone_(bone) {}
  void Load(aiAnimation* anim);

  const std::string& name() const { return name_;}
 private:
  double duration_;
  std::string name_;
  AnimationData<azer::Vector4> positions_;
  AnimationData<azer::Vector4> scales_;
  AnimationData<azer::Quaternion> rotate_;
  Bone* bone_;
  DISALLOW_COPY_AND_ASSIGN(Animation);
};

class AnimationSet {
 public:
  explicit AnimationSet(Skeleton* skeleton) : skeleton_(skeleton) {}
  ~AnimationSet();
  void Load(aiScene* scene);
 private:
  Skeleton* skeleton_;
  std::map<std::string, Animation*> animations_;
  DISALLOW_COPY_AND_ASSIGN(AnimationSet);
};
