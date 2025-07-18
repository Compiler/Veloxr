#include "RenderEntity.h"

using Veloxr::RenderEntity;

int RenderEntity::ENTITY_COUNT = 0;

RenderEntity::RenderEntity(){_name = "entity" + std::to_string(ENTITY_COUNT++);}
RenderEntity::RenderEntity(std::shared_ptr<VVDataPacket> dataPacket) {
    _texture.setDataPacket(dataPacket);
}

void RenderEntity::setPosition(float x, float y) {
    _position = {x, y, _position.z};
}

void RenderEntity::setPosition(glm::vec3& pos) {
    _position = pos;
}

const std::vector<Veloxr::Vertex> RenderEntity::getVertices () {
    auto vertices = _texture.getVertices();

    for(auto& vert : vertices ) {
        vert.pos.x += _position.x;
        vert.pos.y += _position.y;
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

}
