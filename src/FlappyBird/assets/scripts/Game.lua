require("AssetGUIDs")
require("Bird")
require("BackgroundLayer")
require("CheatMode")
require("FireworksContainer")
require("GameState")
require("NumberDisplay")
require("Pipes")
require("Utility")

Game = {}
Game.__index = Game

function Game.new(screenSize)
    local result = {
        gameState = GameState.STARTING,
        screenSize = screenSize,
        cheatMode = false,
        initialHorizontalMovementSpeed = 200,
        score = 0
    }
    setmetatable(result, Game)
    result:setup()
    return result
end

function Game:setup()
    self.horizontalMovementSpeed = self.initialHorizontalMovementSpeed
    local backgroundLayers = {
        ground = BackgroundLayer.new(0.1, assetGUIDs.groundTextureGUID, 100, 0, 1.0),
        trees = BackgroundLayer.new(-0.2, assetGUIDs.treeTextureGUID, 40, 100, 0.2),
        skyline = BackgroundLayer.new(-0.3, assetGUIDs.skylineTextureGUID, 60, 100 + 40 / 3, 0.1),
        clouds = BackgroundLayer.new(-0.4, assetGUIDs.cloudTextureGUID, 90, 100 + 40 / 2, 0.06)
    }
    self.backgroundLayerContainer = BackgroundLayerContainer.new(self.screenSize, backgroundLayers)
    self.pipeContainer = PipeContainer.new(assetGUIDs.topPipeTextureGUID, assetGUIDs.bottomPipeTextureGUID, 100, 400, 150, self:getGroundHeight(), self.screenSize)
    self.bird = Bird.new(80, 30, assetGUIDs.birdSpriteSheetGUID, self.screenSize, self:getGroundHeight())
    local scoreDisplayDigitSize = 64
    local scoreDisplayCenterPosition = Vec3.new(0, self.screenSize.y / 4 + scoreDisplayDigitSize / 2, 0.5)
    self.scoreDisplay = NumberDisplay.new(assetGUIDs.digitsSpriteSheetGUID, assetGUIDs.digitsTextureGUID, scoreDisplayCenterPosition, scoreDisplayDigitSize, 0)
    self.fireworksContainer = FireworksContainer.new(assetGUIDs.fireworksParticleSystemGUID, self.screenSize, 2 * self.pipeContainer.pipeWidth)
    self.gameStateUpdater = {
        [GameState.STARTING] = Game.updateGameStateStarting,
        [GameState.PLAYING] = Game.updateGameStatePlaying,
        [GameState.COLLIDED] = Game.updateGameStateCollided,
        [GameState.BIRD_DOWN] = Game.updateGameStateBirdDown
    }
end

function Game:getGroundHeight()
    return self.backgroundLayerContainer.layers.ground.height
end

function Game:wasJumpPressed()
    return input:mousePressed(MouseButton.Left) or input:keyPressed(Key.Space)
end

function Game:increaseScore()
    self.score = self.score + 1
    self.scoreDisplay:show(self.score)
end

function Game:spawnFireworks(passedPipeIndex)
    local topPipeTransform = self.pipeContainer.entities[passedPipeIndex + 1]:getTransform()
    local fireworksPosition = Vec3.new(
            topPipeTransform.position.x,
            topPipeTransform.position.y - self.pipeContainer.pipeHeight / 2 + 15,
            -0.2
    )
    self.fireworksContainer:spawn(fireworksPosition)
end

function Game:restart()
    self.bird:resetTransform()
    self.pipeContainer:clear()
    self.horizontalMovementSpeed = self.initialHorizontalMovementSpeed
    self.score = 0
    self.scoreDisplay:show(self.score)
    self.gameState = GameState.STARTING
end

function Game:handleGroundCollision()
    if self.gameState ~= GameState.BIRD_DOWN and self.bird:hasHitTheGround() then
        self.gameState = GameState.BIRD_DOWN
        self.horizontalMovementSpeed = 0
        self.fireworksContainer:clear()
    end
end

function Game:updateGameStateStarting()
    self.bird:roam()
    if self:wasJumpPressed() then
        self.bird:jump()
        self.gameState = GameState.PLAYING
    end
end

function Game:updateGameStatePlaying()
    if self.cheatMode then
        handleCheatMode(self)
        self.bird:updateAnimation()
    else
        self.bird:update(self:wasJumpPressed(), self.horizontalMovementSpeed)
        local collidedWithPipe = self.pipeContainer:collisionOfAnyPipeWithCircle(self.bird.transform.position, self.bird.collisionRadius)
        if collidedWithPipe then
            self.gameState = GameState.COLLIDED
        end
    end
    if time.elapsed >= self.pipeContainer.nextSpawnTime then
        self.pipeContainer:spawn(self.horizontalMovementSpeed)
    end
end

function Game:updateGameStateCollided()
    -- disable controls while falling down
    self.bird:update(false, self.horizontalMovementSpeed)
end

function Game:updateGameStateBirdDown()
    if self:wasJumpPressed() then
        self:restart()
    end
end

function Game:update()
    if input:keyPressed(Key.Escape) then
        c2k.application.quit()
    end
    if input:keyPressed(Key.C) then
        self.cheatMode = not self.cheatMode
    end

    self.backgroundLayerContainer:update(self.horizontalMovementSpeed)

    local passedPipeIndex = self.pipeContainer:update(self.horizontalMovementSpeed, self.bird)
    if passedPipeIndex ~= nil then
        self:increaseScore()
        self:spawnFireworks(passedPipeIndex)
    end

    self.fireworksContainer:update(self.horizontalMovementSpeed)

    self.gameStateUpdater[self.gameState](self)

    self:handleGroundCollision()
end
