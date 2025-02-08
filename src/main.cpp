#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui-SFML.h"

#include "MandelbrotSet.h"

#include <SFML/Window.hpp>

#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>

std::string doubleToString(double num, int precision) {
	std::stringstream str;
	str << std::fixed << std::setprecision(precision) << num;
	return str.str();
}

int main()
{
	const int screenWidth = 1920;
	const int screenHeight = 1080;

	const float zoomMultiplier = 0.8;
	const int panAmount = 50;

	const char* defaultScriptFile = "scripts/default.lua";

	// Initialize Lua
	lua_State* lua = luaL_newstate();
	luaopen_base(lua);
	luaL_openlibs(lua);
	luaL_loadfile(lua, defaultScriptFile);

	// Initialize window
	sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Mandelbrot Visualizer", sf::Style::Titlebar | sf::Style::Close);
	ImGui::SFML::Init(window);

	MandelbrotSet mandelbrot(100, screenWidth, screenHeight, lua);
	mandelbrot.setImageSize(screenWidth, screenHeight);

	// Stores current values of input boxes
	int setMaxIter = mandelbrot.getMaxIterations();
	int setWidth = screenWidth;
	int setHeight = screenHeight;

	// Load default coloring script
	std::ifstream ifs(defaultScriptFile);
	std::string colorScript((std::istreambuf_iterator<char>(ifs)),
							(std::istreambuf_iterator<char>()));

	bool drawUi = true;
	bool textEditActive = false;

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(window, event);

			if (event.type == sf::Event::Closed) {
				window.close();
			} else if (event.type == sf::Event::KeyPressed && !textEditActive) {
				bool regen = false;
				switch (event.key.code) {
					case sf::Keyboard::Add: // Zoom in
						mandelbrot.zoomVal *= zoomMultiplier;
						regen = true;
						break;
					case sf::Keyboard::Subtract: // Zoom out
						mandelbrot.zoomVal /= zoomMultiplier;
						regen = true;
						break;
					case sf::Keyboard::Up:
					case sf::Keyboard::W:
						mandelbrot.yOffset -= panAmount * mandelbrot.zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Down:
					case sf::Keyboard::S:
						mandelbrot.yOffset += panAmount * mandelbrot.zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Left:
					case sf::Keyboard::A:
						mandelbrot.xOffset -= panAmount * mandelbrot.zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Right:
					case sf::Keyboard::D:
						mandelbrot.xOffset += panAmount * mandelbrot.zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Tab:
						drawUi = !drawUi;
						break;
					case sf::Keyboard::Space:
						sf::Image img = window.capture();
						img.saveToFile("export.png");
						break;
				}
				if (regen) {
					mandelbrot.generateSet();
				}
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		if (drawUi) {
			ImGui::Begin("Zoom");
			if (ImGui::Button("Zoom in")) {
				mandelbrot.zoomVal *= zoomMultiplier;
				mandelbrot.generateSet();
			}
			if (ImGui::Button("Zoom out")) {
				mandelbrot.zoomVal /= zoomMultiplier;
				mandelbrot.generateSet();
			}
			ImGui::End();

			ImGui::Begin("Options");
			if (ImGui::TreeNode("Render")) {
				if (ImGui::InputInt("Iterations", &setMaxIter, 10)) {
					if (setMaxIter < 0) {
						setMaxIter = 0;
					} else if (setMaxIter > 10000) {
						setMaxIter = 10000;
					}
				}
				if (ImGui::Button("Set")) {
					mandelbrot.setMaxIterations(setMaxIter);
					mandelbrot.generateSet();
				}
				ImGui::SameLine();
				if (ImGui::Button("Preview")) {
					int temp = mandelbrot.getMaxIterations();
					mandelbrot.setMaxIterations(setMaxIter);
					mandelbrot.generateSet();
					mandelbrot.setMaxIterations(temp);
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Scripting")) {
				ImGui::InputTextMultiline("script", &colorScript, ImVec2(500, 500), ImGuiInputTextFlags_AllowTabInput);
				// Disable keyboard controls while the text editor is active
				textEditActive = ImGui::IsItemActive();
				if (ImGui::Button("Update")) {
					luaL_loadstring(lua, colorScript.data());
					mandelbrot.setLuaState(lua);
					mandelbrot.generateSet();
				}
				ImGui::TreePop();
			}

			ImGui::End();

			ImGui::Begin("Info");
			double zoomPercentage = 0.003 / mandelbrot.zoomVal;
			std::string zoomValStr = "Zoom: ";
			if (zoomPercentage > 100000) {
				// Format using scientific notation
				std::ostringstream str;
				str << std::setprecision(2) << std::scientific << zoomPercentage;
				zoomValStr += str.str();
			} else {
				// Round to 2 decimal places and remove trailing zeros
				std::string zpRounded = std::to_string(std::ceil(zoomPercentage * 100.0) / 100.0);
				zpRounded.erase(zpRounded.find_last_not_of('0') + 1, std::string::npos);
				zpRounded.erase(zpRounded.find_last_not_of('.') + 1, std::string::npos);
				zoomValStr += zpRounded;
			}
			ImGui::Text(zoomValStr.c_str());

			std::string xOffStr = "X offset: " + doubleToString(mandelbrot.xOffset, 12);
			ImGui::Text(xOffStr.c_str());

			std::string yOffStr = "Y offset: " + doubleToString(mandelbrot.yOffset, 12);
			ImGui::Text(yOffStr.c_str());

			std::string timeStr = "Render time: " + std::to_string(mandelbrot.latestGenTime) + " ms";
			ImGui::Text(timeStr.c_str());
			ImGui::End();

		}

		window.clear();
		window.draw(mandelbrot.getSet());
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	lua_close(lua);

	return 0;
}
