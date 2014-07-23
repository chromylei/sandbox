#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/base/base.h"

#include "hardware_skinned.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/skinned_mesh2/"
#define SHADER_NAME "haredware_skinned.afx"
using base::FilePath;

#define kMeshPath "sbox/skinned_mesh2/res/soldier.X"

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  virtual void OnCreate() {}

  void Init() {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    azer::ShaderArray shaders;
    CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
    CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
    effect_.reset(new HaredwareSkinnedMeshEffect(shaders.GetShaderVec(),
                                    azer::RenderSystem::Current()));

    renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
    CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
    CHECK(renderer->GetCullingMode() == azer::kCullBack);
    renderer->EnableDepthTest(true);
    // renderer->SetCullingMode(azer::kCullNone);
    // renderer->SetFillMode(azer::kWireFrame);

    azer::Vector3 eye(0.0f, 0.0f, 5.0f);
    azer::Vector3 dir(0.0f, 0.0f, -1.0f);
    azer::Vector3 up(0.0f, 1.0f, 0.0f);
    view_ = std::move(LookDirRH(eye, dir, up));
    proj_ = std::move(
        PerspectiveRHD3D(azer::Degree(45.0f), 4.0f / 3.0f, 0.10f, 100.0f));

    camera_.SetPosition(azer::Vector3(0.0f, 1.0f, 3.0f));
    mesh_.Load(::base::FilePath(::base::UTF8ToWide(kMeshPath)), rs);
    mesh_.GetSkeleton().PrepareRender(rs);
    mesh_.GetSkeleton().UpdateHierarchy(azer::Matrix4::kIdentity);

    bone_mat_.resize(mesh_.GetSkeleton().GetBoneNum());
    LOG(ERROR) << "\n" << mesh_.GetSkeleton().DumpHierarchy();
  }
  virtual void OnUpdateScene(double time, float delta_time) {
    float rspeed = 3.14f * 2.0f / 4.0f;
    azer::Radians camera_speed(azer::kPI / 2.0f);
    UpdatedownCamera(&camera_, camera_speed, delta_time);
  }

  virtual void OnRenderScene(double time, float delta_time) {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    renderer->SetCullingMode(azer::kCullNone);
    DCHECK(NULL != rs);
    renderer->Clear(azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f));
    // renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->ClearDepthAndStencil();
    // mesh_.GetSkeleton().Render(renderer, camera_.GetProjViewMatrix());

    Render(effect_.get(), renderer, camera_.GetProjViewMatrix(),
           azer::Matrix4::kIdentity, &mesh_);
  }

  void Render(HaredwareSkinnedMeshEffect* effect, azer::Renderer* renderer,
              const azer::Matrix4& pv, const azer::Matrix4& world,
              HardwareSkinnedMesh* mesh);
  virtual void OnQuit() {}
 private:
  azer::Camera camera_;
  azer::Matrix4 proj_;
  azer::Matrix4 view_;
  std::vector<azer::Matrix4> bone_mat_;
  std::unique_ptr<HaredwareSkinnedMeshEffect> effect_;
  HardwareSkinnedMesh mesh_;
};

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");
  
  MainDelegate delegate;
  azer::WindowHost win(azer::WindowHost::Options(), &delegate);
  win.Init();
  CHECK(azer::LoadRenderSystem(&win));
  LOG(ERROR) << "Current RenderSystem: " << azer::RenderSystem::Current()->name();
  delegate.Init();
  win.Show();
  azer::MainRenderLoop(&win);
  return 0;
}

void MainDelegate::Render(HaredwareSkinnedMeshEffect* effect,
                          azer::Renderer* renderer,
                          const azer::Matrix4& pv, const azer::Matrix4& world,
                          HardwareSkinnedMesh* mesh) {
  for (uint32 i = 0; i < mesh->rgroups().size(); ++i) {
    const HardwareSkinnedMesh::RenderGroup& rg = mesh->rgroups()[i];
    const HardwareSkinnedMesh::Group& group = mesh->groups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    HardwareSkinnedMesh::Material mtrl = mesh->materials()[rg.mtrl_idx];
    
    memcpy(&bone_mat_[0], &(mesh->GetSkeleton().GetBoneMat()[0]),
           sizeof(azer::Matrix4) * bone_mat_.size());
    for (auto iter = group.offset.begin(); iter != group.offset.end(); ++iter) {
      bone_mat_[iter->first] = mesh->GetSkeleton().GetBoneMat()[iter->first]
          * iter->second;
    }
    
    effect->SetBones((azer::Matrix4*)(&bone_mat_[0]), bone_mat_.size());
    effect->SetProjView(pv);
    effect->SetWorld(world);
    effect->SetDiffuseTex(mtrl.tex);
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}
