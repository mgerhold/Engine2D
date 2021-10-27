textureGUID = "3fa5a7a0-992f-4a5a-a1f1-af5db950f94e"

ballSize = Vec2.new()
ballSize.x = 20
ballSize.y = ballSize.x

courtSize = Vec2.new()
courtSize.x = 800
courtSize.y = 600

maxBallX = (courtSize.x - ballSize.x) / 2
minBallX = (-courtSize.x + ballSize.x) / 2
maxBallY = (courtSize.y - ballSize.y) / 2
minBallY = (-courtSize.y + ballSize.y) / 2

initialBallVelocity = Vec2.new()
initialBallVelocity.x = 200
initialBallVelocity.y = 200

ballVelocity = Vec2.new()
ballBounceVelocityFactor = 1.1

paddlePadding = 30
paddleSize = Vec2.new()
paddleSize.x = 20
paddleSize.y = 100
leftPaddleX = (-courtSize.x + paddleSize.x) / 2 + paddlePadding
rightPaddleX = (courtSize.x - paddleSize.x) / 2 - paddlePadding
paddleSpeed = 400
minPaddleY = (-courtSize.y + paddleSize.y) / 2
maxPaddleY = (courtSize.y - paddleSize.y) / 2

function resetBall()
    ballTransform.position.x = 0
    ballTransform.position.y = 0
    ballVelocity.x = initialBallVelocity.x
    ballVelocity.y = initialBallVelocity.y
end

function onAttach(entity)
    time = c2k.getTime()
    input = c2k.getInput()

    -- spawn ball
    ball = Entity.new()
    ball:attachRoot()
    local ballSprite = ball:attachDynamicSprite()
    ballSprite.texture = c2k.assets.texture(textureGUID)
    ballTransform = ball:attachTransform()
    ballTransform.scale.x = ballSize.x / 2
    ballTransform.scale.y = ballSize.y / 2
    resetBall()

    -- spawn paddles
    paddles = {}
    paddleTransforms = {}
    for i = 1, 2 do
        paddles[i] = Entity.new()
        paddles[i]:attachRoot()
        local paddleSprite = paddles[i]:attachDynamicSprite()
        paddleSprite.texture = c2k.assets.texture(textureGUID)
        paddleTransforms[i] = paddles[i]:attachTransform()
        paddleTransforms[i].scale.x = paddleSize.x / 2
        paddleTransforms[i].scale.y = paddleSize.y / 2
    end
    paddleTransforms[1].position.x = leftPaddleX
    paddleTransforms[2].position.x = rightPaddleX
end

function bounceBall()
    ballVelocity.x = -ballVelocity.x * ballBounceVelocityFactor
end

function hasPaddleRightHeight(paddleNumber)
    return (ballTransform.position.y - ballSize.y / 2 <= paddleTransforms[paddleNumber].position.y + paddleSize.y / 2 and
            ballTransform.position.y + ballSize.y / 2 >= paddleTransforms[paddleNumber].position.y - paddleSize.y / 2)
end

function handlePlayerInput()
    if input:keyPressed(Key.Escape) then
        c2k.application.quit()
    end
    if input:keyDown(Key.W) then
        paddleTransforms[1].position.y = paddleTransforms[1].position.y + paddleSpeed * time.delta
    end
    if input:keyDown(Key.S) then
        paddleTransforms[1].position.y = paddleTransforms[1].position.y - paddleSpeed * time.delta
    end
    if input:keyDown(Key.Up) then
        paddleTransforms[2].position.y = paddleTransforms[2].position.y + paddleSpeed * time.delta
    end
    if input:keyDown(Key.Down) then
        paddleTransforms[2].position.y = paddleTransforms[2].position.y - paddleSpeed * time.delta
    end
    for i = 1, 2 do
        if paddleTransforms[i].position.y > maxPaddleY then
            paddleTransforms[i].position.y = maxPaddleY
        elseif paddleTransforms[i].position.y < minPaddleY then
            paddleTransforms[i].position.y = minPaddleY
        end
    end
end

function moveBall()
    ballTransform.position.x = ballTransform.position.x + ballVelocity.x * time.delta
    ballTransform.position.y = ballTransform.position.y + ballVelocity.y * time.delta
end

function update(entity)
    moveBall()

    if ballTransform.position.y >= maxBallY then
        ballTransform.position.y = maxBallY
        ballVelocity.y = -ballVelocity.y
    elseif ballTransform.position.y <= minBallY then
        ballTransform.position.y = minBallY
        ballVelocity.y = -ballVelocity.y
    end

    if ballTransform.position.x + ballSize.x / 2 >= rightPaddleX - paddleSize.x / 2 then
        if hasPaddleRightHeight(2) then
            ballTransform.position.x = rightPaddleX - (paddleSize.x + ballSize.x) / 2
            bounceBall()
        else
            resetBall()
        end
    elseif ballTransform.position.x - ballSize.x / 2 <= leftPaddleX + paddleSize.x / 2 then
        if hasPaddleRightHeight(1) then
            ballTransform.position.x = leftPaddleX + (paddleSize.x + ballSize.x) / 2
            bounceBall()
        else
            resetBall()
        end
    end

    handlePlayerInput()
end

