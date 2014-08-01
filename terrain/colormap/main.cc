#include <limits>

#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/terrain/base/grid.h"
#include "sbox/base/coord.h"
#include "sbox/base/image.h"
#include "sbox/base/base.h"

#include <tchar.h>
#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/terrain/colormap/"
#define SHADER_NAME "diffuse.afx"
const float kRepeatNum = 10.0f;

const char* kHeightmapPath = "sbox/terrain/res/heightmap01.bmp";
const char* kColorMapPath  = "sbox/terrain/res/colorm01.bmp";
const char* kTexPath       = "sbox/terrain/res/dirt01.dds";
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  virtual void OnCreate() {}

  void Init();
  virtual void OnUpdateScene(double time, float delta_time);
  virtual void OnRenderScene(double time, float delta_time);
  virtual void OnQuit();
 private:
  void InitPhysicsBuffer(azer::RenderSystem* rs);
  azer::Camera camera_;
  Grid grid_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  std::unique_ptr<DiffuseEffect> effect_;
  CoordMesh coord_;
  azer::Matrix4 coord_world_;
  DiffuseEffect::DirLight light_;
  azer::TexturePtr texptr_;
  sbox::Image image_;
  float top_y_;
  float bottom_y_;
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

  azer::Vector3 pos = camera_.position() +
      camera_.direction().NormalizeCopy() * 3.0f;
  coord_world_ = azer::Translate(pos);
}

void MainDelegate::OnRenderScene(double time, float delta_time) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetCullingMode(azer::kCullNone);
  DCHECK(NULL != rs);
  renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
  renderer->ClearDepthAndStencil();

  azer::Matrix4 world = azer::Translate(-50.0f, 0.0f, -50.0f);
  effect_->SetWorld(world);
  effect_->SetPVW(std::move(camera_.GetProjViewMatrix() * world));
  // effect_->SetDiffuse(azer::Vector4(0.5f, 0.5f, 0.6f, 1.0f));
  effect_->SetDiffuseTex(texptr_);
  effect_->SetDirLight(light_);
  effect_->Use(renderer);
  renderer->Render(vb_.get(), ib_.get(), azer::kTriangleList);

  // azer::Matrix4 pvw = camera_.GetProjViewMatrix() * coord_world_;
  // coord_.Render(renderer, coord_world_, camera_.GetProjViewMatrix());
}

void MainDelegate::OnQuit() {
}

void MainDelegate::Init() {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::Renderer* renderer = rs->GetDefaultRenderer();
  renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
  CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
  CHECK(renderer->GetCullingMode() == azer::kCullBack);
  renderer->EnableDepthTest(true);
  // renderer->SetFillMode(azer::kWireFrame);
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  effect_.reset(new DiffuseEffect(shaders.GetShaderVec(), rs));
  grid_.Init(::base::FilePath(::base::UTF8ToWide(kHeightmapPath)));

  camera_.SetPosition(azer::Vector3(0.0f, 8.0f, 10.0f));
  camera_.SetLookAt(azer::Vector3(0.0f, 8.0f, 0.0f));
  coord_.Init(rs);

  light_.dir = azer::Vector4(0.0f, -0.3f, 0.75f, 1.0f);
  light_.diffuse = azer::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
  light_.ambient = azer::Vector4(0.15f, 0.15f, 0.15f, 1.0f);

  ::base::FilePath texpath(::base::UTF8ToWide(kTexPath));
  texptr_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath));
  CHECK(image_.Load(::base::FilePath(::base::UTF8ToWide(kColorMapPath))));

  InitPhysicsBuffer(rs);
}

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  top_y_ = std::numeric_limits<float>::min();
  bottom_y_ = std::numeric_limits<float>::max();;
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), grid_.vertices().size()));
  DiffuseEffect::Vertex* v = (DiffuseEffect::Vertex*)vdata->pointer();
  float cell_width = 1.0f / (float)grid_.width();
  float cell_height = 1.0f / (float)grid_.height();
  for (int i = 0; i < grid_.height(); ++i) {
    for (int j = 0; j < grid_.width(); ++j) {
      int index = i * grid_.width() + j;
      const Grid::Vertex& vertex = grid_.vertices()[index];
      v->position = azer::Vector4(vertex.position, 1.0f) * 0.05f;
      float tu = cell_width * j;
      float tv = cell_height * i;
      // v->coordtex = azer::Vector2(u * kRepeatNum, v * kRepeatNum);
      v->normal = azer::Vector4(vertex.normal, 0.0f);
      v->color = sbox::SampleImage(tu, tv, &image_);
      v++;

      if (v->position.y > top_y_) { top_y_ = v->position.y;}
      if (v->position.y < bottom_y_) { bottom_y_ = v->position.y;}
    }
  }

  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(grid_.indices().size(), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr->pointer(), &(grid_.indices()[0]),
         sizeof(int32) * grid_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
}
