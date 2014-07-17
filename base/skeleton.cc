#include "sbox/base/skeleton.h"

#include "sbox/base/assimp.h"
#include "sbox/base/loader.h"
#include "sbox/base/mesh.h"
#include "base/strings/utf_string_conversions.h"
#include "base/logging.h"
#include "azer/render/render.h"

#include "skeleton_effect.h"

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "skeletion_effect"

Bone::Bone(const std::string& name, Bone* parent)
    : TreeNode<Bone>(::base::UTF8ToWide(name), parent)
    , bone_name_(name) {
}

Skeleton::Skeleton() {
  sphere_ = new Mesh;
}

Skeleton::~Skeleton() {
  if (sphere_) delete sphere_;
}

bool Skeleton::Load(aiNode* root, azer::RenderSystem* rs) {
  Bone* new_bone = InitBone(root);
  HierarchyBone(root, new_bone);
  return true;
}

namespace {
#define kSphereMeshPath "sbox/res/sphere.3DS"
const azer::VertexDesc::Desc kVertexDesc[] = {
  {"POSITION", 0, azer::kVec3},
};
}

void Skeleton::InitVertex(azer::RenderSystem* rs) {
  /*
  azer::VertexDescPtr vertex_desc_ptr(
      new azer::VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, 2));
  */
  LoadMesh(::base::FilePath(::base::UTF8ToWide(kSphereMeshPath)), sphere_, rs);
}

Bone* Skeleton::InitBone(aiNode* node) {
  Bone* new_bone = new Bone(node->mName.data, NULL);
  new_bone->transform_ = azer::Matrix4(node->mTransformation[0][0],
                                       node->mTransformation[0][1],
                                       node->mTransformation[0][2],
                                       node->mTransformation[0][3],
                                       node->mTransformation[1][0],
                                       node->mTransformation[1][1],
                                       node->mTransformation[1][2],
                                       node->mTransformation[1][3],
                                       node->mTransformation[2][0],
                                       node->mTransformation[2][1],
                                       node->mTransformation[2][2],
                                       node->mTransformation[2][3],
                                       node->mTransformation[3][0],
                                       node->mTransformation[3][1],
                                       node->mTransformation[3][2],
                                       node->mTransformation[3][3]);
  AddNewBone(new_bone);
  return new_bone;
}

void Skeleton::HierarchyBone(aiNode* pnode, Bone* bone) {
  for (uint32 i = 0; i < pnode->mNumChildren; ++i) {
    aiNode* cnode = pnode->mChildren[i];
    Bone* new_bone = InitBone(cnode);
    bone->AddChild(new_bone);
  }
}

void Skeleton::AddNewBone(Bone* bone) {
  bone_.push_back(bone);
  DCHECK(bone_map_.find(bone->node_name()) == bone_map_.end())
      << "cannot find bone " << bone->node_name();
  bone_map_[bone->node_name()] = bone_.size() - 1;
}

int Skeleton::GetBoneIndex(const std::string& name) {
  auto iter = bone_map_.find(::base::UTF8ToWide(name));
  DCHECK(iter != bone_map_.end());
  return iter->second;
}

Bone* Skeleton::root() {
  DCHECK_GT(bone_.size(), 0);
  return bone_[0];
}

Bone* Skeleton::root() const {
  DCHECK_GT(bone_.size(), 0);
  return bone_[0];
}

namespace {
class SkeletonHierarchyTraverser : public azer::TreeNode<Bone>::Traverser {
 public:
  SkeletonHierarchyTraverser() : depth_(0) {}
  virtual bool OnVisitBegin(Bone* bone) {
    std::string ident(depth_ * 2, ' ');
    std::stringstream ss;
    ss << ident << bone->bone_name() << "\n";
    depth_++;
    dump_.append(ss.str());
    return true;
  }

  virtual void OnVisitEnd(Bone* bone) {
    depth_--;
  }

  const std::string& str() const { return dump_;}
 private:
  int depth_;
  std::string dump_;
  DISALLOW_COPY_AND_ASSIGN(SkeletonHierarchyTraverser);
};
}  // namespace

std::string Skeleton::DumpHierarchy() const {
  SkeletonHierarchyTraverser traverser;
  root()->traverse(&traverser);
  return traverser.str();
}

namespace {
void RenderSphere(SkeletonEffect* effect, azer::Renderer* renderer, Mesh* mesh) {
  for (uint32 i = 0; i < mesh->rgroups().size(); ++i) {
    Mesh::RenderGroup rg = mesh->rgroups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    effect->SetSkeletonDiffuse(azer::Vector4(0.8f, 0.7f, 0.6f, 1.0f));
    effect->Use(renderer);
    renderer->Render(vb, azer::kTriangleList);
  }
}
}  // namespace

void Skeleton::Render(Bone* node, azer::Renderer* renderer) {
  Bone* cur = node->first_child();
  for (; cur != NULL; cur = cur->next_sibling()) {
    RenderSphere((SkeletonEffect*)effect_.get(), renderer, sphere_);
    Render(cur, renderer);
  }
}

void Skeleton::Render(azer::Renderer* renderer) {
  Render(root(), renderer);
}
