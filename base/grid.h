#pragma once

#include "azer/render/render.h"
#include "sbox/base/heightmap.h"

class GridEffect : public azer::Effect {
 public:
  GridEffect(azer::RenderSystem* rs)
      : azer::Effect(rs) {
    Init(rs);
  }

#pragma pack(push, 4)
  struct vs_cbuffer {
    azer::Matrix4 vp;
    azer::Matrix4 world;
  };
  struct ps_cbuffer {
    azer::Vector4 diffuse;
  };
#pragma pack(pop)
  
  azer::VertexDescPtr GetVertexDesc() {
    DCHECK(NULL != technique_.get());
    return technique_->GetVertexDesc();
  }

  void SetWVP(const azer::Matrix4& wvp) {
    azer::GpuConstantsTable* tb = gpu_table_[(int)azer::kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, (void*)&wvp,  sizeof(azer::Matrix4));
  }

  void SetWorld(const azer::Matrix4& wvp) {
    azer::GpuConstantsTable* tb = gpu_table_[(int)azer::kVertexStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(1, (void*)&wvp,  sizeof(azer::Matrix4));
  }

  void SetDiffuse(const azer::Vector4& value) {
    azer::GpuConstantsTable* tb = gpu_table_[(int)azer::kPixelStage].get();
    DCHECK(tb != NULL);
    tb->SetValue(0, (void*)&value,  sizeof(azer::Vector4));
  }

  void SetTechnique(azer::TechniquePtr ptr) { technique_ = ptr;}
 private:
  virtual void UseTexture(azer::Renderer* renderer) OVERRIDE {}
  void Init(azer::RenderSystem* rs);
};

class Grid {
 public:
  Grid(azer::RenderSystem* rs, azer::TechniquePtr ptr) {
    effect_.reset(new GridEffect(rs));
    effect_->SetTechnique(ptr);
  }

  GridEffect* effect() { return effect_.get();}

  void Init(const base::FilePath& filepath);
  void Render(azer::Renderer* rs);
  
  azer::VertexDescPtr desc() { return vb_->desc();}
 private:
  void InitVertex();

  std::unique_ptr<GridEffect> effect_;
  azer::TexturePtr tex_;
  azer::VertexBufferPtr vb_;
  azer::IndicesBufferPtr ib_;
  HeightMapInfo height_map_;
};

typedef std::shared_ptr<Grid> GridPtr;
