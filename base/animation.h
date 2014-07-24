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

  azer::Matrix4 Interpolate(double t);
  int FindIndex(double t, double* left) const;
 private:
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
  azer::Matrix4 transform(double t) const;
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

  void CalcBone(double t, std::vector<azer::Matrix4>* mat);
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


template<>
azer::Matrix4 AnimationData<azer::Quaternion>::Interpolate(double t) {
  int index = FindIndex(t);
  double interp = 0.0f;
  const azer::Quaternion& quat1 = data(index, &interp);
  const azer::Quaternion& quat2 = data(NextIndex(index));
  return std::move(azer::Queratonion((float)interp, quat1, quat2));
}

template<>
azer::Matrix4 AnimationData<azer::Vector4>::Interpolate(double t) {
  int index = FindIndex(t);
  double interp = 0.0f;
  const azer::Quaternion& quat1 = data(index, &interp);
  const azer::Quaternion& quat2 = data(NextIndex(index));
  return std::move(azer::Queratonion((float)interp, quat1, quat2));
  return azer::Matrix4::kIdentity;
}

template<class T>
int AnimationData<T>::FindIndex(double t, double left) const {
  int num = (int)(t / duration_);
  t -= num * duration_;

  double sum = 0.0;
  for (size_t i = 0; i < time_.size(); ++i) {
    if (t > *iter) {
      *left = t - sum;
      return i;
    }

    sum += *iter;
  }
  NOTREACHED();
  return -1;
}

int AnimationData<T>::NextIndex(int index) const {
  return (index + 1) % time_.size();
}
