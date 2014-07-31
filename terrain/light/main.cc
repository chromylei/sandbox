#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "sbox/terrain/base/grid.h"
#include "sbox/base/coord.h"
#include "sbox/base/base.h"

#include <tchar.h>
#include "diffuse.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/terrain/heightmap/"
#define SHADER_NAME "diffuse.afx"

const char* kHeightmapPath = "sbox/terrain/res/heightmap01.bmp";
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
  float rspeed = 3.14f * 2.0f / 4.0f;
  azer::Radians camera_speed(azer::kPI / 2.0f);
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
  effect_->SetDiffuse(azer::Vector4(0.8f, 0.7f, 0.6f, 1.0f));
  effect_->Use(renderer);
  renderer->Render(vb_.get(), ib_.get(), azer::kTriangleList);

  azer::Matrix4 pvw = camera_.GetProjViewMatrix() * coord_world_;
  coord_.Render(renderer, coord_world_, camera_.GetProjViewMatrix());
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
    InitPhysicsBuffer(rs);

    camera_.SetPosition(azer::Vector3(0.0f, 8.0f, 10.0f));
    camera_.SetLookAt(azer::Vector3(0.0f, 8.0f, 0.0f));
    coord_.Init(rs);
  }

void MainDelegate::InitPhysicsBuffer(azer::RenderSystem* rs) {
  azer::VertexDataPtr vdata(
      new azer::VertexData(effect_->GetVertexDesc(), grid_.vertices().size()));
  DiffuseEffect::Vertex* v = (DiffuseEffect::Vertex*)vdata->pointer();
  for (uint32 i = 0; i < grid_.vertices().size(); ++i) {
    const Grid::Vertex& vertex = grid_.vertices()[i];
    v->position = azer::Vector4(vertex.position, 1.0f);
    v->coordtex = vertex.tex;
    v->normal = azer::Vector4(vertex.normal, 0.0f);
    v++;
  }

  azer::IndicesDataPtr idata_ptr(
      new azer::IndicesData(grid_.indices().size(), azer::IndicesData::kUint32,
                            azer::IndicesData::kMainMemory));
  memcpy(idata_ptr->pointer(), &(grid_.indices()[0]),
         sizeof(int32) * grid_.indices().size());

  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
}
