#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Entity {
public:
    Entity();
    Entity(Entity* aRoot);
    void AddComponent(Entity* componentToAdd);
    glm::mat4 GetModelMatrix();
    void SetTranslate(glm::vec3 aTranslation);
    void SetQuat(glm::quat aQuat);
    void SetScale(glm::vec3 aScale);
    ~Entity();
    
protected:
    Entity* root;
    std::vector<Entity*> components;
    
    glm::vec3 translate;
    glm::quat quaternion;
    glm::vec3 scale;
    
private:
    
};

