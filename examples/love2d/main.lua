local clay = require "clay"
local scrollX, scrollY = 0, 0

local COLOR_LIGHT      = { r = 224, g = 215, b = 210, a = 255 }
local COLOR_RED        = { r = 168, g =  66, b =  28, a = 255 }
local COLOR_ORANGE     = { r = 225, g = 138, b =  50, a = 255 }
local COLOR_BACKGROUND = { r = 250, g = 250, b = 255, a = 255 }
local COLOR_WHITE      = { r = 255, g = 255, b = 255, a = 255 }

local sidebarItemConfig = {
  layout = {
    sizing = { width = { type = "grow", minMax = 0 }, height = 50 }
  },
  backgroundColor = COLOR_ORANGE
}

local sidebarConfig = {
  id = "SideBar",
  layout = { 
    layoutDirection = "column", 
    sizing = { 
      width = 300,
      height = "grow"
    },
    padding = 16, 
    childGap = 16
  }
}

local outerConfig = {
  id = "OuterContainer",
  layout = { 
    sizing = "grow",
    padding = 16, 
    childGap = 16
  },
  backgroundColor = COLOR_BACKGROUND
}

local outerPictureConfig = {
  id = "ProfilePictureOuter",
  layout = { sizing = { width = "grow" } },
  padding = 16,
  childGap = 16,
  childAlignment = { y = "center" },
  backgroundColor = COLOR_RED
}

local pictureConfig = { 
  id = "ProfilePicture",
  layout = {
    sizing = {
      width = 60, 
      height = 60
    }
  },
  image = {
    imageData = love.graphics.newImage("profile-picture.png"),
    sourceDimensions = { width = 300, height = 300 }
  }
}

local textConfig = { fontSize = 12, textColor = COLOR_WHITE }

local font = love.graphics.newFont()

local function SidebarItemComponent()
  clay(sidebarItemConfig)
end

local function MainContent()
  clay {
    id = "MainContent",
    layout = {
      sizing = "grow"
    },
    backgroundColor = COLOR_LIGHT
  }
end

local COMMAND_RENDERS = {}

function COMMAND_RENDERS.rectangle(command)
  local bb = command.boundingBox
  local color = command.backgroundColor
  love.graphics.setColor(color.r / 255, color.g / 255, color.b / 255, color.a / 255)
  love.graphics.rectangle("fill", bb.x, bb.y, bb.width, bb.height)
end

function COMMAND_RENDERS.border(command)
  local bb = command.boundingBox
end

function COMMAND_RENDERS.text(command)
  local bb = command.boundingBox
  local color = command.textColor
  love.graphics.setFont(font)
  love.graphics.setColor(color.r / 255, color.g / 255, color.b / 255, color.a / 255)
end

function COMMAND_RENDERS.image(command)
  local bb = command.boundingBox
  local img = command.imageData
end

function COMMAND_RENDERS.scissorStart(command)
  local bb = command.boundingBox
  love.graphics.setScissor(bb.x, bb.y, bb.width, bb.height)
end

function COMMAND_RENDERS.scissorEnd(command)
  love.graphics.setScissor()
end

local function measureText(text, config)
  return font:getWidth(text), font:getHeight() * font:getLineHeight()
end

function love.load()
  clay.initialize(love.graphics.getDimensions())
  clay.setMeasureTextFunction(measureText)
end

function love.update(dt)
  clay.setLayoutDimensions(love.graphics.getDimensions())
  local x, y = love.mouse.getPosition()
  clay.setPointerState(x, y, love.mouse.isDown(1))
  clay.updateScrollContainers(true, scrollX, scrollY, dt)
  scrollX, scrollY = 0, 0
end

function love.draw()
  clay.beginLayout()
  clay(outerConfig, function ()
    clay(sidebarConfig, function ()
      clay(outerPictureConfig, function ()
        clay(pictureConfig)
        clay.text("Clay - UI Library", textConfig)
      end)
      for i = 1, 5 do
        SidebarItemComponent()
      end
    end)
    MainContent()
  end)
  local commands = clay.endLayout()
  love.graphics.clear()
  for _, command in ipairs(commands) do
    local render = COMMAND_RENDERS[command.commandType]
    if render then
      render(command)
    end
  end
end

function love.wheelmoved(x, y)
  scrollX, scrollY = x, y
end
