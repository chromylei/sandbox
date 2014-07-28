#pragma once

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "base/basictypes.h"
#include "azer/math/math.h"
#include "base/logging.h"

class Skeleton;

template <class T>
class AnimationData {
 public:
  AnimationData() : duration_(0) {}
  double duration() const {
    DCHECK_GT(time_.size(), 0u);
    return time_.back();
  }
  void Add(const T& t, double duration) {
    time_.push_back(duration);
    data_.push_back(t);
  }

  T Interpolate(double t) const;
  int FindIndex(double t, double* left) const;
  bool has_data() const { return time_.size() != 0u;}
 private:
  int NextIndex(int index) const;
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
  explicit AnimationNode(int bone_index, const std::string& name)
      : name_(name)
      , bone_index_(bone_index) {
  }

  void Load(const aiNodeAnim* animnode);

  const std::string& name() const { return name_;}
  azer::Matrix4 transform(double t) const;
  int bone_index() const { return bone_index_;}
 private:
  std::string name_;
  int bone_index_;
  AnimationData<azer::Vector4> positions_;
  AnimationData<azer::Vector4> scales_;
  AnimationData<azer::Quaternion> rotate_;
  DISALLOW_COPY_AND_ASSIGN(AnimationNode);
};

class Animation {
 public:
  Animation(const std::string& name, Skeleton* skeleton)
      : name_(name)
      , skeleton_(skeleton) {
  }
  void Load(const aiAnimation* anim);

  azer::Matrix4 CalcBone(double t, int index) const;
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

  const Animation* GetAnimation(const std::string& name) const;
 private:
  Skeleton* skeleton_;
  std::map<std::string, std::shared_ptr<Animation> > animations_;
  DISALLOW_COPY_AND_ASSIGN(AnimationSet);
};


template<>
inline azer::Quaternion AnimationData<azer::Quaternion>::Interpolate(double t)
    const {
  double interp = 0.0f;
  int index = FindIndex(t, &interp);
  int next_index = NextIndex(index);
  const azer::Quaternion& quat1 = data_[index];
  const azer::Quaternion& quat2 = data_[next_index];
  const azer::Quaternion rot =
      std::move(azer::Quaternion::nlerp((float)interp, quat1, quat2));
  return rot;
}

template<>
inline azer::Vector4 AnimationData<azer::Vector4>::Interpolate(double t) const {
  double interp = 0.0f;
  int index = FindIndex(t, &interp);
  int next_index = NextIndex(index);
  const azer::Vector4& v1 = data_[index];
  const azer::Vector4& v2 = data_[next_index];
  azer::Vector4 pos = std::move(v1 * interp + v2 * (1.0f - interp));
  return pos;
}

template<class T>
int AnimationData<T>::FindIndex(double t, double* left) const {
  DCHECK_GT(time_.size(), 0u);
  const double duration = time_.back();
  int num = (int)(t / duration);
  t -= num * duration;

  for (size_t i = 0; i < time_.size() - 1; ++i) {
    if (t < time_[i + 1]) {
      double span = time_[i+1] - time_[i];
      *left = (t - time_[i]) / span;
      return i;
    }
  }
  NOTREACHED();
  return -1;
}

template <class T>
int AnimationData<T>::NextIndex(int index) const {
  return (index + 1) % time_.size();
}
