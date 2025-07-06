#include "RenderEntity.h"

namespace Veloxr {

    RenderEntity::RenderEntity(glm::vec3 position): _position(position) {}
    RenderEntity::RenderEntity(glm::vec2 position): _position(position.x, position.y, 0) {}
    RenderEntity::RenderEntity(float x, float y): _position(x, y, 0) {}

    void RenderEntity::setTextureFilepath(std::string& filepath) {
        _filepath = filepath;
    }

    void RenderEntity::setTextureBuffer(std::unique_ptr<Veloxr::VeloxrBuffer> buffer) {
        _textureBuffer = std::shared_ptr<Veloxr::VeloxrBuffer>(std::move(buffer));
    }

    void RenderEntity::setTextureBuffer(std::shared_ptr<Veloxr::VeloxrBuffer> buffer) {
        _textureBuffer = buffer;
    }
}
