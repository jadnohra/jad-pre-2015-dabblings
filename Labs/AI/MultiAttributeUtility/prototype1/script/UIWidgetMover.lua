
UIWidgetMover = class(function(a)
	a.isMoving = false
	a.pickedWidget = nil
end)

function UIWidgetMover:update(camera, widgets)
	
	if love.mouse.isDown(love.mouse_left) then 
		
		local picked = nil
		
		if (self.isMoving) then
			picked = self.pickedEntity
		else
		
			local mousePoint = camera:invTransform(Vector2D(love.mouse.getX(), love.mouse.getY()))
			picked = pickWidget(widgets, mousePoint)
		end
		
		self.pickedEntity = picked
		self.isMoving = true

		if (picked ~= nil) then
			local mousePoint = camera:invTransform(Vector2D(love.mouse.getX(), love.mouse.getY()))
			picked:moveTo(mousePoint)
		end		
		
	else	
	
		self.isMoving = false
	
	end 
	
end	
