//
// Created by coder2k on 20.09.2021.
//

#include "BufferedScriptCommand.hpp"
#include "Registry.hpp"
#include "Component.hpp"
#include "AssetDatabase.hpp"
#include "ApplicationContext.hpp"
#include <utility>

namespace c2k::BufferedScriptCommands {

    void AttachScript::process(ApplicationContext& appContext) const noexcept {
        appContext.registry.attachComponent<ScriptComponent>(
                targetEntity, ScriptComponent{ .script{ &appContext.assetDatabase.scriptMutable(scriptGUID) } });
    }

    void DestroyEntity::process(ApplicationContext& appContext) const noexcept {
        appContext.registry.destroyEntity(targetEntity);
    }

}// namespace c2k::BufferedScriptCommands