#pragma once

#include "RenderEntity.h"
#include <memory>
#include <unordered_map>
#include "VLogger.h"
namespace Veloxr {

    class EntityManager {
    
        public:
            EntityManager() = default;


            std::shared_ptr<Veloxr::RenderEntity> createEntity(const std::string& name);
            void registerEntity(std::shared_ptr<Veloxr::RenderEntity> entity);
            void destroyEntity(const std::string& name);
            std::shared_ptr<Veloxr::RenderEntity> getEntity(const std::string& name);

        private:
            Veloxr::LLogger console {"[Veloxr] [EntityManager] "};

            // TODO: EntityComponentSystem
            std::unordered_map<std::string, std::shared_ptr<Veloxr::RenderEntity>> _entityMap;

    };

}
