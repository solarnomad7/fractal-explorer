#include "JuliaSet.h"

#include <SFML/System/Clock.hpp>
#include <chrono>
#include <cmath>

JuliaSet::JuliaSet(int maxIterations_, int imageWidth_, int imageHeight_, lua_State* lua_) {
    // Set the VertexArray's primitive type to Points so we can manipulate individual pixels
	set.setPrimitiveType(sf::Points);
	setImageSize(imageWidth_, imageHeight_);
	maxIterations = maxIterations_;

	setLuaState(lua_);
	generateSet();
}

void JuliaSet::generateSet() {
    auto start = std::chrono::steady_clock::now();
    #pragma omp parallel for
    for (int x = 0; x < imageWidth; x++) {
        for (int y = 0; y < imageHeight; y++) {
            // Scale pixel coordinates
			double x0 = (x - imageWidth / 2.0) * zoomVal + xOffset;
			double y0 = (y - imageHeight / 2.0) * zoomVal + yOffset;

            int val = calculatePoint(x0, y0);

            // Convert coordinates to array index
			int index = x + y * imageWidth;

			set[index].position = sf::Vector2f(x, y);

			// Set pixel color based on number of iterations
			if (val <= maxIterations) {
				set[index].color = palette[val];
			} else {
				set[index].color = sf::Color::Black;
			}
        }
    }

    auto end = std::chrono::steady_clock::now();
	latestGenTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
}

int JuliaSet::calculatePoint(double x, double y) {
    int iteration = 0;

    // Calculate number of iterations using escape time algorithm
    while (x*x + y*y <= 4 && iteration < maxIterations) {
        double xtemp = x*x - y*y + cx;
        y = 2*x*y + cy;
        x = xtemp;
        iteration++;
    }

    return iteration;
}