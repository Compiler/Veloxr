#pragma once

#include "Common.h"
#include "VVShaderStageData.h"
#include "RenderEntity.h"
#include <map>
#include <memory>
#include <unordered_map>
#include "VLogger.h"
namespace Veloxr {

    /**
     * EntityManager will hold Entities and the data for a shader stage in the pipeline.
     * Why do we hold shader data in the entitymanager?!?!?!
     * An entity is not just a render entity but any component inside the application that happens in 
     * world space.
     *
     * Therefor, The entitymanager functions as the entity component system. 
     * Since we are also the entity component system, we have the most knowledge about what would be 
     * the best way to construct the pipeline for fastest output. 
     *
     *
     * Each entity will contain VVTexture. This is the buffer and memory required for rendering a texture.
     * The entity manager will then initialize the _shaderData to create:
     *      - Uniform Buffers
     *      - Vertex Buffers
     *      - Descriptor sets, layouts, and pools
     *
     */

    class EntityManager {
    
        public:
            EntityManager() = delete;
            EntityManager(std::shared_ptr<Veloxr::VVDataPacket> dataPacket);
            ~EntityManager();


            // Single entity endpoints
            std::shared_ptr<Veloxr::RenderEntity> createEntity(const std::string& name) noexcept;
            void registerEntity(std::shared_ptr<Veloxr::RenderEntity> entity) noexcept;
            void destroyEntity(const std::string& name) noexcept;
            std::shared_ptr<Veloxr::RenderEntity> getEntity(const std::string& name);

            // hard code, quick endpoint -- TODO: Keep stridable memory as well
            std::vector<std::shared_ptr<Veloxr::RenderEntity>> getEntityHandles() {
                std::vector<std::shared_ptr<Veloxr::RenderEntity>> entities;
                for(auto& [_, entity] : _entityMap) {
                    entities.push_back(entity);
                }
                return entities;
            }

            std::shared_ptr<Veloxr::VVShaderStageData> getShaderStageData() { return _shaderData; }

            // ECS Systems
            [[nodiscard]] inline const std::vector<Veloxr::Vertex>& getVertices () const { return _vertices; }
            void initialize();
            void updateUniformBuffers(uint32_t currentImage, const Veloxr::UniformBufferObject& ubo);


            void destroy();

        private:
            static constexpr auto DEFAULT_PIXEL_ENTITY_NAME = "veloxr_single_pixel_default_entity";
            Veloxr::LLogger console {"[Veloxr][EntityManager] "};

            std::shared_ptr<VVDataPacket> _data;

            // TODO: EntityComponentSystem
            // We can afford disjoint memory because the big hitters are a pointer right now. If we inline
            // the data and possess the full memory, we can stride correctly. But we can't assume the client
            // is willing to give us the data permanently
            std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>> _entityMap;
            std::vector<Veloxr::Vertex> _vertices;

            std::shared_ptr<Veloxr::VVShaderStageData> _shaderData;


            // Vk 
            void createVertexBuffer();

    };

}
