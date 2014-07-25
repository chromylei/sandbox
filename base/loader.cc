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
          paiMesh->HasTextureCoords(0) ? (mesh->mTextureCoords[0][i]) : zero3d;
      BaseVertex vertex(azer::Vector4(pos.x, pos.y, pos.z, 1.0),
                        azer::Vector2(texcoord.x, texcoord.y),
                        azer::Vector4(normal.x, normal.y, normal.z, 0.0));
      delegate_->OnVertex(i, j, vertex, bone);
    }

    for (int j = 0; j < mesh->mNumFaces; ++j) {
      const aiFace& face = paiMesh->mFaces[j];
      delegate_->OnIndices(i, j, face.mIndices[0], face.mIndices[1],
                           face.mIndices[2]);
    }
  }
}
