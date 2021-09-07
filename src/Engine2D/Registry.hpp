//
// Created by coder2k on 26.07.2021.
//

#pragma once

#include "ComponentHolder.hpp"
#include "optional/optional.hpp"
#include "Entity.hpp"
#include "TypeIdentifier.hpp"
#include "PredefinedSystems.hpp"
#include "Component.hpp"
#include "System.hpp"

namespace c2k {

    class Registry;

    class SystemHolder final {// this class is inside this file because of circular dependency with class Registry
    public:
        explicit SystemHolder(Registry& registry);

        ~SystemHolder();

        void run() noexcept;

        template<typename... Components, typename SetupFunction, typename ForEachFunction, typename FinalizeFunction>
        void emplace(SetupFunction&& setup, ForEachFunction&& forEach, FinalizeFunction&& finalize) noexcept;

    private:
        struct SystemContext {
            void* address{ nullptr };
            void (*setupFunction)(void*){ nullptr };
            void (*forEachFunction)(void*, SystemHolder*){ nullptr };
            void (*finalizeFunction)(void*){ nullptr };
        };

    private:
        std::vector<SystemContext> mSystemContexts;
        std::vector<void (*)(void*)> mDestructors;
        Registry& mRegistry;
    };

    class Registry final {
    public:
        explicit Registry(std::size_t initialEntityCapacity = 0)
            : mComponentHolder{ initialEntityCapacity },
              mSystemHolder{ *this } {
            mEntities.reserve(initialEntityCapacity);
        }

        template<typename Component>
        void attachComponent(Entity entity, const Component& component) noexcept {
            mComponentHolder.template attach<Component>(getIdentifierBitsFromEntity(entity), component);
            if constexpr (std::is_same_v<Component, ScriptComponent>) {
                component.script->invoke("awake", entity);
            }
        }

        template<typename... Components>
        void attachComponents([[maybe_unused]] Entity entity, const Components&... components) noexcept {
            // entity is marked as maybe_unused because GCC reports a false-positive on this one
            (attachComponent(entity, components), ...);
        }

        template<typename Component>
        [[nodiscard]] bool hasComponent(Entity entity) const noexcept {
            return mComponentHolder.template has<Component>(getIdentifierBitsFromEntity(entity));
        }

        template<typename Component>
        [[nodiscard]] tl::optional<const Component&> component(Entity entity) const noexcept {
            if (!hasComponent<Component>(entity)) {
                return {};
            }
            return mComponentHolder.template get<Component>(getIdentifierBitsFromEntity(entity));
        }

        template<typename Component>
        [[nodiscard]] tl::optional<Component&> componentMutable(Entity entity) noexcept {
            if (!hasComponent<Component>(entity)) {
                return {};
            }
            return mComponentHolder.template getMutable<Component>(getIdentifierBitsFromEntity(entity));
        }

        template<typename... Components>
        [[nodiscard]] auto componentsMutable() noexcept {
            using ranges::views::transform;
            return mComponentHolder.template getMutable<Components...>() | transform([this](auto&& tuple) {
                       return std::apply(
                               [this](auto&& identifier, auto&&... rest) {
                                   return std::forward_as_tuple(mEntities[identifier], rest...);
                               },
                               tuple);
                   });
        }

        template<typename... Components>
        [[nodiscard]] auto components() const noexcept {
            using ranges::views::transform;
            return mComponentHolder.template get<Components...>() | transform([this](auto&& tuple) {
                       return std::apply(
                               [this](auto&& identifier, auto&&... rest) {
                                   return std::forward_as_tuple(mEntities[identifier], rest...);
                               },
                               tuple);
                   });
        }

        template<typename Iterator>
        [[nodiscard]] auto componentsTypeErasedMutable(Iterator typeIdentifiersBegin, Iterator typeIdentifiersEnd) {
            auto&& [begin, end] = mComponentHolder.getTypeErasedMutable(typeIdentifiersBegin, typeIdentifiersEnd);
            const auto transformerFunc = [this](std::size_t index) { return mEntities[index]; };
            begin.setIndexTransformerFunc(transformerFunc);
            end.setIndexTransformerFunc(transformerFunc);
            return std::pair(begin, end);
        }

