#include "sbox/clipmap/mergetex/terrain_tile.h"

#include "tex_merge.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/clipmap/mergetex/"
#define SHADER_NAME "tex_merge.afx"

bool TerrainTile::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  TexMergeEffect* effect = new TexMergeEffect(shaders.GetShaderVec(), rs);
  effect_.reset(effect);

  azer::Texture::SamplerState sampler;
  sampler.mag_filter = azer::Texture::kLinear;
  sampler.min_filter = azer::Texture::kLinear;
  sampler.mipmag_filter = azer::Texture::kLinear;
  sampler.mipmin_filter = azer::Texture::kLinear;
  azer::Texture::Options texopt;
  texopt.width = kWidth;
  texopt.height = kHeight;
  texopt.sampler = sampler;
  texopt.format = azer::kRGBAf;
  texopt.sampler.wrap_u = azer::Texture::kClamp;
  texopt.sampler.wrap_v = azer::Texture::kClamp;
  texopt.sampler.wrap_w = azer::Texture::kClamp;
  texopt.sampler.compare_func = azer::CompareFunc::kLessEqual;
  texopt.target = (azer::Texture::BindTarget)
      (azer::Texture::kRenderTarget | azer::Texture::kShaderResource);
  // heightmap_renderer_.reset(rs->CreateRenderer(texopt));
  texvec_.resize(4u);

  TexMergeEffect::Vertex vertex[] = {
    TexMergeEffect::Vertex(azer::Vector4(-1.0f, 1.0f, 0.5f, 1.0f)),
    TexMergeEffect::Vertex(azer::Vector4(-1.0f, -1.0f, 0.5f, 1.0f)),
    TexMergeEffect::Vertex(azer::Vector4(1.0f, 1.0f, 0.5f, 1.0f)),
    TexMergeEffect::Vertex(azer::Vector4(1.0f, 1.0f, 0.5f, 1.0f)),
    TexMergeEffect::Vertex(azer::Vector4(-1.0f, -1.0f, 0.5f, 1.0f)),
    TexMergeEffect::Vertex(azer::Vector4(1.0f, -1.0f, 0.5f, 1.0f)),
  };
  azer::VertexBuffer::Options vbopt;
  azer::VertexDescPtr vdesc_ptr(effect->GetVertexDesc());
  azer::VertexDataPtr vdata(new azer::VertexData(vdesc_ptr, arraysize(vertex)));
  memcpy(vdata->pointer(), vertex, sizeof(vertex));
  vb_.reset(rs->CreateVertexBuffer(vbopt, vdata));

  return true;
}

void TerrainTile::SetTexture(int index, azer::TexturePtr ptr) {
  TexMergeEffect* effect = (TexMergeEffect*)effect_.get();
  switch (index) {
    case 0: effect->SetHeightTex1(ptr); break;
    case 1: effect->SetHeightTex2(ptr); break;
    case 2: effect->SetHeightTex3(ptr); break;
    case 3: effect->SetHeightTex4(ptr); break;
    default: NOTREACHED();
  }
}

void TerrainTile::OnUpdate(float x, float y, azer::Renderer* renderer) {
  /*
  sm_renderer_->Use();
  sm_renderer_->SetViewport(azer::Renderer::Viewport(0, 0, kWidth, kHeight));
  sm_renderer_->Clear(azer::Vector4(0.0f, 0.0f, 0.0f, 0.0f));
  sm_renderer_->EnableDepthTest(false);
  */
  TexMergeEffect* effect = (TexMergeEffect*)effect_.get();
  effect->SetViewpos(azer::Vector4(x, y, 0.0f, 1.0f));
  effect->Use(renderer);
  renderer->Render(vb_.get(), azer::kTriangleList);
}

bool TerrainTexMap::GetTexture(float x, float z, azer::TexturePtr* ptrarr) {
  int xcell = floor(x / kCellWidth) + 5;
  int zcell = floor(z / kCellWidth) + 5;
  if (xcell >= 1 && xcell < 10 && zcell > 0 && zcell < 10) {
    ptrarr[0] = map_[xcell - 1][zcell - 1];
    ptrarr[1] = map_[xcell][zcell - 1];
    ptrarr[2] = map_[xcell][zcell];
    ptrarr[3] = map_[xcell - 1][zcell];
    return true;
  } else {
    return false;
  }
}

azer::Vector2 TerrainTexMap::CalcTexViewpos(float x, float z) {
  float orgx = (x - floor(x / kCellWidth) * kCellWidth) / kCellWidth;
  float orgy = (z - floor(z / kCellWidth) * kCellWidth) / kCellWidth;
  if (orgx <= 0.5) {
    orgx = 0.5 - orgx;
  } else if (orgx > 0.5f && orgx <= 1.0f) {
    orgx = 1.5 - orgx;
  }

  if (orgy <= 0.5) {
    orgy = 0.5 - orgy;
  } else if (orgy > 0.5f && orgy <= 1.0f) {
    orgy = 1.5 - orgx;
  }

  return azer::Vector2(orgx, orgy);
}
