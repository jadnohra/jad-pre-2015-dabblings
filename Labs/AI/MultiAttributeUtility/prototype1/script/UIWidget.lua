
UIWidget = class(function(a)
end)

function pickWidget(widgets, point)

	for i,widget in ipairs(widgets) do
		
		if (widget:contains(point)) then
			return widget
		end
	end

	return nil
end