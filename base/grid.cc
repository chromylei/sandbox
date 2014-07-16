#include "sbox/base/grid.h"


using azer::GpuConstantsTable;

void GridEffect::Init(azer::RenderSystem* rs) {
  GpuConstantsTable::Desc vs_table_desc[] = {
    GpuConstantsTable::Desc("vp", azer::GpuConstantsType::kMatrix4,
                            offsetof(GridEffect::vs_cbuffer, vp), 1),
    GpuConstantsTable::Desc("vp", azer::GpuConstantsType::kMatrix4,
                            offsetof(GridEffect::vs_cbuffer, world), 1), 
  };

  gpu_table_[azer::kVertexStage].reset(rs->CreateGpuConstantsTable(
      arraysize(vs_table_desc), vs_table_desc));
  GpuConstantsTable::Desc ps_table_desc[] = {
    GpuConstantsTable::Desc("diffuse", azer::GpuConstantsType::kVector4,
                            offsetof(GridEffect::ps_cbuffer, diffuse), 1), 
  };
  gpu_table_[azer::kPixelStage].reset(rs->CreateGpuConstantsTable(
      arraysize(ps_table_desc), ps_table_desc));
}

struct Vertex {
  azer::Vector3 position;
  azer::Vector2 tex;
  azer::Vector3 normal;

  Vertex(const azer::Vector3& p, const azer::Vector2& t, const azer::Vector3& n)
      : position(p), tex(t), normal(n) {
  }
};

void ComputeNormal(Vertex* v, int vertex_num, DWORD* indices, int indices_num) {
  std::unique_ptr<uint32> used(new uint32[vertex_num]);
  memset(used.get(), 0, sizeof(uint32) * vertex_num);
  for (int i = 0; i < indices_num; i+=3) {
    Vertex* p1 = v + indices[i];
    Vertex* p2 = v +indices[i+1];
    Vertex* p3 = v + indices[i+2];
    used.get()[indices[i]]++;
    used.get()[indices[i+1]]++;
    used.get()[indices[i+2]]++;
    azer::Vector3 normal = azer::CalcPlaneNormal(p1->position,
                                                 p2->position,
                                                 p3->position);
    p1->normal += normal;
    p2->normal += normal;
    p3->normal += normal;
  }

  for (int i = 0; i < vertex_num; ++i) {
    v[i].normal /= (float)used.get()[i];
  }
}

void Grid::Init(const base::FilePath& filepath) {
  azer::RenderSystem* rs = azer::RenderSystem::Current();
  CHECK(HeightMapLoad(filepath, &height_map_));
  InitVertex();
}


void Grid::InitVertex() {
  const int kRow = height_map_.width;
  const int kColumn = height_map_.height;
  const int kVertexNum = kRow * kColumn;

  azer::RenderSystem* rs = azer::RenderSystem::Current();
  azer::VertexDataPtr data(new azer::VertexData(effect_->GetVertexDesc(),
                                                kRow * kColumn));
  Vertex* begin_ptr = (Vertex*)data->pointer();
  Vertex* curr = begin_ptr;
  for (int i = 0; i < kColumn; ++i) {
    for (int j = 0; j < kRow; ++j) {
      int idx = i * kRow + j;
      curr->position = azer::vec3((float)i, height_map_.map[idx].y, (float)j);
      curr->tex = azer::Vector2(100.0f * j / (float)kRow,
                                100.0f * i / (float)kColumn);
      curr->normal = azer::vec3(0.0f, 1.0f, 0.0f);
      curr++;
    }
  }

  const int kTriangleNum = (kRow - 1) * (kColumn - 1) * 2;
  using azer::IndicesData;
  azer::IndicesDataPtr idata(new IndicesData(kTriangleNum * 3,
                                             IndicesData::kUint32,
                                             IndicesData::kMainMemory));
  int32* cur = (int32*)idata->pointer();
  for (int i = 0; i < kColumn-1; ++i) {
    for (int j = 0; j < kRow-1; ++j) {
      *cur++ = i * kRow + j;
      *cur++ = (i + 1) * kRow + j + 1;
      *cur++ = (i + 1) * kRow + j;
      
      *cur++ = i * kRow + j;
      *cur++ = i * kRow + j + 1;
      *cur++ = (i + 1) * kRow + j + 1;
    }
  }

  ::ComputeNormal((Vertex*)data->pointer(), kColumn * kRow,
                (DWORD*)idata->pointer(), kTriangleNum * 3);

  Vertex* v = (Vertex*)data->pointer();
  vb_.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), data));
  ib_.reset(rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata));
}


void Grid::Render(azer::Renderer* rs) {
  const int kRow = height_map_.width;
  const int kColumn = height_map_.height;
  const int kTriangleNum = (kRow - 1) * (kColumn - 1) * 2;
  rs->SetCullingMode(azer::kCullNone);
  rs->Render(vb_.get(), ib_.get(), azer::kTriangleList, kTriangleNum * 3);
}

