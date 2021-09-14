--- The c2k engine module.
-- This module contains the Lua application programming interface (API) of the c2k game engine.
-- @module c2k

c2k = {}

c2k.assets = {}

--- Retrieves a shader program handle.
-- @param guid The global unique id of the shader program.
-- @see ShaderProgram
function c2k.assets.shaderProgram(guid)
end

--- Retrieves a sprite sheet handle.
-- @param guid The global unique id of the sprite sheet.
-- @see SpriteSheet
-- @see SpriteSheetFrame
function c2k.assets.spriteSheet(guid)
end

--- Retrieves a texture handle.
-- @param guid The global unique id of the texture program.
-- @see Texture
function c2k.assets.texture(guid)
end

--- Retrieves timing information.
-- This function returns data about the timing (e. g. delta time) of the current frame.
-- @see Time
-- @treturn Time an object that contains timing information
function c2k.getTime()
end

--- Retrieves a handle to the input system.
-- @see Input
-- @treturn Input a handle to the input system
function c2k.getInput()
end

--- This class represents a color.
-- @type Color

--- The red channel of the color.
Color.r = 0

--- The green channel of the color.
Color.g = 0

--- The blue channel of the color.
Color.b = 0

--- The alpha channel of the color.
Color.a = 0

--- This class represents a dynamic sprite component.
-- A dynamic sprite component is one of the components that must be present
-- on an @{Entity} for a sprite to be rendered. The entity also needs a
-- @{Transform} component and either a @{Root} or a @{Relationship} component.
-- @type DynamicSprite

--- The texture rect to use.
-- This @{Rect} defines the region of the texture to be shown. Its values
-- must be specified in normalized coordinates.
-- @see Rect
DynamicSprite.textureRect = Rect.new()

--- The color component of the sprite.
-- Depending on the used shader, the sprite may get tinted with this color.
-- @see Color
DynamicSprite.color = Color.new()

--- The texture to be used for this sprite.
-- @see Texture
DynamicSprite.texture = Texture.new()

--- The shader program to be used for this sprite.
-- @see ShaderProgram
DynamicSprite.shaderProgram = ShaderProgram.new()

--- This class represents an entity.
-- @type Entity

--- The id of the entity.
Entity.id = 0

--- Constructs a new entity.
-- @treturn Entity handle to the new entity
function Entity.new()
end

--- Destroys the entity and all of its components.
function Entity:destroy()
end

--- Retrieves the transform component.
-- @treturn Transform the transform component
function Entity:getTransform()
end

--- Attaches a newly created transform component to the entity.
-- @treturn Transform the transform component that has been attached
function Entity:attachTransform()
end

--- Retrieves the dynamic sprite component.
-- @treturn DynamicSprite the dynamic sprite component
function Entity:getDynamicSprite()
end

--- Attaches a newly created dynamic sprite component to the entity.
-- @treturn DynamicSprite the dynamic sprite component that has been attached
function Entity:attachDynamicSprite()
end

--- Checks if the entity has a @{Root} component attached.
-- @treturn bool true if the entity has a root component, false otherwise
function Entity:hasRoot()
end

--- Attaches a newly created @{Root} component to the entity.
function Entity:attachRoot()
end

--- This class provides access to the engine's input system.
-- @type Input

--- Checks if the specified key is currently being held down.
-- @tparam Key key the key of interest
-- @treturn bool true if the key is held, false otherwise
function Input:keyDown(key)
end

--- Checks if the specified key has been repeated this frame.
-- @tparam Key key the key of interest
-- @treturn bool true if the key is repeated, false otherwise
function Input:keyRepeated(key)
end

--- Checks if the specified key has been pressed this frame.
-- @tparam Key key the key of interest
-- @treturn bool true if the key has been pressed, false otherwise
function Input:keyPressed(key)
end

--- Checks if the specified key has been released this frame.
-- @tparam Key key the key of interest
-- @treturn bool true if the key has been released, false otherwise
function Input:keyReleased(key)
end

--- Gets the current position of the mouse cursor.
-- @treturn Vec2 the cursor position
function Input:mousePosition()
end

--- Checks if the mouse cursor currently is inside the window.
-- @treturn bool true if the cursor is inside the window, false otherwise
function Input:mouseInsideWindow()
end

--- Checks if the specified mouse button is currently being held down.
-- @tparam MouseButton mouseButton the mouse button of interest
-- @treturn bool true if the mouse button is held, false otherwise
function Input:mouseDown(mouseButton)
end

