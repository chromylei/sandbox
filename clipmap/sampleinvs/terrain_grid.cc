#include "sbox/clipmap/sampleinvs/terrain_grid.h"

#include "base/files/file_path.h"

const char* kTerrainTex1 = "sbox/clipmap/res/height1.bmp";
#include "grid_tile.afx.h"
#define EFFECT_GEN_DIR "out/dbg/gen/sbox/clipmap/sampleinvs/"
#define SHADER_NAME "grid_tile.afx"


GridTile::GridTile(float cell, int cellnum, float x, float z)
    : kCellWidth(cell)
    , kCellNum(cellnum)
    , orgx_(x)
    , orgz_(z) {
}

bool GridTile::Init(azer::RenderSystem* rs) {
  azer::ShaderArray shaders;
  CHECK(azer::LoadVertexShader(EFFECT_GEN_DIR SHADER_NAME ".vs", &shaders));
  CHECK(azer::LoadPixelShader(EFFECT_GEN_DIR SHADER_NAME ".ps", &shaders));
  GridTileEffect* effect = new GridTileEffect(shaders.GetShaderVec(), rs);
  effect_.reset(effect);

  CreateVertex(rs, effect);

  ::base::FilePath texpath1(::base::UTF8ToWide(kTerrainTex1));
  texptr_.reset(rs->CreateTextureFromFile(azer::Texture::k2D, texpath1));
  return true;
}

void GridTile::CreateVertex(azer::RenderSystem* rs, GridTileEffect* effect) {
  const int vertex_num = (kCellNum + 1) * (kCellNum + 1);
  azer::VertexBuffer::Options vbopt;
  azer::VertexDescPtr vdesc_ptr(effect->GetVertexDesc());
  azer::VertexDataPtr vdata(new azer::VertexData(vdesc_ptr, vertex_num));
  GridTileEffect::Vertex* vertices = (GridTileEffect::Vertex*)vdata->pointer();
  float x = orgx_ - kCellWidth * kCellNum / 2.0f;
  float z = orgz_ - kCellWidth * kCellNum / 2.0f;
  int index = 0;
  for (int i = 0; i < kCellNum + 1; ++i) {
    for (int j = 0; j < kCellNum + 1; ++j) {
      vertices[index++] = GridTileEffect::Vertex(azer::Vector4(x, 0.0f, z, 1.0));
      x += kCellWidth;
    }
    z += kCellWidth;
    x = orgx_ - kCellWidth * kCellNum / 2.0f;
  }

  vb_.reset(rs->CreateVertexBuffer(vbopt, vdata));

  const int kTriangleNum = kCellNum * kCellNum * 2;
  azer::IndicesDataPtr idata_ptr(new azer::IndicesData(
      kTriangleNum * 3, azer::IndicesData::kUint32, azer::IndicesData::kMainMemory));
  uint32* indices = (uint32*)idata_ptr->pointer();
  const int strip = kCellNum + 1;
  for (int i = 0; i < kCellNum; ++i) {
    for (int j = 0; j < kCellNum; ++j) {
      *indices++ = i * strip + j;
      *indices++ = i * strip + j + 1;
      *indices++ = (i + 1) * strip + j;
      *indices++ = i * strip + j + 1;
      *indices++ = (i + 1) * strip + j + 1;
      *indices++ = (i + 1) * strip + j;
    }
  }
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
}

void GridTile::Render(azer::Renderer* renderer, const azer::Camera& camera) {
  const azer::Matrix4& pvw = camera.GetProjViewMatrix();
  GridTileEffect* effect = (GridTileEffect*)effect_.get();
  effect->SetRange(azer::Vector4(-kCellWidth * kCellNum * 0.5f,
                                 -kCellWidth * kCellNum * 0.5f,
                                 kCellWidth * kCellNum,
                                 kCellWidth * kCellNum));
  effect->SetHeightTex(texptr_);
  effect->SetPVW(pvw);
  effect_->Use(renderer);
  renderer->DrawIndex(vb_.get(), ib_.get(), azer::kTriangleList);
}

