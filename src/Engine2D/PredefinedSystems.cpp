//
// Created by coder2k on 30.08.2021.
//

#include "Registry.hpp"

namespace c2k {

    namespace DynamicSpriteRenderer {

        namespace RootEntities {

            void init(const ApplicationContext& appContext, const Transform& cameraTransform) noexcept {
                appContext.renderer.beginFrame(Camera::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const RootComponent&,
                         const DynamicSprite& sprite,
                         const Transform& transform) {
                appContext.renderer.drawQuad(transform.position, transform.rotation, transform.scale, *sprite.shader,
                                             *sprite.texture, sprite.textureRect, sprite.color);
            }

            void finalize(const ApplicationContext& appContext) {
                appContext.renderer.endFrame();
            }

        }// namespace RootEntities

        namespace RelationshipEntities {
            void init(const ApplicationContext& appContext, const Transform& cameraTransform) noexcept {
                appContext.renderer.beginFrame(Camera::viewMatrix(cameraTransform));
            }

            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const Relationship& relationship,
                         const DynamicSprite& sprite,
                         const Transform& transform) {
                auto transformMatrix = transform.matrix();
                Entity current = relationship.parent;
                transformMatrix = appContext.registry.component<Transform>(current).value().matrix() * transformMatrix;
                while (appContext.registry.hasComponent<Relationship>(current)) {
                    current = appContext.registry.component<Relationship>(current)->parent;
                    transformMatrix =
                            appContext.registry.component<Transform>(current).value().matrix() * transformMatrix;
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