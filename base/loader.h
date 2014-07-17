#pragma once

#include <string>
#include "base/files/file_path.h"
#include "sbox/base/mesh.h"

bool LoadMesh(const ::base::FilePath& filepath, Mesh* mesh,
              azer::RenderSystem* rs);
