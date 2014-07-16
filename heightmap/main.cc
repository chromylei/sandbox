#include "azer/azer.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/strings/stringprintf.h"
#include "azer/util/util.h"
#include "sbox/base/grid.h"
#include "sbox/base/camera_control.h"
#include "diffuse.h"
#include <tchar.h>

#define EFFECT_GEN_DIR AZER_LITERAL("out/dbg/gen/sbox/heightmap/")
#define VS_SHADER_NAME AZER_LITERAL("grid.vs")
#define PS_SHADER_NAME AZER_LITERAL("grid.ps")
#define RASTERTEK_RES_DIR AZER_LITERAL("sbox/res")

using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : culling_num_(0) {}
  void Init();
  virtual void OnCreate() {}
  virtual void OnUpdateScene(double time, float delta_time) OVERRIDE;
  virtual void OnRenderScene(double time, float delta_time) OVERRIDE;
  virtual void OnQuit() OVERRIDE {};
 private:
  int culling_num_;
  azer::Camera camera_;
  GridPtr grid_;
  std::unique_ptr<azer::ResourceManager> res_mgr_;
};

int main(int argc, char* argv[]) {
  ::base::InitApp(&argc, &argv, "");

  ::azer::FileSystem::InitDefaultFileSystem(AZER_LITERAL("azer/resources/common"));

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

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  azer::TechniquePtr terrain_effect = res_mgr_->GetTechnique(
      AZER_LITERAL("technique/heightmap.tech"), rs);
  grid_.reset(new Grid(rs, terrain_effect));
  grid_->Init(::base::FilePath(
      AZER_LITERAL("sandbox/rastertek/media/heightmap01.bmp")));

  renderer->EnableDepthTest(true);
  renderer->SetCullingMode(azer::kCullNone);
  camera_.SetPosition(azer::Vector3(0.0f, 3.0f, 0.0f));
}

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
  CameraControl(&camera_, camera_speed, delta_time);

  grid_->effect()->SetWVP(camera_.GetProjViewMatrix());
  grid_->effect()->SetWorld(azer::Matrix4::kIdentity);
  grid_->effect()->SetDiffuse(azer::Vector4(0.8f, 0.8f, 0.8f, 1.0f));
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  DCHECK(NULL != rs);
  azer::Renderer* renderer = rs->GetDefaultRenderer();

  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  renderer->SetCullingMode(azer::kCullNone);

  grid_->Render(renderer);
}

