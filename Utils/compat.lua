-- Execute this script using dofile("compat.lua") at the start of your old
-- Lua module to ensure compatibility with the new version of Passerelle

if (passerelle ~= nil) then
	-- Compatibility
	saitek = passerelle;

	-- Save all the object properties
	local list = {};
	for key,value in pairs(passerelle) do
		list[key] = value;
	end

	-- Iterate and add old uppercase method names
	for key,value in pairs(list) do
		local char = string.sub(key, 1, 1);

		if (char ~= string.upper(char)) then
			local upper = string.upper(char) .. string.sub(key, 2)
			saitek[upper] = function(...)
				print("passerelle: the method '" .. upper .. "' is obsolete");
				return passerelle[key](...);
			end
		end
	end
end
