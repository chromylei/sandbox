#include "sbox/base/skinned_mesh.h"

#include "sbox/base/assimp.h"
#include "sbox/base/mesh.h"
#include "azer/render/render.h"

namespace sbox {
void SkinnedMesh::LoadVertex(const aiMesh* paiMesh, Group* group) {
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

void SkinnedMesh::LoadBoneWeights(const aiMesh* paiMesh,
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
      vertex.index.push_back(bone_index);
      vertex.weights.push_back(weight);
    }
  }
}

void SkinnedMesh::LoadMaterial(const ::base::FilePath& filepath,
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

void SkinnedMesh::LoadScene(const aiScene* scene) {
  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    Group group;
    LoadVertex(scene->mMeshes[i], &group);
    LoadBoneWeights(scene->mMeshes[i], &group.vertices, &group.offset);
    groups_.push_back(group);
  }

  LoadNode(scene->mRootNode);
}

void SkinnedMesh::LoadNode(const aiNode* node) {
  for (uint32 i = 0; i < node->mNumMeshes; ++i) {
    int index = node->mMeshes[i];
    std::string node_name(node->mName.data);
    int bone_index = skeleton_.GetBoneIndex(node_name);
    DCHECK(bone_index != -1);
    groups_[index].bone = skeleton_.GetBone(bone_index);
  }
  for (uint32 i = 0; i < node->mNumChildren; ++i) {
    LoadNode(node->mChildren[i]);
  }
}

bool SkinnedMesh::Load(const ::base::FilePath& filepath,
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
  anim_set_.Load(scene);
  LoadMaterial(filepath, rs, scene);
  return true;
}
}  // namespace base