--- Checks if the specified mouse button has been pressed this frame.
-- @tparam MouseButton mouseButton the mouse button of interest
-- @treturn bool true if the mouse button has been pressed, false otherwise
function Input:mousePressed(mouseButton)
end

--- Checks if the specified mouse button has been released this frame.
-- @tparam MouseButton mouseButton the mouse button of interest
-- @treturn bool true if the mouse button has been released, false otherwise
function Input:mouseReleased(mouseButton)
end

--- This class represents a rectangle.
-- @type Rect

--- The x-value of the left boundary of the rectangle.
Rect.left = 0

--- The x-value of the right boundary of the rectangle.
Rect.right = 0

--- The y-value of the top boundary of the rectangle.
Rect.top = 0

--- The y-value of the bottom boundary of the rectangle.
Rect.bottom = 0

--- This class represents a relationship component.
-- The root relationship serves as a tag for entities inside the scene hierarchy that
-- do have a parent entity. Entities that have no specified hierarchy status
-- (relationship or @{Root} component) won't be rendered.
-- @type Relationship

--- The parent entity.
Relationship.parent = Entity.new()

--- This class represents a root component.
-- The root component serves as a tag for entities inside the scene hierarchy that
-- don't have a parent entity. Entities that have no specified hierarchy status
-- (root or @{Relationship} component) won't be rendered.
-- @type Root

--- Ignore this.
-- This placeholder forces the root component to appear in the documentation.
-- It is needed since this is an empty type.
Root._ = ""

--- This class represents a shader program.
-- This is a handle to a shader program that resides in GPU memory. Handles can be
-- retrieved by calling @{c2k.assets.shaderProgram} and passing a global unique id (GUID)
-- as a string.
-- @type ShaderProgram

--- The global unique id (GUID) of this shader program.
ShaderProgram.guid = ""

--- This class represents a sprite sheet.
-- This is a handle to a sprite sheet. Handles can be retrieved by calling
-- @{c2k.assets.spriteSheet} and passing a global unique id (GUID) as a string.
-- @type SpriteSheet

--- The list of frames of this sprite sheet.
-- @see SpriteSheetFrame
SpriteSheet.frames = []

--- The texture that should be used with this sprite sheet.
-- @see Texture
SpriteSheet.texture = Texture.new()

--- This class represents a single frame of a @{SpriteSheet}.
-- @type SpriteSheetFrame

--- The texture rect for this sprite sheet frame as normalized coordinates.
SpriteSheetFrame.rect = Rect.new()

--- The width of the original sprite graphic in pixels.
SpriteSheetFrame.sourceWidth = 0

--- The height of the original sprite graphic in pixels.
SpriteSheetFrame.sourceHeight = 0

--- This class represents a texture.
-- This is a handle to a texture that resides in GPU memory. Handles can be
-- retrieved by calling @{c2k.assets.texture} and passing a global unique id (GUID)
-- as a string.
-- @type Texture
-- @usage print("The width of this texture is " .. c2k.assets.texture("dff006d0-3c6e-4a22-99ff-2b2af8fdc770").width .. " pixels")

--- The global unique id (GUID) of this texture.
Texture.guid = ""

--- The width of this texture in pixels.
Texture.width = 0

--- The height of this texture in pixels.
Texture.height = 0

--- The number of channels of this texture.
Texture.numChannels = 0

--- This class represents timing information.
-- The data provided inside an object of this type represents timing information
-- for the current frame. An object of this type should only be obtained by
-- calling @{getTime}.
-- @type Time
-- @usage print("The main loop is running since " .. getTime().elapsed .. " seconds")

--- The elapsed time since the start of the engine's main loop.
Time.elapsed = 0

--- The duration of the last frame.
Time.delta = 0

--- This class represents a transform component.
-- @type Transform

--- The position components of the transform.
-- The position is encoded as a three-dimensional vector.
-- @see Vec3
Transform.position = Vec3.new()

--- The rotation of the transform (in radians).
Transform.rotation = 0

--- The scale of the transform.
-- The scale is encoded as a two-dimensional vector.
-- @see Vec2
Transform.scale = Vec2.new()

--- This class represents a two-dimensional vector.
-- @type Vec2

--- The x-value.
Vec2.x = 0

--- The y-value.
Vec2.y = 0

--- This class represents a three-dimensional vector.
-- @type Vec3

--- The x-value.
Vec3.x = 0

--- The y-value.
Vec3.y = 0

--- The z-value.
Vec3.z = 0

return c2k