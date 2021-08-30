//
// Created by coder2k on 19.08.2021.
//

#pragma once

#include "Entity.hpp"
#include "Renderer.hpp"
#include "Component.hpp"
#include "ApplicationContext.hpp"

namespace c2k {

    namespace DynamicSpriteRenderer {
        namespace RootEntities {

            void init(const ApplicationContext& appContext, const Transform& cameraTransform) noexcept;
            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const RootComponent&,
                         const DynamicSprite& sprite,
                         const Transform& transform);
            void finalize(const ApplicationContext& appContext);

        }// namespace RootEntities

        namespace RelationshipEntities {

            void init(const ApplicationContext& appContext, const Transform& cameraTransform) noexcept;
            void forEach(const ApplicationContext& appContext,
                         Entity,
                         const Relationship& relationship,
                         const DynamicSprite& sprite,
                         const Transform& transform);
            void finalize(const ApplicationContext& appContext);

        }// namespace RelationshipEntities


    }// namespace DynamicSpriteRenderer

    namespace ScreenClearer {

        void init(const ApplicationContext& appContext, bool colorBuffer, bool depthBuffer);

    }// namespace ScreenClearer
}// namespace c2k