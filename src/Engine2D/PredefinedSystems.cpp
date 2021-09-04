//
// Created by coder2k on 30.08.2021.
//

#include "Registry.hpp"
#include "Component.hpp"

namespace c2k {

    namespace DynamicSpriteRenderer {

        namespace RootEntities {

            void init(const ApplicationContext& appContext,
                      const c2k::Components::Transform& cameraTransform) noexcept {
                appContext.renderer.beginFrame(c2k::Components::Camera::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const c2k::Components::RootComponent&,
                         const c2k::Components::DynamicSprite& sprite,
                         const c2k::Components::Transform& transform) {
                appContext.renderer.drawQuad(transform.position, transform.rotation, transform.scale, *sprite.shader,
                                             *sprite.texture, sprite.textureRect, sprite.color);
            }

            void finalize(const ApplicationContext& appContext) {
                appContext.renderer.endFrame();
            }

        }// namespace RootEntities

        namespace RelationshipEntities {
            void init(const ApplicationContext& appContext,
                      const c2k::Components::Transform& cameraTransform) noexcept {
                appContext.renderer.beginFrame(c2k::Components::Camera::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const c2k::Components::Relationship& relationship,
                         const c2k::Components::DynamicSprite& sprite,
                         const c2k::Components::Transform& transform) {
                auto transformMatrix = transform.matrix();
                Entity current = relationship.parent;
                transformMatrix = appContext.registry.component<c2k::Components::Transform>(current).value().matrix() *
                                  transformMatrix;
                while (appContext.registry.hasComponent<c2k::Components::Relationship>(current)) {
                    current = appContext.registry.component<c2k::Components::Relationship>(current)->parent;
                    transformMatrix =
                            appContext.registry.component<c2k::Components::Transform>(current).value().matrix() *
                            transformMatrix;
                }
                appContext.renderer.drawQuad(transformMatrix, *sprite.shader, *sprite.texture, sprite.textureRect,
                                             sprite.color);
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