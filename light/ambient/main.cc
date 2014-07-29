#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/base/base.h"

#include "ambient.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/light/ambient/"
#define SHADER_NAME "ambient.afx"
using base::FilePath;

#define kSphereMeshPath "sbox/res/sphere.3DS"
#define kTeaportMeshPath "sbox/res/teaport.3DS"
#define kTorusMeshPath "sbox/res/torus.3DS"
#define kPlaneMeshPath "sbox/res/plane.3DS"
#define kXYZMeshPath "sbox/res/xyz.x3d"

void Render(AmbientEffect* effect, azer::Renderer* renderer, Mesh* mesh) {
  for (uint32 i = 0; i < mesh->rgroups().size(); ++i) {
    Mesh::RenderGroup rg = mesh->rgroups()[i];
    azer::VertexBuffer* vb = rg.vb.get();
    azer::IndicesBuffer* ib = rg.ib.get();
    effect->SetDiffuse(azer::Vector4(0.8f, 0.7f, 0.6f, 1.0f));
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

    effect_.reset(new AmbientEffect(shaders.GetShaderVec(),
                                    azer::RenderSystem::Current()));

    renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
    CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
    CHECK(renderer->GetCullingMode() == azer::kCullBack);
    renderer->EnableDepthTest(true);
    // renderer->SetCullingMode(azer::kCullNone);
    // renderer->SetFillMode(azer::kWireFrame);
    proj_ = std::move(
        PerspectiveRHD3D(azer::Degree(45.0f), 4.0f / 3.0f, 0.10f, 100.0f));

    camera_.SetPosition(azer::Vector3(0.0f, 1.0f, 2.0f));
    LoadMesh(::base::FilePath(::base::UTF8ToWide(kTeaportMeshPath)), &teaport_, rs);
    LoadMesh(::base::FilePath(::base::UTF8ToWide(kPlaneMeshPath)), &plane_, rs);

    light_.direction = azer::Vector4(0.5f, -0.5f, 0.0f, 1.0f);
    light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    light_.ambient = azer::Vector4(0.1f, 0.0f, 0.0f, 1.0f);
  }

  virtual void OnUpdateScene(double time, float delta_time) {
    float rspeed = 3.14f * 2.0f / 4.0f;
    azer::Radians camera_speed(azer::kPI / 2.0f);
    UpdatedownCamera(&camera_, camera_speed, delta_time);
    azer::Matrix4 scale_mat = azer::Scale(azer::Vector3(0.3f, 0.3f, 0.3f));
    azer::Matrix4 rotate1 = azer::RotateX(azer::Degree(90.0f));
    azer::Matrix4 rotate2 = azer::RotateY(azer::Radians(rspeed * time));
    azer::Matrix4 pos = azer::Translate(azer::Vector3(0.0f, 0.0f, 0.0f));
    teaport_world_ = pos * rotate2 * rotate1 * scale_mat;
  }

  virtual void OnRenderScene(double time, float delta_time) {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    // renderer->SetCullingMode(azer::kCullNone);
    DCHECK(NULL != rs);
    renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    renderer->ClearDepthAndStencil();

    azer::Matrix4 pvw;
    pvw = camera_.GetProjViewMatrix() * teaport_world_;
    effect_->SetPointLight(light_);
    effect_->SetDiffuse(azer::Vector4(0.4f, 0.4f, 0.4f, 1.0f));
    effect_->SetPVW(pvw);
    effect_->SetWorld(teaport_world_);
    Render(effect_.get(), renderer, &teaport_);

    azer::Matrix4 world = azer::RotateX(azer::Degree(90.0f));
    pvw = camera_.GetProjViewMatrix() * world;
    effect_->SetDiffuse(azer::Vector4(0.0f, 1.0f, 0.0f, 1.0f));
    effect_->SetPVW(pvw);
    effect_->SetWorld(world);
    Render(effect_.get(), renderer, &plane_);
  }

  virtual void OnQuit() {}
 private:
  azer::Camera camera_;
  azer::Matrix4 proj_;
  azer::Matrix4 teaport_world_;
  Mesh teaport_;
  Mesh plane_;
  std::unique_ptr<AmbientEffect> effect_;
  AmbientEffect::PointLight light_;
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


