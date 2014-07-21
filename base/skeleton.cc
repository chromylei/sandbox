#include "sbox/base/skeleton.h"

#include "sbox/base/assimp.h"
#include "sbox/base/util.h"
#include "sbox/base/mesh.h"
#include "base/strings/utf_string_conversions.h"
#include "base/strings/stringprintf.h"
#include "base/logging.h"
#include "azer/render/render.h"

#include "skeleton_effect.afx.h"

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/base/"
#define SHADER_NAME "skeleton.afx"
#define kSphereMeshPath "sbox/res/sphere.3DS"

Bone::Bone(const std::string& name, Bone* parent)
    : TreeNode<Bone>(::base::UTF8ToWide(name), parent)
    , bone_name_(name)
    , offset_(azer::Matrix4::kIdentity) {
}

azer::Vector3 Bone::position() const {
  return azer::Vector3(combined_transform_[0][3],
                       combined_transform_[1][3],
                       combined_transform_[2][3]);
}

Skeleton::Skeleton() {
  sphere_ = new Mesh;
}

Skeleton::~Skeleton() {
  if (sphere_) delete sphere_;
}

bool Skeleton::Load(aiNode* root, azer::RenderSystem* rs) {
  aiMatrix4x4 mat = root->mTransformation;
  mat.Inverse();
  global_inverse_ = azer::Matrix4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                                  mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                                  mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                                  mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
  Bone* new_bone = InitBone(root);
  HierarchyBone(root, new_bone);
  return true;
}

void Skeleton::PrepareRender(azer::RenderSystem* rs) {
  LoadMesh(::base::FilePath(::base::UTF8ToWide(kSphereMeshPath)), sphere_, rs);

  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new SkeletonEffect(shaders.GetShaderVec(), rs));

  line_.Init(rs);
}

Bone* Skeleton::InitBone(aiNode* node) {
  std::string name;
  if (strlen(node->mName.data) > 0) {
    name = std::string(node->mName.data);
  } else {
    name = ::base::StringPrintf("__anonymous__%d", empty_skeleton_++);
    return NULL;
  }

  Bone* new_bone = new Bone(name, NULL);
  aiMatrix4x4 mat = node->mTransformation;
  // mat.Inverse();
  new_bone->transform_ = azer::Matrix4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                                       mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                                       mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                                       mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    // new_bone->transform_.Transpose();
  AddNewBone(new_bone);

  return new_bone;
}

void Skeleton::HierarchyBone(aiNode* pnode, Bone* bone) {
  for (uint32 i = 0; i < pnode->mNumChildren; ++i) {
    aiNode* cnode = pnode->mChildren[i];
    Bone* new_bone = InitBone(cnode);
    if (new_bone) {
      bone->AddChild(new_bone);
      HierarchyBone(cnode, new_bone);
    }
  }
}

void Skeleton::AddNewBone(Bone* bone) {
  DCHECK(bone_map_.find(bone->node_name()) == bone_map_.end())
      << "bone \"" << bone->node_name() << "\" has been exist";
  bone_.push_back(bone);
  bone_map_[bone->node_name()] = bone_.size() - 1;
}

int Skeleton::GetBoneIndex(const std::string& name) const {
  auto iter = bone_map_.find(::base::UTF8ToWide(name));
  DCHECK(iter != bone_map_.end()) << "cannot find bone: \"" << name << "\"";
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
  SkeletonHierarchyTraverser(const Skeleton* skeleton)
      : depth_(0)
      , skeleton_(skeleton) {
  }

  virtual bool OnVisitBegin(Bone* bone) {
    std::string ident(depth_ * 2, ' ');
    std::stringstream ss;
    int index = skeleton_->GetBoneIndex(bone->bone_name());
    ss << ident << bone->bone_name() << "(index: " << index
       << ", position: " << bone->position() << ")";

    /*
    ss << "\n" << ident << ident << "Combined";
    for (int i = 0; i < 4; ++i) {
      ss << "\n" << ident << ident;
      for (int j = 0; j < 4; ++j) {        
        ss << bone->combined()[i][j] << "\t\t";
      }
    }
    ss << "\n" << ident << ident << "LocalTransform";
    for (int i = 0; i < 4; ++i) {
      ss << "\n" << ident << ident;
      for (int j = 0; j < 4; ++j) {        
        ss << bone->local()[i][j] << "\t\t";
      }
    }
    ss << "\n" << ident << ident << "Local";
    for (int i = 0; i < 4; ++i) {
      ss << "\n" << ident << ident;
      for (int j = 0; j < 4; ++j) {        
        ss << bone->offset()[i][j] << "\t\t";
      }
    }
    */
    ss << "\n";
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
  const Skeleton* skeleton_;
  DISALLOW_COPY_AND_ASSIGN(SkeletonHierarchyTraverser);
};
}  // namespace

std::string Skeleton::DumpHierarchy() const {
  SkeletonHierarchyTraverser traverser(this);
  root()->traverse(&traverser);
  return traverser.str();
}

namespace {
void RenderSphere(SkeletonEffect* effect, azer::Renderer* renderer, Mesh* mesh) {
  for (uint32 i = 0; i < mesh->rgroups().size(); ++i) {
    Mesh::RenderGroup rg = mesh->rgroups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    effect->Use(renderer);
    renderer->Render(vb, azer::kTriangleList);
  }
}
}  // namespace

void Skeleton::Render(Bone* node, azer::Renderer* renderer,
                      const azer::Matrix4& pv) {
  azer::Matrix4 scale = azer::Scale(0.02f, 0.02f, 0.02f);
  SkeletonEffect* effect = (SkeletonEffect*)effect_.get();
  Bone* cur = node->first_child();
  for (; cur != NULL; cur = cur->next_sibling()) {
    azer::Matrix4 world = std::move(cur->combined_transform_ * scale);
    azer::Matrix4 pvw = std::move(pv * world);
    effect->SetWorld(world);
    effect->SetPVW(pvw);
    effect->SetSkeletonDiffuse(azer::Vector4(0.1f, 0.1f, 0.1f, 1.0f));
    RenderSphere(effect, renderer, sphere_);
    Render(cur, renderer, pv);

    if (node->parent()) {
      azer::Vector3 p1 = node->parent()->position();
      azer::Vector3 p2 = node->position();
      line_.SetPosition(p1, p2);
      line_.Render(renderer, pv);
    }
  }
}

void Skeleton::Render(azer::Renderer* renderer, const azer::Matrix4& pvw) {
  Render(root(), renderer, pvw);
}

void Skeleton::UpdateHierarchy(const azer::Matrix4& world) {
  UpdateHierarchy(root(), world);
}

void Skeleton::UpdateHierarchy(Bone* bone, const azer::Matrix4& pmat) {
  DCHECK(bone);
  const azer::Matrix4& trans = pmat * bone->transform_;
  bone->combined_transform_ = trans * bone->offset_;
  Bone* child = bone->first_child();
  while (child) {
    UpdateHierarchy(child, trans);
    child = child->next_sibling();
  }
}
