#include <limits>

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/terrain/base/grid.h"
#include "sbox/base/coord.h"
#include "sbox/base/base.h"
#include "sbox/clipmap/mergetex/terrain_tile.h"

#include <tchar.h>
const char* kTerrainTex1 = "sbox/clipmap/res/height1.bmp";
const char* kTerrainTex2 = "sbox/clipmap/res/height2.bmp";
const char* kTerrainTex3 = "sbox/clipmap/res/height3.bmp";
const char* kTerrainTex4 = "sbox/clipmap/res/height4.bmp";
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  MainDelegate() : tile_(256, 256) {}
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit();
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  azer::TexturePtr tex1_;
  azer::TexturePtr tex2_;
  azer::TexturePtr tex3_;
  azer::TexturePtr tex4_;
  TerrainTile tile_;
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

void MainDelegate::OnUpdateScene(double time, float delta_time) {
  float rspeed = 3.14f * 2.0f;
  azer::Radians camera_speed(4.0 * azer::kPI / 2.0f);
  UpdatedownCamera(&camera_, camera_speed, delta_time);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();
  tile_.OnUpdate(0.5f, 0.5f, renderer);
}

void MainDelegate::OnQuit() {
}

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  renderer->EnableDepthTest(false);

  ::base::FilePath texpath1(::base::UTF8ToWide(kTerrainTex1));
  tex1_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath1));
  ::base::FilePath texpath2(::base::UTF8ToWide(kTerrainTex2));
  tex2_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath2));
  ::base::FilePath texpath3(::base::UTF8ToWide(kTerrainTex3));
  tex3_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath3));
  ::base::FilePath texpath4(::base::UTF8ToWide(kTerrainTex4));
  tex4_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath4));

  tile_.Init(rs);
  tile_.SetTexture(0, tex1_);
  tile_.SetTexture(1, tex2_);
  tile_.SetTexture(2, tex3_);
  tile_.SetTexture(3, tex4_);
}

