#pragma once

#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "texture.hpp"
#include "entity.hpp"

class Entity;

class Model : public Entity
{
public:
    Model();
    
    void LoadModel(const std::string& fileName);
    void RenderModel();
    void ClearModel();
    
    ~Model();

private:
    void LoadNode(aiNode *node, const aiScene *scene);
    void LoadMesh(aiMesh *mesh, const aiScene *scene);
    void LoadMaterials(const aiScene *scene);
    
    std::vector<Mesh*> meshList;
    std::vector<Texture*> textureList;
    std::vector<unsigned int> meshToTex;
};

