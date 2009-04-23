
UI = class(function(a)
	a.disabled = {}
	a.enabled = {}
	a.widgetMover = UIWidgetMover()
end)

function UI:update(engine, dt)
	
	self.widgetMover:update(engine.camera, self.enabled)
		
end

function UI:draw(engine,camera)

	love.graphics.setColor(0, 0, 0, 255)

	for i,widget in ipairs(self.enabled) do
		widget:draw(engine, camera)
	end
	
end	