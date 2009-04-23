
Box2D = class(function(a, x, y, w, h)
   a.pos = Vector2D(x, y)
   a.size = Vector2D(w, h)
end)

function Box2D:contains(vector2D)
	return math.abs(self.pos[1] - vector2D[1]) <= self.size[1] and math.abs(self.pos[2] - vector2D[2]) <= self.size[2];
end	
