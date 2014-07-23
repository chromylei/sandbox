#include "sbox/base/hardware_skinned_mesh.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "azer/render/render.h"

int GetNextIndex(const HardwareSkinnedMesh::Vertex& vertex) {
  for (int i = 0; i < 4; ++i) {
    if (vertex.index[i] == -1) {
      return i;
    }
  }

  return -1;
}

void HardwareSkinnedMesh::LoadVertex(const aiMesh* paiMesh, Group* group) {
  const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
  for (uint32 i = 0; i < paiMesh->mNumVertices; ++i) {
    const aiVector3D& pos = paiMesh->mVertices[i];
    const aiVector3D& normal = paiMesh->mNormals[i];
    const aiVector3D& texcoord =
        paiMesh->HasTextureCoords(0) ? (paiMesh->mTextureCoords[0][i]) : zero3d;

    Vertex vertex(azer::Vector4(pos.x, pos.y, pos.z, 1.0),
                  azer::Vector2(texcoord.x, texcoord.y),
                  azer::Vector4(normal.x, normal.y, normal.z, 0.0));
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

void HardwareSkinnedMesh::LoadBoneWeights(const aiMesh* paiMesh,
                                          std::vector<Vertex>* vertices,
                                          OffsetType* offsets) {
  for (uint32 i = 0; i < paiMesh->mNumBones; ++i) {
    std::string bone_name = paiMesh->mBones[i]->mName.data;
    int bone_index = skeleton_.GetBoneIndex(bone_name);
    Bone* bone = skeleton_.GetBone(bone_index);
    aiMatrix4x4 mat = paiMesh->mBones[i]->mOffsetMatrix;
    azer::Matrix4 offset = azer::Matrix4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                                         mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                                         mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                                         mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    offsets->push_back(std::make_pair(bone_index, offset));
    for (int j = 0; j < paiMesh->mBones[i]->mNumWeights; ++j) {
      int vertex_id = paiMesh->mBones[i]->mWeights[j].mVertexId;
      float weight = paiMesh->mBones[i]->mWeights[j].mWeight;
      Vertex& vertex = (*vertices)[vertex_id];
      int index = GetNextIndex(vertex);
      DCHECK_GE(index, 0);
      vertex.weights[index] = weight;
      vertex.index[index] = bone_index;
    }
  }
}

void HardwareSkinnedMesh::LoadMaterial(const ::base::FilePath& filepath,
                                       azer::RenderSystem* rs,
                                       const aiScene* scene) {
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
        Material mtrl;
        mtrl.tex = texptr;
        materials_.push_back(mtrl);
      }
    }
  }
}

void HardwareSkinnedMesh::LoadScene(const aiScene* scene) {
  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    Group group;
    LoadVertex(scene->mMeshes[i], &group);
    LoadBoneWeights(scene->mMeshes[i], &group.vertices, &group.offset);
    groups_.push_back(group);
  }
}

bool HardwareSkinnedMesh::Load(const ::base::FilePath& filepath,
                               azer::RenderSystem* rs) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      ::base::WideToUTF8(filepath.value()),
      aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);
  if (scene == NULL) {
    LOG(ERROR) << "Failed to load file: " << filepath.value();
    return false;
  }

  skeleton_.Load(scene->mRootNode, rs);
  LoadScene(scene);
  Init(rs);
  LoadMaterial(filepath, rs, scene);
  return true;
}

namespace {
const azer::VertexDesc::Desc kVertexDesc[] = {
  {"POSITION", 0, azer::kVec4},
  {"COORDTEX", 0, azer::kVec2},
  {"NORMAL", 0, azer::kVec4},
  {"WEIGHTS", 0, azer::kVec4},
  {"INDEX", 0, azer::kIntVec4},
};
}
void HardwareSkinnedMesh::Init(azer::RenderSystem* rs) {
  azer::VertexBuffer::Options vbopt;
  vbopt.usage = azer::GraphicBuffer::kDynamic;
  vbopt.cpu_access = azer::kCPUWrite;
  azer::VertexDescPtr vertex_desc_ptr(
      new azer::VertexDesc(kVertexDesc, arraysize(kVertexDesc)));
  for (uint32 i = 0; i < groups_.size(); ++i) {
    RenderGroup rgroup;
    const int vertex_num = groups_[i].vertices.size();
    azer::VertexDataPtr vdata(new azer::VertexData(vertex_desc_ptr, vertex_num)); 
    memcpy(vdata->pointer(), (uint8*)&(groups_[i].vertices[0]),
           sizeof(Vertex) * vertex_num);
    rgroup.vb.reset(rs->CreateVertexBuffer(vbopt, vdata));

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
