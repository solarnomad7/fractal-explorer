#include "Set.h"

void Set::setMaxIterations(int newMaxIterations) {
    maxIterations = newMaxIterations;
    palette = generatePalette();
}

void Set::setImageSize(int width, int height) {
    if (width > 0 && height > 0) {
        imageWidth = width;
        imageHeight = height;
        set.resize(width * height);
    }
}

void Set::setLuaState(lua_State* newState) {
    lua = newState;
	lua_pcall(lua, 0, 0, 0);
	palette = generatePalette();
}

int const& Set::getMaxIterations() const {
	return maxIterations;
}

sf::VertexArray const& Set::getSet() const {
	return set;
}

std::map<int, sf::Color> Set::generatePalette() {
	std::map<int, sf::Color> palette;
	for (int i = 0; i < maxIterations+1; i++) {
		lua_getglobal(lua, "setPaletteColor");
		lua_pushnumber(lua, i);
		lua_pushnumber(lua, maxIterations);

		lua_call(lua, 2, 3);
		int r = (int)lua_tonumber(lua, -3);
		int g = (int)lua_tonumber(lua, -2);
		int b = (int)lua_tonumber(lua, -1);

		lua_pop(lua, 3);

		palette[i] = sf::Color(r, g, b);
	}

	return palette;
}
