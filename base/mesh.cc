#include "sbox/base/mesh.h"

#include "sbox/base/assimp.h"
#include "azer/render/render.h"

namespace {
const azer::VertexDesc::Desc kVertexDesc[] = {
  {"POSITION", 0, azer::kVec3},
  {"COORDTEX", 0, azer::kVec2},
  {"NORMAL", 0, azer::kVec3},
};
}

void LoadVertex(const aiMesh* paiMesh, Mesh::Group* group) {
  const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
  for (uint32 i = 0; i < paiMesh->mNumVertices; ++i) {
    const aiVector3D& pos = paiMesh->mVertices[i];
    const aiVector3D& normal = paiMesh->mNormals[i];
    const aiVector3D& texcoord =
        paiMesh->HasTextureCoords(0) ? (paiMesh->mTextureCoords[0][i]) : zero3d;

    Mesh::Vertex vertex(azer::Vector3(pos.x, pos.y, pos.z),
                        azer::Vector2(texcoord.x, texcoord.y),
                        azer::Vector3(normal.x, normal.y, normal.z));
    group->vertices.push_back(vertex);
  }

  for (uint32 i = 0; i < paiMesh->mNumFaces; ++i) {
    const aiFace& face = paiMesh->mFaces[i];
    group->indices.push_back(face.mIndices[0]);
    group->indices.push_back(face.mIndices[1]);
    group->indices.push_back(face.mIndices[2]);
  }

  group->mtrl_idx = paiMesh->mMaterialIndex;
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

bool LoadMesh(const ::base::FilePath& filepath, Mesh* mesh, azer::RenderSystem* rs) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      ::base::WideToUTF8(filepath.value()),
      aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
  if (scene == NULL) {
    LOG(ERROR) << "Failed to load file: " << filepath.value();
    return false;
  }

  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    Mesh::Group group;
    LoadVertex(scene->mMeshes[i], &group);
    mesh->mutable_groups()->push_back(group);
  }

  for (uint32 i = 0; i < scene->mNumMaterials; ++i) {
    const aiMaterial* material = scene->mMaterials[i];
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString path;
      if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path,
                               NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
        ::base::FilePath texpath(::base::UTF8ToWide(path.C_Str()));
        ::base::FilePath realpath;
        realpath = filepath.DirName();
        realpath = realpath.Append(texpath.BaseName());
        azer::TexturePtr texptr(rs->CreateTextureFromFile(azer::Texture::k2D,
                                                          realpath));
        Mesh::Material mtrl;
        mtrl.tex = texptr;
        mesh->mutable_materials()->push_back(mtrl);
      }
    }
  }

  mesh->Init(rs);
  return true;
}
