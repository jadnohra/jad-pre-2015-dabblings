
Camera = class(function(a)
   a.scale = 1.0
   a.pos = Vector2D(0.0, 0.0)
   a.dragRefPos = nil
   a.startDragPos = nil
end)


function Camera:transform(vector2D)
	return Vector2D((self.scale * vector2D[1] + self.pos[1]), (self.scale * vector2D[2] + self.pos[2]))
end	

function Camera:invTransform(vector2D)
	return Vector2D((vector2D[1] - self.pos[1]) / self.scale, (vector2D[2] - self.pos[2]) / self.scale)
end	

function Camera:update(dt)
	
	if love.mouse.isDown(love.mouse_right) then 
		
		if (self.startDragPos == nil) then
			self.dragRefPos = self.pos
			self.startDragPos = self:invTransform(Vector2D(love.mouse.getX(), love.mouse.getY()))
		end
		
		local newPos = self:invTransform(Vector2D(love.mouse.getX(), love.mouse.getY()))
		self.pos[1] = self.dragRefPos[1] + newPos[1] - self.startDragPos[1]
		self.pos[2] = self.dragRefPos[2] + newPos[2] - self.startDragPos[2]
	
	else	
	
		self.startDragPos = nil
	
	end 
	
end	