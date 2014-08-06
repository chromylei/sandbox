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
  MainDelegate()
      : tile_(256, 256)
      , x_(0.0f)
      , z_(0.0f) {
  }
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit();
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  azer::TexturePtr tex_[4];
  TerrainTile tile_;
  TerrainTexMap texmap_;
  float x_, z_;
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
  float speed = 100.0 * delta_time;
  if( ::GetAsyncKeyState('A') & 0x8000f ) {
    x_ -= speed;
  }
  if( ::GetAsyncKeyState('D') & 0x8000f ) {
    x_ += speed;
  }
  if( ::GetAsyncKeyState('W') & 0x8000f ) {
    z_ -= speed;
  }
  if( ::GetAsyncKeyState('S') & 0x8000f ) {
    z_ += speed;
  }

  
  azer::TexturePtr tmp[4];
  texmap_.GetTexture(x_, z_, tmp);
  for (int i = 0; i < 4; ++i) {
    tile_.SetTexture(i, tmp[i]);
  }
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  azer::Vector2 pos = std::move(texmap_.CalcTexViewpos(x_, z_));
  tile_.OnUpdate(pos.x, pos.y, renderer);
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
  tex_[0].reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath1));
  ::base::FilePath texpath2(::base::UTF8ToWide(kTerrainTex2));
  tex_[1].reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath2));
  ::base::FilePath texpath3(::base::UTF8ToWide(kTerrainTex3));
  tex_[2].reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath3));
  ::base::FilePath texpath4(::base::UTF8ToWide(kTerrainTex4));
  tex_[3].reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath4));

  tile_.Init(rs);
  int index = 0;
  for (int i = 0; i < 11; ++i) {
    for (int j = 0; j < 11; ++j) {
      texmap_.SetCell(i, j, tex_[index++ % arraysize(tex_)]);
    }
  }
}

