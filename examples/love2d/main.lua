local clay = require "clay"

function love.load()
  clay.initialize(love.graphics.getDimensions())
end

function love.update(dt)
  clay.setLayoutDimensions(love.graphics.getDimensions())
  local x, y = love.mouse.getPosition()
  clay.setPointerState(x, y, love.mouse.isDown(1))
end

function love.draw()
  clay.beginLayout()
  for _, command in ipairs(clay.endLayout()) do
  end
end