        template<typename Iterator>
        [[nodiscard]] auto componentsTypeErased(Iterator typeIdentifiersBegin, Iterator typeIdentifiersEnd) {
            auto&& [begin, end] = mComponentHolder.getTypeErased(typeIdentifiersBegin, typeIdentifiersEnd);
            const auto transformerFunc = [this](std::size_t index) { return mEntities[index]; };
            begin.setIndexTransformerFunc(transformerFunc);
            end.setIndexTransformerFunc(transformerFunc);
            return std::pair(begin, end);
        }

        template<typename... Components>
        Entity createEntity(Components... components) noexcept {
            const auto entity = [&]() {
                if (mNumRecyclableEntities > 0) {
                    const auto index = getIndexFromEntity(mNextRecyclableEntity);
                    swapIdentifiers(mEntities[index], mNextRecyclableEntity);
                    --mNumRecyclableEntities;
                    assert(static_cast<std::size_t>(getIdentifierBitsFromEntity(mEntities[index])) == index);
                    return mEntities[index];
                } else {
                    mEntities.push_back(entityFromIdentifierAndGeneration(
                            Identifier{ gsl::narrow_cast<Entity>(mEntities.size()) }, Generation{ 0 }));
                    // TODO: add option to deny resizing
                    if (mComponentHolder.size() < mEntities.size()) {
                        mComponentHolder.resize(mEntities.size());
                    }
                    return mEntities.back();
                }
            }();// immediately invoked
            attachComponents(entity, components...);
            return entity;
        }

        void destroyEntity(Entity entity) noexcept {
            assert(isEntityAlive(entity) && "The entity to remove must be alive.");
            const auto index = getIndexFromEntity(entity);
            increaseGeneration(mEntities[index]);
            swapIdentifiers(mNextRecyclableEntity, mEntities[index]);
            ++mNumRecyclableEntities;
        }
        [[nodiscard]] bool isEntityAlive(Entity entity) const noexcept {
            const auto index = getIndexFromEntity(entity);
            assert(mEntities.size() > index);
            return mEntities[index] == entity;
        }
        [[nodiscard]] std::size_t numEntities() const noexcept {
            return mEntities.size();
        }
        [[nodiscard]] std::size_t numEntitiesAlive() const noexcept {
            return numEntities() - numEntitiesDead();
        }
        [[nodiscard]] std::size_t numEntitiesDead() const noexcept {
            return mNumRecyclableEntities;
        }
        template<typename Type>
        [[nodiscard]] std::size_t typeIdentifier() const noexcept {
            return mComponentHolder.template typeIdentifier<Type>();
        }
        template<typename... Components, typename SetupFunction>
        void emplaceSystem(SetupFunction&& setup) noexcept {
            emplaceSystem<Components...>(
                    std::forward<SetupFunction>(setup), []() {}, []() {});
        }
        template<typename... Components, typename SetupFunction, typename ForEachFunction>
        void emplaceSystem(SetupFunction&& setup, ForEachFunction&& forEach) noexcept {
            emplaceSystem<Components...>(std::forward<SetupFunction>(setup), std::forward<ForEachFunction>(forEach),
                                         []() {});
        }
        template<typename... Components, typename SetupFunction, typename ForEachFunction, typename FinalizeFunction>
        void emplaceSystem(SetupFunction&& setup, ForEachFunction&& forEach, FinalizeFunction&& finalize) noexcept {
            mSystemHolder.emplace<Components...>(std::forward<SetupFunction>(setup),
                                                 std::forward<ForEachFunction>(forEach),
                                                 std::forward<FinalizeFunction>(finalize));
        }
        void addDynamicSpriteRenderer(
                const ApplicationContext& appContext,
                const TransformComponent& cameraTransform = TransformComponent::identity()) noexcept {
            emplaceSystem<const RootComponent&, const DynamicSpriteComponent&, const TransformComponent&>(
                    [&appContext, &cameraTransform]() {
                        DynamicSpriteRenderer::RootEntities::init(appContext, cameraTransform);
                    },
                    [&appContext](Entity entity, const RootComponent& root, const auto& sprite,
                                  const TransformComponent& transform) {
                        DynamicSpriteRenderer::RootEntities::forEach(appContext, entity, root, sprite, transform);
                    },
                    [&appContext]() { DynamicSpriteRenderer::RootEntities::finalize(appContext); });
            emplaceSystem<const RelationshipComponent&, const DynamicSpriteComponent&, const TransformComponent&>(
                    [&appContext, &cameraTransform]() {
                        DynamicSpriteRenderer::RelationshipEntities::init(appContext, cameraTransform);
                    },
                    [&appContext](Entity entity, const RelationshipComponent& relationship, const auto& sprite,
                                  const TransformComponent& transform) {
                        DynamicSpriteRenderer::RelationshipEntities::forEach(appContext, entity, relationship, sprite,
                                                                             transform);
                    },
                    [&appContext]() { DynamicSpriteRenderer::RelationshipEntities::finalize(appContext); });
        }
        void addScreenClearer(const ApplicationContext& appContext, bool colorBuffer, bool depthBuffer) {
            emplaceSystem<>([&appContext, colorBuffer, depthBuffer]() {
                ScreenClearer::init(appContext, colorBuffer, depthBuffer);
            });
        }
        void runSystems() noexcept {
            mSystemHolder.run();
        }

