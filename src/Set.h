#pragma once

extern "C" {
    #include "lua.h"
    #include "lualib.h"
	#include "lauxlib.h"
}

#include <SFML/Graphics.hpp>
#include <map>

class Set {
    public:
        virtual void generateSet() = 0;
        void setMaxIterations(int newMaxIterations);
        void setImageSize(int width, int height);
        void setLuaState(lua_State* newState);
        int const& getMaxIterations() const;
        sf::VertexArray const& getSet() const;

        int latestGenTime = 0;
        double zoomVal = 0.003;
        double xOffset = -0.7;
        double yOffset = 0.0;

    protected:
        virtual int calculatePoint(double x0, double y0) = 0;
        std::map<int, sf::Color> generatePalette();

        int maxIterations;
        int imageHeight;
        int imageWidth;
        sf::VertexArray set;
        std::map<int, sf::Color> palette;
        lua_State* lua;
};
