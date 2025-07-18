#include "EntityManager.h"
#include "RenderEntity.h"
#include "VVShaderStageData.h"
#include <memory>


using Veloxr::EntityManager;

EntityManager::EntityManager(std::shared_ptr<Veloxr::VVDataPacket> dataPacket): _data(dataPacket) {
    _shaderData = std::make_shared<Veloxr::VVShaderStageData>(_data);

}

void EntityManager::initialize() {
    console.logc2(__func__);
    console.logc2("Num of entities: ", _entityMap.size() );
    _vertices.clear();
    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (auto& [name, entity] : _entityMap) {
        entity->getVVTexture().tileTexture(entity->getBuffer());

        const auto& verts = entity->getVertices();
        _vertices.insert(_vertices.begin(), verts.begin(), verts.end());

        for (const auto& vertex : verts) {
            minX = std::min(minX, vertex.pos.x);
            maxX = std::max(maxX, vertex.pos.x);
            minY = std::min(minY, vertex.pos.y);
            maxY = std::max(maxY, vertex.pos.y);
        }
    }
    console.debug("Vertices initialized, count: ", _vertices.size(), ". BoundingBox: ", minX, ", ", minY, " : ", maxX, ", ", maxY);
    _shaderData->setTextureMap(_entityMap);
    _shaderData->createStageData();
}

void EntityManager::updateUniformBuffers(uint32_t currentImage, const Veloxr::UniformBufferObject& ubo) {
    _shaderData->updateUniformBuffers(currentImage, ubo);
}

void EntityManager::registerEntity(std::shared_ptr<Veloxr::RenderEntity> entity) noexcept {
    auto name = entity->getName();
    console.log(__func__, " for ", name);
    auto findIt = _entityMap.find(name);

    if (findIt != _entityMap.end()) {
        console.warn("Entity with name '", name, "' already exists. Aborting.");
        return;
    }

    _entityMap[name] = entity;
    return ;
}
void EntityManager::destroyEntity(const std::string& name) noexcept {
    console.log(__func__, " for ", name);

    auto findIt = _entityMap.find(name);

    if (findIt != _entityMap.end()) {
        console.warn("Entity with name '", name, "' does not exist. Aborting.");
        return;
    }

    findIt->second->destroy();
    _entityMap.erase(findIt);
}

std::shared_ptr<Veloxr::RenderEntity> EntityManager::createEntity(const std::string& name) noexcept {
    console.log(__func__, " for ", name);
    auto findIt = _entityMap.find(name);

    if (findIt != _entityMap.end()) {
        console.warn("Entity with name '", name, "' already exists. Aborting.");
        return findIt->second;
    }

    std::shared_ptr<Veloxr::RenderEntity> entity = std::make_shared<Veloxr::RenderEntity>(_data);
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

