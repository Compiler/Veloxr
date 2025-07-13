#include "RenderEntity.h"

using Veloxr::RenderEntity;

int RenderEntity::ENTITY_COUNT = 0;

RenderEntity::RenderEntity(){_name = "entity" + std::to_string(ENTITY_COUNT++);}
RenderEntity::RenderEntity(glm::vec3 position): _position(position) {_name = "entity" + std::to_string(ENTITY_COUNT++);}
RenderEntity::RenderEntity(glm::vec2 position): _position(position.x, position.y, 0) {_name = "entity" + std::to_string(ENTITY_COUNT++);}
RenderEntity::RenderEntity(float x, float y): _position(x, y, 0) {_name = "entity" + std::to_string(ENTITY_COUNT++);}

void RenderEntity::setTextureFilepath(std::string& filepath) {
    _filepath = filepath;
}

void RenderEntity::setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer) {
    _textureBuffer = std::shared_ptr<Veloxr::VeloxrBuffer>(std::move(buffer));
}

void RenderEntity::setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer) {
    _textureBuffer = buffer;
}
void RenderEntity::setName(const std::string& name) {
    _name = name;
}

void RenderEntity::destroy() {

}
