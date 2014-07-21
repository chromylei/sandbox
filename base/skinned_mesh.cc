#include "sbox/base/skinned_mesh.h"

#include "sbox/base/assimp.h"

void LoadBoneWeights(const aiMesh* paiMesh, BoneWeights* vec) {
  const aiVector3D zero3d(0.0f, 0.0f, 0.0f);
  for (uint32 i = 0; i < paiMesh->mNumVertices; ++i) {
  }
}

bool SoftSkinnedMesh::Load(const ::base::FilePath& filepath,
                           azer::RenderSystem* rs) {
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
    mutable_groups()->push_back(group);

    BoneWeights weights;
    LoadBoneWeights(scene->mMeshes[i], &weights);
    group_weights_.push_back(weights);
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
        mutable_materials()->push_back(mtrl);
      }
    }
  }

  Mesh::Init(rs);

  skeleton_.Load(scene->mRootNode, rs);
  return true;
}

