#include "EntityManager.h"
#include "RenderEntity.h"
#include <memory>


using Veloxr::EntityManager;


void EntityManager::initialize() {
    for(auto& [name, entity] : _entityMap) {
        entity->getVVTexture().tileTexture(entity->getBuffer());

    }
}

void EntityManager::registerEntity(std::shared_ptr<Veloxr::RenderEntity> entity) {
    auto name = entity->getName();
    console.log(__func__, " for ", name);
    auto findIt = _entityMap.find(name);

    if (findIt == _entityMap.end()) {
        console.warn("Entity with name '", name, "' already exists. Aborting.");
        return;
    }

    _entityMap[name] = entity;
    return ;
}
void EntityManager::destroyEntity(const std::string& name) {
    console.log(__func__, " for ", name);

    auto findIt = _entityMap.find(name);

    if (findIt != _entityMap.end()) {
        console.warn("Entity with name '", name, "' does not exist. Aborting.");
        return;
    }

    findIt->second->destroy();
    _entityMap.erase(findIt);
}

std::shared_ptr<Veloxr::RenderEntity> EntityManager::createEntity(const std::string& name) {
    console.log(__func__, " for ", name);
    auto findIt = _entityMap.find(name);

    if (findIt == _entityMap.end()) {
        console.warn("Entity with name '", name, "' already exists. Aborting.");
        return findIt->second;
    }

    std::shared_ptr<Veloxr::RenderEntity> entity = std::make_shared<Veloxr::RenderEntity>();
    entity->setName(name);
    _entityMap[name] = entity;
    return entity;

}

std::shared_ptr<Veloxr::RenderEntity> EntityManager::getEntity(const std::string& name) {
    console.log(__func__, " for ", name);
    auto findIt = _entityMap.find(name);

    if (findIt == _entityMap.end()) {
        console.warn("Could not find entity '", name, "'");
        return nullptr;
    }

    return findIt->second;
}

