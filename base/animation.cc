#include "sbox/base/animation.h"
#include "sbox/base/assimp.h"

void Animation::Load(aiScene* scene) {
  for (uint32 i = 0; i < scene->mNumAnimations; ++i) {
    aiAnimation* anim = scene[i];
  }
}
