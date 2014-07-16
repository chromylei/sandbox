#include "sbox/base/mesh.h"
#include "azer/render/render.h"

namespace {
const azer::VertexDesc::Desc kVertexDesc[] = {
  {"POSITION", 0, azer::kVec3},
  {"COORDTEX", 0, azer::kVec2},
  {"NORMAL", 0, azer::kVec3},
};

}
void Mesh::Init(azer::RenderSystem* rs) {
  azer::VertexDescPtr vertex_desc_ptr(
      new azer::VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  for (uint32 i = 0; i < groups_.size(); ++i) {
    RenderGroup rgroup;
    const int vertex_num = groups_[i].vertices.size();
    azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, vertex_num)); 
    memcpy(vdata->pointer(), (uint8*)&(groups_[i].vertices[0]),
           sizeof(Mesh::Vertex) * vertex_num);
    rgroup.vb.reset(rs->CreateVertexBuffer(azer::VertexBuffer::Options(), vdata));

    azer::IndicesDataPtr idata_ptr(
        new azer::IndicesData(vertex_num, azer::IndicesData::kUint32,
                              azer::IndicesData::kMainMemory));
    memcpy(idata_ptr->pointer(), &(groups_[i].indices[0]),
           groups_[i].indices.size() * sizeof(uint32));
    rgroup.ib.reset(
        rs->CreateIndicesBuffer(azer::IndicesBuffer::Options(), idata_ptr));
    rgroup.mtrl_idx = groups_[i].mtrl_idx;
    rgroups_.push_back(rgroup);
  }
}

void Mesh::Render(azer::Effect* effect, azer::Renderer* renderer) {
  for (uint32 i = 0; i < rgroups_.size(); ++i) {
    azer::VertexBuffer* vb = rgroups_[i].vb.get();
    azer::IndicesBuffer* ib = rgroups_[i].ib.get();
    renderer->Render(vb, ib, azer::kTriangleList);
  }
}  // namespace azer
