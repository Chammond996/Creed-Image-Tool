#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Main.hpp"
#include "vector"
#include "imgui.h"
#include "imgui-SFML.h"
#include "ImGuiFileDialog.h"
#include <iostream>
#include <filesystem>
#include <memory>


struct ImageData
{
	sf::Image img;
	std::string fileName;
	sf::Texture texture;
};
struct ColourChangeData
{
	sf::Uint16 imageIndex;
	sf::Uint16 x;
	sf::Uint16 y;
	sf::Color oldColour;
};
struct ColourChanges
{
	sf::Color newColour;
	std::vector<std::unique_ptr<ColourChangeData>> changeData;
};
class UI
{
private:
	const int WINDOW_WIDTH = 1280;
	const int WINDOW_HEIGHT = 720;
	const std::string WINDOW_NAME = "Creed Image Tool 1.0.0";

	bool running = false;
	bool needsDraw = false;

	const float MIN_SCALE = 1.0;
	const float MAX_SCALE = 10.f;
	const int MIN_SLICE_WIDTH = 16;
	const int MAX_SLICE_WIDTH = 128;
	sf::Uint8 delayDraw;

	enum FUNCTION_TYPE
	{
		RECOLOUR = 0,
		SLICER = 1
	};
	FUNCTION_TYPE functionType;

	bool changeBackGroundColourMenu = false;
	std::vector<std::string> directoryBrowserList;
	std::vector<std::string> filesToLoadList;

	std::vector<std::unique_ptr<ColourChanges>> colourChanges;


	float backgroundColour[4] = { 0.1f, 0.1f, 0.1f, 0.0f }; // Initial RGBA color
	float colourToReplace[4] = { 0.1f, 0.1f, 0.1f, 0.0f }; // Initial RGBA color
	float newColour[4] = { 0.1f, 0.1f, 0.1f, 0.0f }; // Initial RGBA color

	int activeImageID = 0;
	float imageScale = 1.f;
	int slicerWidth = 0;
	int activeSliceID = 0;
	bool cutSlices = false;

	void ShowPopUP(const std::string& title);

	void ShowPopUP(const std::string& title, std::vector<std::string>);
	bool showPopUp = false;
	std::string popUpDialogTitle;
	std::vector<std::string> popUpDialogs;
	
	
	sf::Sprite imageSprite;

	std::vector<std::unique_ptr<ImageData>> imageData;
	std::vector<sf::RectangleShape> paletteColours;
	std::vector<sf::RectangleShape> slicerBoxes;

	void DrawImGuiMenu();
	void LoadImages(std::vector<std::string> names);
	void CreateImagePalette();
	void CreateSlicerBoxes();
	void ClickSelectPaletteColour(const sf::Color& colour);
	void Tick(sf::RenderWindow& window);
	void Click(sf::Vector2i pos, bool rightClick = false);
	void UpdateSlicerVisualSelected();
	void UpdateColourChanges(sf::Color colourToReplace, sf::Color newColour);
	void Undo();
	void Save();
public:

	UI();
	~UI();
};

