#pragma once

#include "Common.h"
#include "RenderEntity.h"
#include <memory>
#include <unordered_map>
#include "VLogger.h"
namespace Veloxr {

    class EntityManager {
    
        public:
            EntityManager(std::shared_ptr<Veloxr::VVDataPacket> dataPacket);


            // Single entity endpoints
            std::shared_ptr<Veloxr::RenderEntity> createEntity(const std::string& name);
            void registerEntity(std::shared_ptr<Veloxr::RenderEntity> entity);
            void destroyEntity(const std::string& name);
            std::shared_ptr<Veloxr::RenderEntity> getEntity(const std::string& name);

            // ECS Systems
            [[nodiscard]] inline const std::vector<Veloxr::Vertex>& getVertices () const { return _vertices; }
            void initialize();

        private:
            Veloxr::LLogger console {"[Veloxr][EntityManager] "};

            std::shared_ptr<VVDataPacket> _data;

            // TODO: EntityComponentSystem
            // We can afford disjoint memory because the big hitters are a pointer right now. If we inline
            // the data and possess the full memory, we can stride correctly. But we can't assume the client
            // is willing to give us the data permanently
            std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>> _entityMap;
            std::vector<Veloxr::Vertex> _vertices;

    };

}
