#include "MandelbrotSet.h"

#include <SFML/System/Clock.hpp>
#include <chrono>
#include <cmath>

MandelbrotSet::MandelbrotSet(int maxIterations_, int imageWidth_, int imageHeight_, lua_State* lua_) {
	// Set the VertexArray's primitive type to Points so we can manipulate individual pixels
	set.setPrimitiveType(sf::Points);
	setImageSize(imageWidth_, imageHeight_);
	maxIterations = maxIterations_;

	setLuaState(lua_);
	generateSet();
}

void MandelbrotSet::generateSet() {
	auto start = std::chrono::steady_clock::now();
	#pragma omp parallel for
	for (int x = 0; x < imageWidth; x++) {
		for (int y = 0; y < imageHeight; y++) {
			// Scale pixel coordinates
			double x0 = (x - imageWidth / 2.0) * zoomVal + xOffset;
			double y0 = (y - imageHeight / 2.0) * zoomVal + yOffset;

			int val;
			if (pointInCardioidOrBulb(x0, y0)) {
				val = maxIterations;
			} else {
				// Determine whether the point is in the Mandelbrot set
				val = calculatePoint(x0, y0);
			}

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

int MandelbrotSet::calculatePoint(double x0, double y0) {
	double x = 0;
	double y = 0;
	int iteration = 0;

	// Calculate number of iterations using escape time algorithm
	while (x*x + y*y <= 4 && iteration < maxIterations) {
		double xtemp = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = xtemp;
		iteration++;
	}

	return iteration;
}

bool MandelbrotSet::pointInCardioidOrBulb(double x, double y) {
	// Check if point is in cardioid
	double p = std::sqrt((double)(std::pow(x-0.25,2)+std::pow(y,2)));
	if (x <= p-2*std::pow(p,2)+0.25) { return true; }
	
	// Check if point is in period-2 bulb
	if (std::pow(x+1,2)+std::pow(y,2) <= 1/16) { return true; }

	return false;
}
