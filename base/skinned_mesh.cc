#include "sbox/base/skinned_mesh.h"

#include "sbox/base/assimp.h"

void SoftSkinnedMesh::LoadBoneWeights(const aiMesh* paiMesh,
                                      BoneWeightsVec* vec,
                                      MeshOffsetMat* offset) {
  for (uint32 i = 0; i < paiMesh->mNumBones; ++i) {
    std::string bone_name = paiMesh->mBones[i]->mName.data;
    int bone_index = skeleton_.GetBoneIndex(bone_name);
    Bone* bone = skeleton_.GetBone(bone_index);
    aiMatrix4x4 mat = paiMesh->mBones[i]->mOffsetMatrix;
    (*offset)[bone_index] = 
        new  azer::Matrix4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                           mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                           mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                           mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
    for (int j = 0; j < paiMesh->mBones[i]->mNumWeights; ++j) {
      int vertex_id = paiMesh->mBones[i]->mWeights[j].mVertexId;
      float weight = paiMesh->mBones[i]->mWeights[j].mWeight;
      (*vec)[vertex_id].push_back(BoneAttached(bone_index, weight));
    }
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

  skeleton_.Load(scene->mRootNode, rs);
  for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
    Mesh::Group group;
    LoadVertex(scene->mMeshes[i], &group);
    mutable_groups()->push_back(group);

    MeshOffsetMat offset;
    BoneWeightsVec weights;
    weights.resize(group.vertices.size());
    offset.resize(skeleton_.GetBoneNum());
    LoadBoneWeights(scene->mMeshes[i], &weights, &offset);
    group_weights_.push_back(weights);
    group_offset_.push_back(offset);
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
  return true;
}

azer::Matrix4 SoftSkinnedMesh::CalcPosition(const azer::Matrix4& world,
                                            const BoneWeights& weights,
                                            const MeshOffsetMat& offsets) {
  azer::Matrix4 matrix = azer::Matrix4::kZero;
  for (auto iter = weights.begin(); iter != weights.end(); ++iter) {
    Bone* bone = skeleton_.GetBone(iter->index);
    const azer::Matrix4* offset = offsets[iter->index];
    if (offset == NULL) {
      matrix += bone->combined() * iter->weight;
    } else {
      matrix += std::move(bone->combined() * *offset) * iter->weight;
    }
  }
  azer::Matrix4 ret = std::move(world * matrix);
  return ret;
}

void SoftSkinnedMesh::UpdateVertex(const azer::Matrix4& world) {
  for (size_t i = 0; i < rgroups_.size(); ++i) {
    azer::LockDataPtr dataptr(rgroups_[i].vb->map(azer::kWriteDiscard));
    Vertex* vertex = (Vertex*)dataptr->data_ptr();
    Group& group = groups_[i];
    BoneWeightsVec weights = group_weights_[i];
    for (int j = 0; j < group.vertices.size(); ++j) {
      azer::Matrix4 new_world = std::move(CalcPosition(world, weights[j],
                                                       group_offset_[i]));
      vertex[j].position =  std::move(new_world* group.vertices[j].position);
      vertex[j].coordtex = group.vertices[j].coordtex;
      vertex[j].normal = new_world * group.vertices[j].normal;
    }
    rgroups_[i].vb->unmap();
  }
}

