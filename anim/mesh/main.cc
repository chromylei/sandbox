#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/base/base.h"

#include "texture.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/mesh/"
#define SHADER_NAME "texture.afx"
using base::FilePath;

#define kMeshPath "sbox/anim/mesh/res/soldier.X"

void Render(TextureEffect* effect, azer::Renderer* renderer, Mesh* mesh) {
  for (uint32 i = 0; i < mesh->rgroups().size(); ++i) {
    Mesh::RenderGroup rg = mesh->rgroups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    Mesh::Material mtrl = mesh->materials()[rg.mtrl_idx];
    effect->SetDiffuseTex(mtrl.tex);
    effect->Use(renderer);
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  virtual void OnCreate() {}

  void Init() {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    azer::ShaderArray shaders;
    CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
    CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));

    effect_.reset(new TextureEffect(shaders.GetShaderVec(),
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

    camera_.SetPosition(azer::Vector3(0.0f, 2.0f, 3.0f));
    LoadMesh(::base::FilePath(::base::UTF8ToWide(kMeshPath)), &mesh_, rs);
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
    renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->ClearDepthAndStencil();
    effect_->SetPVW(camera_.GetProjViewMatrix());
    effect_->SetWorld(azer::Matrix4::kIdentity);
    Render(effect_.get(), renderer, &mesh_);
  }

  virtual void OnQuit() {}
 private:
  azer::Camera camera_;
  azer::Matrix4 proj_;
  azer::Matrix4 view_;
  Mesh mesh_;
  std::unique_ptr<TextureEffect> effect_;
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


