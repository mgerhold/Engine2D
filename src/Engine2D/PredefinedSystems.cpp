//
// Created by coder2k on 30.08.2021.
//

#include "Registry.hpp"
#include "Component.hpp"

namespace c2k {

    namespace DynamicSpriteRenderer {

        namespace RootEntities {

            void init(const ApplicationContext& appContext, const TransformComponent& cameraTransform) noexcept {
                appContext.renderer.beginFrame(CameraComponent::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const RootComponent&,
                         const DynamicSpriteComponent& sprite,
                         const TransformComponent& transform) {
                appContext.renderer.drawQuad(transform.position, transform.rotation, transform.scale,
                                             *sprite.shaderProgram, *sprite.texture, sprite.textureRect, sprite.color);
            }

            void finalize(const ApplicationContext& appContext) {
                appContext.renderer.endFrame();
            }

        }// namespace RootEntities

        namespace RelationshipEntities {
            void init(const ApplicationContext& appContext, const TransformComponent& cameraTransform) noexcept {
                appContext.renderer.beginFrame(CameraComponent::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const RelationshipComponent& relationship,
                         const DynamicSpriteComponent& sprite,
                         const TransformComponent& transform) {
                auto transformMatrix = transform.matrix();
                Entity current = relationship.parent;
                transformMatrix =
                        appContext.registry.component<TransformComponent>(current).value().matrix() * transformMatrix;
                while (appContext.registry.hasComponent<RelationshipComponent>(current)) {
                    current = appContext.registry.component<RelationshipComponent>(current)->parent;
                    transformMatrix = appContext.registry.component<TransformComponent>(current).value().matrix() *
                                      transformMatrix;
                }
                appContext.renderer.drawQuad(transformMatrix, *sprite.shaderProgram, *sprite.texture,
                                             sprite.textureRect, sprite.color);
            }

            void finalize(const ApplicationContext& appContext) {
                appContext.renderer.endFrame();
            }
        }// namespace RelationshipEntities


    }// namespace DynamicSpriteRenderer

    namespace ScreenClearer {
        void init(const ApplicationContext& appContext, bool colorBuffer, bool depthBuffer) {
            appContext.renderer.clear(colorBuffer, depthBuffer);
        }
    }// namespace ScreenClearer

}// namespace c2k