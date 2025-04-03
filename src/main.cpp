#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui-SFML.h"

#include "MandelbrotSet.h"
#include "JuliaSet.h"

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

enum SetType {
	Mandelbrot,
	Julia
};

void changeSet(int setType, int iterations, int width, int height, lua_State* lua, std::shared_ptr<Set> &set) {
	switch (setType) {
		case SetType::Mandelbrot:
			set = std::make_unique<MandelbrotSet>(iterations, width, height, lua);
			break;
		case SetType::Julia:
			set = std::make_unique<JuliaSet>(iterations, width, height, lua);
			break;
	}
}

int main()
{
	const int screenWidth = 1920;
	const int screenHeight = 1080;

	const double defaultReal = -0.8;
	const double defaultImaginary = 0.2;

	const float zoomMultiplier = 0.8;
	const int panAmount = 50;
	const int defaultIterations = 100;

	const char* defaultScriptFile = "scripts/default.lua";

	// Initialize Lua
	lua_State* lua = luaL_newstate();
	luaopen_base(lua);
	luaL_openlibs(lua);
	luaL_loadfile(lua, defaultScriptFile);

	// Initialize window
	sf::RenderWindow window(sf::VideoMode(screenWidth, screenHeight), "Mandelbrot Visualizer", sf::Style::Titlebar | sf::Style::Close);
	ImGui::SFML::Init(window);

	static int chosenSet = SetType::Mandelbrot;
	int currentSetType = SetType::Mandelbrot;
	std::shared_ptr<Set> set = std::make_unique<MandelbrotSet>(defaultIterations, screenWidth, screenHeight, lua);

	// Stores current values of input boxes
	int setMaxIter = set->getMaxIterations();
	int setWidth = screenWidth;
	int setHeight = screenHeight;
	double setReal = defaultReal;
	double setImaginary = defaultImaginary;

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
						set->zoomVal *= zoomMultiplier;
						regen = true;
						break;
					case sf::Keyboard::Subtract: // Zoom out
					set->zoomVal /= zoomMultiplier;
						regen = true;
						break;
					case sf::Keyboard::Up:
					case sf::Keyboard::W:
						set->yOffset -= panAmount * set->zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Down:
					case sf::Keyboard::S:
						set->yOffset += panAmount * set->zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Left:
					case sf::Keyboard::A:
						set->xOffset -= panAmount * set->zoomVal;
						regen = true;
						break;
					case sf::Keyboard::Right:
					case sf::Keyboard::D:
						set->xOffset += panAmount * set->zoomVal;
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
					set->generateSet();
				}
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		if (drawUi) {
			ImGui::Begin("Zoom");
			if (ImGui::Button("Zoom in")) {
				set->zoomVal *= zoomMultiplier;
				set->generateSet();
			}
			if (ImGui::Button("Zoom out")) {
				set->zoomVal /= zoomMultiplier;
				set->generateSet();
			}
			ImGui::End();

			ImGui::Begin("Options");
			if (ImGui::TreeNode("Render")) {
				ImGui::RadioButton("Mandelbrot", &chosenSet, SetType::Mandelbrot); ImGui::SameLine();
				ImGui::RadioButton("Julia", &chosenSet, SetType::Julia);

				// Check if a new fractal type was chosen
				if (chosenSet != currentSetType) {
					currentSetType = chosenSet;
					changeSet(chosenSet, defaultIterations, screenWidth, screenHeight, lua, set);
					setReal = defaultReal;
					setImaginary = defaultImaginary;
				}

				if (ImGui::InputInt("Iterations", &setMaxIter, 10)) {
					if (setMaxIter < 0) {
						setMaxIter = 0;
					} else if (setMaxIter > 10000) {
						setMaxIter = 10000;
					}
				}
				if (ImGui::Button("Set")) {
					set->setMaxIterations(setMaxIter);
					set->generateSet();
				}
				ImGui::SameLine();
				if (ImGui::Button("Preview")) {
					int temp = set->getMaxIterations();
					set->setMaxIterations(setMaxIter);
					set->generateSet();
					set->setMaxIterations(temp);
				}

				if (currentSetType == SetType::Julia) {
					ImGui::InputDouble("Real", &setReal, 0.01);
					ImGui::InputDouble("Imaginary", &setImaginary, 0.01);
					if (ImGui::Button("Set parameters")) {
						// The Julia option is selected so we can safely cast the pointer
						auto julia = std::dynamic_pointer_cast<JuliaSet>(set);
						julia->cx = setReal;
						julia->cy = setImaginary;
						set->generateSet();
					}
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Scripting")) {
				ImGui::InputTextMultiline("script", &colorScript, ImVec2(500, 500), ImGuiInputTextFlags_AllowTabInput);
				// Disable keyboard controls while the text editor is active
				textEditActive = ImGui::IsItemActive();
				if (ImGui::Button("Update")) {
					luaL_loadstring(lua, colorScript.data());
					set->setLuaState(lua);
					set->generateSet();
				}
				ImGui::TreePop();
			}

			ImGui::End();

			ImGui::Begin("Info");
			double zoomPercentage = 0.003 / set->zoomVal;
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

			std::string xOffStr = "X offset: " + doubleToString(set->xOffset, 12);
			ImGui::Text(xOffStr.c_str());

			std::string yOffStr = "Y offset: " + doubleToString(set->yOffset, 12);
			ImGui::Text(yOffStr.c_str());

			std::string timeStr = "Render time: " + std::to_string(set->latestGenTime) + " ms";
			ImGui::Text(timeStr.c_str());
			ImGui::End();

		}

		window.clear();
		window.draw(set->getSet());
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
	lua_close(lua);

	return 0;
}