    private:
        using Generation = Entity;
        using Identifier = Entity;

    private:
        [[nodiscard]] static Identifier getIdentifierBitsFromEntity(Entity entity) noexcept {
            return Identifier{ (entity & identifierMask) >> generationBits };
        }
        [[nodiscard]] static Generation getGenerationBitsFromEntity(Entity entity) noexcept {
            return Generation{ entity & generationMask };
        }
        [[nodiscard]] static Entity entityFromIdentifierAndGeneration(Identifier identifier,
                                                                      Generation generation) noexcept {
            return Entity{ (identifier << generationBits) | generation };
        }
        static void swapIdentifiers(Entity& entity1, Entity& entity2) noexcept {
            const auto temp = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity1),
                                                                getGenerationBitsFromEntity(entity2));
            entity1 = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity2),
                                                        getGenerationBitsFromEntity(entity1));
            entity2 = temp;
        }
        static void increaseGeneration(Entity& entity) noexcept {
            entity = entityFromIdentifierAndGeneration(getIdentifierBitsFromEntity(entity),
                                                       getGenerationBitsFromEntity(entity) + 1);
        }
        [[nodiscard]] static std::size_t getIndexFromEntity(Entity entity) noexcept {
            return static_cast<std::size_t>(getIdentifierBitsFromEntity(entity));
        }

    private:
        static constexpr std::size_t identifierBits = sizeof(Entity) * 8 * 20 / 32;
        static constexpr std::size_t generationBits = sizeof(Entity) * 8 - identifierBits;
        static constexpr Entity generationMask = std::numeric_limits<Entity>::max() >> identifierBits;
        static constexpr Entity identifierMask = std::numeric_limits<Entity>::max() << generationBits;
        ComponentHolder<Identifier> mComponentHolder;
        SystemHolder mSystemHolder;
        std::vector<Entity> mEntities;
        std::size_t mNumRecyclableEntities{ 0 };
        Entity mNextRecyclableEntity{ invalidEntity };
    };

    template<typename... Components, typename SetupFunction, typename ForEachFunction, typename FinalizeFunction>
    void SystemHolder::emplace(SetupFunction&& setup, ForEachFunction&& forEach, FinalizeFunction&& finalize) noexcept {
        using SystemType = System<Entity, std::remove_cvref_t<decltype(setup)>, std::remove_cvref_t<decltype(forEach)>,
                                  std::remove_cvref_t<decltype(finalize)>, Components...>;
        constexpr auto numComponents = sizeof...(Components);
        const auto forEachFunction = []() {
            if constexpr (numComponents == 0) {
                return [](void* address, SystemHolder*) { static_cast<SystemType*>(address)->forEach(); };
            } else {
                return [](void* address, SystemHolder* self) {
                    static_cast<SystemType*>(address)->forEach(
                            self->mRegistry.template componentsMutable<std::remove_cvref_t<Components>...>());
                };
            }
        }();
        mSystemContexts.push_back(SystemContext{
                .address{ new SystemType{ std::forward<decltype(setup)>(setup),
                                          std::forward<decltype(forEach)>(forEach),
                                          std::forward<decltype(finalize)>(finalize) } },
                .setupFunction{ [](void* address) { static_cast<SystemType*>(address)->setup(); } },
                .forEachFunction{ forEachFunction },
                .finalizeFunction{ [](void* address) { static_cast<SystemType*>(address)->finalize(); } } });
        mDestructors.push_back([](void* address) { delete static_cast<SystemType*>(address); });
    }

}// namespace c2k