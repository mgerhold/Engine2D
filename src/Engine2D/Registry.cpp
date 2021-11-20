//
// Created by coder2k on 20.11.2021.
//

#include "Registry.hpp"
#include "EntityUtils/EntityUtils.hpp"

namespace c2k {

    void Registry::destroyEntity(Entity entity) noexcept {
        assert(isEntityAlive(entity) && "The entity to remove must be alive.");
        unparentEntitiesByParent(entity);
        const auto index = getIndexFromEntity(entity);
        deleteComponentsByEntityIndex(index);
        increaseGeneration(mEntities[index]);
        swapIdentifiers(mNextRecyclableEntity, mEntities[index]);
        ++mNumRecyclableEntities;
    }

    void Registry::unparentEntitiesByParent(Entity parent) noexcept {
        std::vector<Entity> entitiesToUnparent;
        std::size_t total = 0;
        for (auto&& [entity, relationship] : componentsMutable<RelationshipComponent>()) {
            if (relationship.parent == parent) {
                entitiesToUnparent.emplace_back(entity);
            }
            ++total;
        }
        for (const auto entityToUnparent : entitiesToUnparent) {
            const auto globalTransform = EntityUtils::getGlobalTransform(*this, entityToUnparent);
            removeComponent<RelationshipComponent>(entityToUnparent);
            attachComponent(entityToUnparent, RootComponent{});
            if (hasComponent<TransformComponent>(entityToUnparent)) {
                auto& transform = componentMutable<TransformComponent>(entityToUnparent).value();
                transform = TransformComponent::fromMatrix(globalTransform);
            }
        }
    }

    bool c2k::Registry::isEntityAlive(c2k::Entity entity) const noexcept {
        const auto index = getIndexFromEntity(entity);
        if (index >= mEntities.size()) {
            return false;
        }
        return mEntities[index] == entity;
    }

    std::size_t c2k::Registry::numEntities() const noexcept {
        return mEntities.size();
    }

    std::size_t c2k::Registry::numEntitiesAlive() const noexcept {
        return numEntities() - numEntitiesDead();
    }

    std::size_t c2k::Registry::numEntitiesDead() const noexcept {
        return mNumRecyclableEntities;
    }

    c2k::Registry::Identifier c2k::Registry::getIdentifierBitsFromEntity(c2k::Entity entity) noexcept {
        return Identifier{ (entity & identifierMask) >> generationBits };
    }
    c2k::Registry::Generation c2k::Registry::getGenerationBitsFromEntity(c2k::Entity entity) noexcept {
        return Generation{ entity & generationMask };
    }

    c2k::Entity c2k::Registry::entityFromIdentifierAndGeneration(c2k::Registry::Identifier identifier,
                                                                 c2k::Registry::Generation generation) noexcept {
        return Entity{ (identifier << generationBits) | generation };
    }

    void c2k::Registry::swapIdentifiers(c2k::Entity& entity1, c2k::Entity& entity2) noexcept {
        const auto temp = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity1),
                                                            getGenerationBitsFromEntity(entity2));
        entity1 = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity2),
                                                    getGenerationBitsFromEntity(entity1));
        entity2 = temp;
    }

    void c2k::Registry::increaseGeneration(c2k::Entity& entity) noexcept {
        entity = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity),
                                                   getGenerationBitsFromEntity(entity) + 1);
    }

    std::size_t c2k::Registry::getIndexFromEntity(c2k::Entity entity) noexcept {
        return static_cast<std::size_t>(getIdentifierBitsFromEntity(entity));
    }

    void c2k::Registry::deleteComponentsByEntityIndex(std::size_t index) noexcept {
        const auto entityIndex = gsl::narrow_cast<Entity>(index);
        for (auto sparseSet : mComponentHolder.sparseSets()) {
            if (sparseSet->has(entityIndex)) {
                sparseSet->remove(entityIndex);
            }
        }
    }

}// namespace c2k