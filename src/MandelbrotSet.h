#pragma once

#include "Set.h"

class MandelbrotSet : public Set {
	public:
		MandelbrotSet(int maxIterations_, int imageWidth_, int imageHeight_, lua_State* lua_);
		void generateSet();

	private:
		int calculatePoint(double x0, double y0);
		bool pointInCardioidOrBulb(double x, double y);
};
