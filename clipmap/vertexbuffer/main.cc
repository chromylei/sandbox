#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"

#include "diffuse.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/clipmap/vertexbuffer/"
#define SHADER_NAME "diffuse.afx"
using base::FilePath;

class MainDelegate : public azer::WindowHost::Delegate {
 public:
  virtual void OnCreate() {}

  void Init() {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    azer::ShaderArray shaders;
    CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
    CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));

    effect_.reset(new DiffuseEffect(shaders.GetShaderVec(),
                                    azer::RenderSystem::Current()));

    DiffuseEffect::Vertex v[] = {
      DiffuseEffect::Vertex(azer::Vector4( 0.0f, 1.0f, 0.5f, 1.0f ),
                            azer::Vector4( 1.0f, 0.0f, 0.0f, 1.0f )),
      DiffuseEffect::Vertex(azer::Vector4( -1.0f, -1.0f, 0.5f, 1.0f ),
                            azer::Vector4( 0.0f, 1.0f,  0.0f, 1.0f )),
      DiffuseEffect::Vertex(azer::Vector4( 1.0f, -1.0f, 0.5f, 1.0f ),
                            azer::Vector4( 0.0f, 0.0f, 1.0f, 1.0f )),
    };
    data_.reset(new azer::VertexData(effect_->GetVertexDesc(), ARRAYSIZE(v)));
    memcpy(data_->pointer(), (uint8*)v, sizeof(v));
    vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data_));
    renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
    CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
    CHECK(renderer->GetCullingMode() == azer::kCullBack);
  }
  virtual void OnUpdateScene(double time, float delta_time) {}

  virtual void OnRenderScene(double time, float delta_time) {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    DCHECK(NULL != rs);
    renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    effect_->Use(renderer);
    renderer->Render(vb_.get(), azer::kTriangleList, 3, 0);
  }

  virtual void OnQuit() {}
 private:
  azer::VertexDataPtr  data_;
  azer::VertexBufferPtr vb_;
  std::unique_ptr<DiffuseEffect> effect_;
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

