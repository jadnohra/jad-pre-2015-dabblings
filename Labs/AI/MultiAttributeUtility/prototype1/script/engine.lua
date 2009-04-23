
Engine = class(function(a)
	a.camera = Camera()
	a.UI = UI()
	a.screenSize = Vector2D(800.0, 600.0)
end)

function Engine:load()
	
	love.graphics.setBackgroundColor(0xff, 0xf1, 0xf7)
	love.graphics.setMode(self.screenSize[1], self.screenSize[2], false, false, 0 ) 
	love.graphics.setFont(love.default_font, 12)
	
	self.camera.scale = 20.0	
	self.camera.pos[1] = 0.5 * self.screenSize[1]
	self.camera.pos[2] = 0.5 * self.screenSize[2]
	
end

function Engine:update(dt)
	
	self.camera:update(dt)
	self.UI:update(self, dt)	
end

function Engine:draw()

	self.UI:draw(self, self.camera)	
	
end	