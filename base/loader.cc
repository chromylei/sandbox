#include "sbox/base/loader.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "azer/render/render.h"

namespace {
int GetNextIndex(const HardwareSkinnedMesh::Vertex& vertex) {
  for (int i = 0; i < 4; ++i) {
    if (vertex.index[i] == -1) {
      return i;
    }
  }

  return -1;
}
}  // namespace

bool ModelLoader::Load(const std::string& path) {
Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(
      ::base::WideToUTF8(filepath.value()),
      aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs);

  if (scene == NULL) {
    LOG(ERROR) << "Failed to load file: " << filepath.value();
    return false;
  }

  const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[i];
    delegate_->OnMesh(i, mesh->mNumVertices, mesh->mNumFaces);
    for (int j = 0; j < mesh->mNumVertices; ++j) {
      const aiVector3D& pos = mesh->mVertices[i]; 
      const aiVector3D& normal = mesh->mNormals[i];
      const aiVector3D& texcoord =
          mesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][i]) : zero3d;
      BaseVertex vertex(azer::Vector4(pos.x, pos.y, pos.z, 1.0),
                        azer::Vector2(texcoord.x, texcoord.y),
                        azer::Vector4(normal.x, normal.y, normal.z, 0.0));
      delegate_->OnVertex(i, j, vertex, bone);
    }

    for (int j = 0; j < mesh->mNumFaces; ++j) {
      const aiFace& face = mesh->mFaces[j];
      delegate_->OnIndices(i, j, face.mIndices[0], face.mIndices[1],
                           face.mIndices[2]);
    }

    for (uint32 j = 0; j < mesh->mNumBones; ++j) {
      const aiMatrix4x4& mat = mesh->mBones[j]->mOffsetMatrix;
      azer::Matrix4 offset = azer::Matrix4(
          mat[0][0], mat[0][1], mat[0][2], mat[0][3],
          mat[1][0], mat[1][1], mat[1][2], mat[1][3],
          mat[2][0], mat[2][1], mat[2][2], mat[2][3],
          mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
      for (int k = 0; k < mesh->mBones[i]->mNumWeights; ++k) {
        int vertex_id = mesh->mBones[j]->mWeights[k].mVertexId;
        float weight = mesh->mBones[j]->mWeights[k].mWeight;
        Vertex& vertex = (*vertices)[vertex_id];
        int index = GetNextIndex(vertex);
        DCHECK_GE(index, 0);
        vertex.weights[index] = weight;
        vertex.index[index] = bone_index;
      }
    }
  }
}
