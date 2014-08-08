#include "azer/render/render.h"
#include "azer/math/math.h"
#include "azer/util/util.h"
#include "base/base.h"
#include "base/command_line.h"
#include "base/files/file_path.h"

#include "samples/common/common.h"
#include "sbox/base/camera_control.h"
#include "sbox/base/image.h"

#include "progtex.afx.h"
#include <tchar.h>

#define EFFECT_GEN_DIR "out/dbg/gen/sbox/clipmap/progtex/"
#define SHADER_NAME "progtex.afx"
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


    effect_.reset(new TextureEffect(shaders.GetShaderVec(),
                                    azer::RenderSystem::Current()));

    TextureEffect::Vertex v[] = {
      TextureEffect::Vertex(azer::Vector4(0.0f, 1.0f, 0.0f, 1.0f),
                            azer::Vector2(0.5f, 0.0f)),
      TextureEffect::Vertex(azer::Vector4(-1.0f, -1.0f, 0.0f, 1.0f),
                            azer::Vector2(0.0f, 1.0f)),
      TextureEffect::Vertex(azer::Vector4(1.0f, -1.0f, 0.0f, 1.0f),
                            azer::Vector2(1.0f, 1.0f)),
      
    };
    data_.reset(new azer::VertexData(effect_->GetVertexDesc(), ARRAYSIZE(v)));
    memcpy(data_->pointer(), (uint8*)v, sizeof(v));
    vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data_));
    renderer->SetViewport(azer::Renderer::Viewport(0, 0, 800, 600));
    CHECK(renderer->GetFrontFace() == azer::kCounterClockwise);
    CHECK(renderer->GetCullingMode() == azer::kCullBack);

    azer::Vector3 eye(0.0f, 0.0f, 3.0f);
    azer::Vector3 dir(0.0f, 0.0f, -1.0f);
    azer::Vector3 up(0.0f, 1.0f, 0.0f);
    view_ = std::move(LookDirRH(eye, dir, up));
    proj_ = std::move(
        PerspectiveRHD3D(azer::Degree(45.0f), 4.0f / 3.0f, 0.10f, 100.0f));

    sbox::Image image;
    CHECK(image.Load(FilePath(TEXT("samples/resources/texture/seafloor.dds"))));
    azer::Texture::Options texopt;
    texopt.width = image.width();
    texopt.height = image.height();
    texopt.target = azer::Texture::kShaderResource;
    tex_.reset(rs->CreateTexture(texopt));
    CHECK(tex_->InitFromData(image.GetDataPtr(), image.GetDataSize()));
          
    DCHECK(tex_.get() != NULL);
  }
  virtual void OnUpdateScene(double time, float delta_time) {
    float rspeed = 3.14f * 2.0f / 4.0f;
    azer::Radians rotate_speed(azer::kPI / 2.0);
    world_ = std::move(azer::RotateY(rotate_speed * time));
  }

  virtual void OnRenderScene(double time, float delta_time) {
    azer::RenderSystem* rs = azer::RenderSystem::Current();
    azer::Renderer* renderer = rs->GetDefaultRenderer();
    renderer->SetCullingMode(azer::kCullNone);
    azer::Matrix4 pvw = proj_ * view_ * world_;
    DCHECK(NULL != rs);
    renderer->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
    effect_->SetPVW(pvw);
    effect_->SetDiffuseTex(tex_);
    effect_->Use(renderer);
    renderer->Render(vb_.get(), azer::kTriangleList, 3, 0);
  }

  virtual void OnQuit() {}
 private:
  azer::Matrix4 proj_;
  azer::Matrix4 view_;
  azer::Matrix4 world_;
  azer::TexturePtr tex_;
  azer::VertexDataPtr  data_;
  azer::VertexBufferPtr vb_;
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


