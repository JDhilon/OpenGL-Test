//
//  entity.cpp
//  OpenGL-Test
//
//  Created by Jashanjot Dhilon on 2020-01-08.
//  Copyright © 2020 Jashanjot Dhilon. All rights reserved.
//

#include "entity.hpp"


Entity::Entity() { 
    root = nullptr;
    translate = glm::vec3(0.0f, 0.0f, 0.0f);
    quaternion = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

Entity::Entity(Entity* aRoot) {
    root = aRoot;
    translate = glm::vec3(0.0f, 0.0f, 0.0f);
    quaternion = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    scale = glm::vec3(1.0f, 1.0f, 1.0f);
}

void Entity::AddComponent(Entity* componentToAdd) {
    components.push_back(componentToAdd);
}

glm::mat4 Entity::GetModelMatrix() { 
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translate);
    model *= glm::mat4_cast(quaternion);
    model = glm::scale(model, scale);
    return model;
}

void Entity::SetTranslate(glm::vec3 aTranslation) { 
    translate = aTranslation;
}

void Entity::SetQuat(glm::quat aQuat) {
    quaternion = aQuat;
}

void Entity::SetScale(glm::vec3 aScale) { 
    scale = aScale;
}

Entity::~Entity() {
}

