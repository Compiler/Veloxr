#include "RenderEntity.h"
#include "UniqueOrderedNumber.h"

using Veloxr::RenderEntity;

Veloxr::OrderedNumberFactory RenderEntity::_entitySlots{4, 1};
RenderEntity::RenderEntity(){
    _entityNumber = _entitySlots.getSlot(); 
    _name = "entity" + std::to_string(_entityNumber);
}
RenderEntity::RenderEntity(std::shared_ptr<VVDataPacket> dataPacket) {
    _entityNumber = _entitySlots.getSlot(); 
    _name = "entity" + std::to_string(_entityNumber);
    _texture.setDataPacket(dataPacket);
}

void RenderEntity::setPosition(float x, float y) {
    _position = {x, y, _position.z};
}

void RenderEntity::setPosition(glm::vec3& pos) {
    _position = pos;
}

const std::vector<Veloxr::Vertex> RenderEntity::getVertices () {
    // if(_isHidden) return {};
    auto vertices = _texture.getBaseVertices();

    for(auto& vert : vertices ) {
        vert.pos.x += _position.x;
        vert.pos.y += _position.y;
        vert.renderUID = _entityNumber;
    }

    return vertices;
}

void RenderEntity::setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer) {
    _textureBuffer = std::shared_ptr<Veloxr::VeloxrBuffer>(std::move(buffer));
}

void RenderEntity::setTextureBuffer(VeloxrBuffer& buffer) {
    _textureBuffer = std::make_shared<Veloxr::VeloxrBuffer>(std::move(buffer));
}

void RenderEntity::setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer) {
    _textureBuffer = buffer;
}
void RenderEntity::setName(const std::string& name) {
    _name = name;
}

void RenderEntity::destroy() {
    _entitySlots.removeSlot(_entityNumber);
    _texture.destroy();

}
