#include "UI.h"

UI::~UI()
{
}
UI::UI()
{
    this->functionType = FUNCTION_TYPE::RECOLOUR;
    this->slicerWidth = MIN_SLICE_WIDTH;

    this->delayDraw = 0;

	// Set window size,name and view
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_NAME, sf::Style::Close);
	sf::View view(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2), sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
	window.setView(view);

    sf::Clock deltaClock;

	// init ImGui
	ImGui::SFML::Init(window);
    this->running = true;

    // init the BG to black
    this->needsDraw = true;

    while (window.isOpen() && this->running)
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(window, event);

            if(window.hasFocus())
                this->needsDraw = true;

            if (event.type == sf::Event::Closed)
            {
                this->running = false;
            }
            else if (event.type == sf::Event::Resized)
            {
                view.setSize(sf::Vector2f(window.getSize().x, window.getSize().y));
                view.setCenter(sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2));
                window.setView(view);


            }
            else if (event.type == sf::Event::MouseButtonPressed && window.hasFocus())
            {
                bool rightClick = event.mouseButton.button == sf::Mouse::Right ? true : false;
                this->Click(sf::Vector2i(sf::Mouse::getPosition(window)), rightClick);
            }
            else if (event.type == sf::Event::MouseMoved && window.hasFocus())
            {
               
            }
        }

        if (this->needsDraw)
        {
            // Start the ImGui frame
            ImGui::SFML::Update(window, deltaClock.restart());

            // Clear the window
            window.clear(sf::Color(backgroundColour[0] * 255.0f, backgroundColour[1] * 255.0f, backgroundColour[2] * 255.0f));


            this->Tick(window);

            

            if (this->showPopUp)
                this->ShowPopUP(this->popUpDialogTitle);


            // Render ImGui contents
            ImGui::SFML::Render(window);

            window.display();

            this->needsDraw = false;
        }

        if (this->delayDraw < 5)
        {
            if (++this->delayDraw == 4)
                this->needsDraw = true;
        }
        
        sf::sleep(sf::milliseconds(33));
    }
    ImGui::SFML::Shutdown();
    window.close();
}

void UI::ShowPopUP(const std::string& title, std::vector<std::string> dialogs)
{
    for (auto& msg : dialogs)
        this->popUpDialogs.emplace_back(msg);

    this->ShowPopUP(title.c_str());
}
void UI::ShowPopUP(const std::string& title)
{
    if (this->popUpDialogs.empty())
    {
        this->showPopUp = false;
        return;
    }
    this->showPopUp = true;
    this->popUpDialogTitle = title;
    
    ImGui::SetNextWindowPos(ImVec2(200, 100)); // Position the overlay below the menu bar
    ImGui::SetNextWindowSize(ImVec2(400, 200)); // Set the size of the overlay
    ImGui::Begin(this->popUpDialogTitle.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    for (auto& dialog : this->popUpDialogs)
        ImGui::Text(dialog.c_str());
    
    if (ImGui::Button("Close"))
    {
        this->showPopUp = false;
        this->popUpDialogs.clear();
    }

    ImGui::End();
   
}

void UI::DrawImGuiMenu()
{
    // Create a main menu bar
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Files"))
        {
            if (ImGui::MenuItem("Load Images"))
            {
                if (!std::filesystem::exists("bmps/"))
                {
                    if(std::filesystem::create_directory("bmps/"))
                        this->ShowPopUP("Nothing to load!", { "A bmps/ folder has been created next to the Creed","Image Tool exe.", "You can place your bmps you wish to edit in there." });
                    
                    return;
                }
                IGFD::FileDialogConfig config;
                config.path = "bmps/";
                config.countSelectionMax = 100;
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlg", "Load Images", ".bmp", config);
            }
            if (ImGui::MenuItem("Save Images"))
            {
                this->Save();
            }
            
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::MenuItem("Change background colour"))
            {
                this->changeBackGroundColourMenu = true;
            }
            if (ImGui::MenuItem("Help/Info"))
            {
                if (!this->showPopUp)
                {
                    this->ShowPopUP("Help/Info..", { "TODO: The slicer won't position the boxes unless", "the background of the sprite is 0,0,0",
                        "There is a few cases where the slicer will target empty", " spaces on the image.. Fix TBD", "Written by Taff (2024)"});
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Functions"))
        {
            if (ImGui::MenuItem("Recolour Tool"))
            {
                this->functionType = FUNCTION_TYPE::RECOLOUR;
                this->imageSprite.setScale(sf::Vector2f(this->imageScale, this->imageScale));
                this->delayDraw = 0;
            }
            else if (ImGui::MenuItem("Slicer"))
            {
                this->imageSprite.setScale(sf::Vector2f(1, 1));
                this->functionType = FUNCTION_TYPE::SLICER;
                this->CreateSlicerBoxes();
                this->delayDraw = 0;
            }
            /*
            if (this->slicerMenu)
            {
                if (ImGui::MenuItem("Cut Slices"))
                {
                    this->cutSlices = true;
                }
            }
            */
            ImGui::EndMenu();
        }
        if (ImGui::Button("Undo"))
        {
            this->Undo();
            this->delayDraw = 0;
        }
        ImGui::EndMainMenuBar();
    }

    if (this->changeBackGroundColourMenu)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 100));
        ImGui::Begin("Settings", &this->changeBackGroundColourMenu);
        ImGui::ColorEdit3("Set background colour", backgroundColour);
        if (ImGui::Button("Close"))
        {
            this->delayDraw = 0;
        }
        ImGui::End();


        if (!this->changeBackGroundColourMenu)
            this->needsDraw = true;
    }
}

void UI::LoadImages(std::vector<std::string> names)
{
    this->imageData.clear();
    this->activeImageID = 0;

    if (names.empty()) return;

    for (int i = 0; i < names.size(); i++)
    {
        sf::Image img;
        if (img.loadFromFile("bmps/" + names[i]))
        {
            std::unique_ptr<ImageData> imgData = std::make_unique<ImageData>();
            imgData->fileName = names[i];
            imgData->img = img;
            imgData->texture.loadFromImage(img);
            this->imageData.emplace_back(std::move(imgData));


#ifdef _DEBUG
            std::cout << "Loaded image" << names[i] << "\n";
#endif // _DEBUG

        }
    }
    this->imageSprite.setTexture(this->imageData[this->activeImageID]->texture, true);

    this->imageSprite.setPosition(150, 100);

    this->CreateImagePalette();

    this->delayDraw = 2;
}

void UI::CreateImagePalette()
{
    if (this->activeImageID >= this->imageData.size()) return;

    this->paletteColours.clear();

    sf::Image& ref = this->imageData[this->activeImageID]->img;

    std::vector<sf::Color> palette;

    for (int x = 0; x < ref.getSize().x; x++)
    {
        for (int y = 0; y < ref.getSize().y; y++)
        {
            sf::Color pxColour = ref.getPixel(x, y);

            bool found = false;

            for (auto& pal : palette)
            {
                if (pal == pxColour)
                    found = true;
            }

            if (!found && pxColour != sf::Color(0,0,0))
                palette.emplace_back(pxColour);
        }
    }
    int palItemCount = 0;
    int x_ = 5;
    int y_ = 0;
    for(auto&pal : palette)
    {
        sf::RectangleShape shape(sf::Vector2f(30, 30));
        shape.setFillColor(pal);
        shape.setPosition(x_, 100 + y_);
        this->paletteColours.emplace_back(shape);

        y_ += 35;

        if (++palItemCount % 15 == 0)
        {
            x_ += 40;
            y_ = 0;
        }
    }

    //this->CreateSlicerBoxes();
}

void UI::CreateSlicerBoxes()
{
    if (this->imageData.empty()) return;

    std::vector<int> pixelsFoundAtTop;
    std::vector<int> pixelsFoundAtBottom;
    sf::Image& ref = this->imageData[this->activeImageID]->img;

    this->slicerBoxes.clear();

    for (int x = 0; x < ref.getSize().x; x++)
    {
        bool found = false; // for borders n shit where it'll scan directly down
        for (int y = 0; y < ref.getSize().y; y++)
        {
            try
            {
                sf::Color pxColour = ref.getPixel(x, y);

                if (pxColour != sf::Color(0, 0, 0))
                {
                    found = true;
                    pixelsFoundAtTop.emplace_back(y);
                    break;
                }
            }
            catch (...)
            {
                /// needs stuff
            }
        }
        if (!found)
            pixelsFoundAtTop.emplace_back(ref.getSize().y - 1);
    }
    for (int x = 0; x < ref.getSize().x; x++)
    {
        bool found = false; // for borders n shit where it'll scan directly down
        for (int y = ref.getSize().y - 1; y > 0; y--)
        {
            try
            {
                sf::Color pxColour = ref.getPixel(x, y);

                if (pxColour != sf::Color(0, 0, 0))
                {
                    found = true;
                    pixelsFoundAtBottom.emplace_back(y);
                    break;
                }
            }
            catch (...)
            {
                /// needs stuff
            }
        }
        if (!found)
            pixelsFoundAtBottom.emplace_back(0); // just to add a reference for the loop later..
    }
    int sliceID = 0;

    for (int i = 0; i < ref.getSize().x - 16; i += this->slicerWidth)
    {
        sf::RectangleShape slice(sf::Vector2f(this->slicerWidth, ref.getSize().y));
        slice.setFillColor(sf::Color::Transparent);
        slice.setOutlineColor(sf::Color::Red);
        slice.setOutlineThickness(1.f);
        slice.setPosition(150 + i, 100);

        int lowestTop = 200;
        int highestBottom = 0;
        // top
        for (int a = i; a < i + this->slicerWidth; a++)
        {
            if (a >= pixelsFoundAtTop.size() - 1 || a > ref.getSize().x) break;

            if (pixelsFoundAtTop[a] < lowestTop)
                lowestTop = pixelsFoundAtTop[a];
        }
        // bottom black space
        for (int a = i; a < i + this->slicerWidth; a++)
        {
            if (a >= pixelsFoundAtBottom.size() - 1 || a > ref.getSize().x) break;

            if (pixelsFoundAtBottom[a] > highestBottom)
                highestBottom = pixelsFoundAtBottom[a];
        }
        slice.setPosition(slice.getPosition().x, slice.getPosition().y + lowestTop);
        slice.setSize(sf::Vector2f(this->slicerWidth, slice.getSize().y - lowestTop));
        slice.setSize(sf::Vector2f(this->slicerWidth, slice.getSize().y - (ref.getSize().y - highestBottom)));

        this->slicerBoxes.emplace_back(slice);

        
        if (this->cutSlices)
        {
            if (!std::filesystem::exists("slices/"))
            {
                if (std::filesystem::create_directory("slices/"))
                    this->popUpDialogs.emplace_back("Created slices/");
            }
            if (slice.getSize().x < 0 || slice.getSize().y < 0) continue;
            sf::Image img;
            img.create(slice.getSize().x, slice.getSize().y, sf::Color(0, 0, 0));
            
            int startX = slice.getPosition().x - (150 + i);
            int startY = slice.getPosition().y - 100;

            if (startX < 0 || startY < 0)
            {
                this->popUpDialogs.emplace_back("Failed to save slice " + this->imageData[this->activeImageID]->fileName + "-" + std::to_string(sliceID + 1) + ".bmp" + "\" (" + std::to_string(img.getSize().x) + "x" + std::to_string(img.getSize().y)+")");
                continue;
            }
            int endX = slice.getSize().x;

            if (endX + slice.getPosition().x - 150 > ref.getSize().x)
            {
                endX = (endX + slice.getPosition().x - 20) - ref.getSize().x;
            }

            img.copy(ref, 0, 0, sf::IntRect(slice.getPosition().x - 150, slice.getPosition().y - 100, endX, slice.getSize().y));
            //std::cout << "Saving..\n";

            if (!std::filesystem::exists("slices/"))
                std::filesystem::create_directory("slices/");

            img.saveToFile("slices/" + this->imageData[this->activeImageID]->fileName+"-"+std::to_string(++sliceID) + ".bmp");
            this->popUpDialogs.emplace_back("Saved Slice \"" + this->imageData[this->activeImageID]->fileName + "-" + std::to_string(sliceID) + ".bmp" + "\" (" + std::to_string(img.getSize().x) + "x" + std::to_string(img.getSize().y) + ")");
        }
    }

    if (this->cutSlices)
    {
        this->cutSlices = false;
        this->ShowPopUP("Slicing information");
    }

    this->UpdateSlicerVisualSelected();
}

void UI::ClickSelectPaletteColour(const sf::Color& colour)
{
    for (auto& pal : this->paletteColours)
    {
        if (pal.getFillColor() == colour)
        {
            pal.setOutlineThickness(1.5f);
            sf::Color px = pal.getFillColor();
            // Set the target color
            this->colourToReplace[0] = static_cast<float>(px.r) / 255.0f;
            this->colourToReplace[1] = static_cast<float>(px.g) / 255.0f;
            this->colourToReplace[2] = static_cast<float>(px.b) / 255.0f;
        }
        else
            pal.setOutlineThickness(0.f);
    }
}

void UI::Tick(sf::RenderWindow& window)
{

    if (!this->imageData.empty())
    {

        // DrawImages

        if (this->imageSprite.getTextureRect().width > 0)
        {
            window.draw(imageSprite);
        }

        // Image overlay info
        // Create a custom overlay window
        ImGui::SetNextWindowPos(ImVec2(0, 20)); // Position the overlay below the menu bar
        ImGui::SetNextWindowSize(ImVec2(475, 53)); // Set the size of the overlay

        ImGui::Begin("sliderOverlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);

        if (ImGui::SliderInt("Active Frame", &this->activeImageID, 0, this->imageData.size() - 1))
        {
            this->imageSprite.setTexture(this->imageData[this->activeImageID]->texture, true);

            this->CreateImagePalette();
            if (this->functionType == FUNCTION_TYPE::SLICER)
                this->CreateSlicerBoxes();

        }
        if (this->functionType == FUNCTION_TYPE::RECOLOUR)
        {
            if (ImGui::SliderFloat("Image Scale", &this->imageScale, MIN_SCALE, MAX_SCALE))
            {
                this->imageSprite.setScale(sf::Vector2f(this->imageScale, this->imageScale));
                this->delayDraw = 0;
            }
            ImGui::SameLine();
            if (ImGui::InputFloat("Scale", &this->imageScale))
            {
                if (this->imageScale < MIN_SCALE) this->imageScale = MIN_SCALE;
                else if (this->imageScale > MAX_SCALE) this->imageScale = MAX_SCALE;
                this->imageSprite.setScale(sf::Vector2f(this->imageScale, this->imageScale));

                // remove?
                this->delayDraw = 2;
            }
            ImGui::End();

            // Second overlay window
            ImGui::SetNextWindowPos(ImVec2(480, 20)); // Adjust the position to place it next to the first window
            ImGui::SetNextWindowSize(ImVec2(315, 53)); // Match the size (or use a different size if needed)

            ImGui::Begin("ColourChange", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::SameLine();
            ImGui::Text("           ");
            ImGui::SameLine();
            if (ImGui::Button("Change Colour"))
            {
                sf::Color colourToReplaceColor(
                    static_cast<sf::Uint8>(this->colourToReplace[0] * 255),
                    static_cast<sf::Uint8>(this->colourToReplace[1] * 255),
                    static_cast<sf::Uint8>(this->colourToReplace[2] * 255)
                );

                sf::Color newColourColor(
                    static_cast<sf::Uint8>(this->newColour[0] * 255),
                    static_cast<sf::Uint8>(this->newColour[1] * 255),
                    static_cast<sf::Uint8>(this->newColour[2] * 255)
                );
                this->UpdateColourChanges(colourToReplaceColor, newColourColor);
            }
            ImageData* imgData = this->imageData[this->activeImageID].get();
            std::string infoLine = imgData->fileName + "  - " + std::to_string(imgData->img.getSize().x) + "x" + std::to_string(imgData->img.getSize().y);
            ImGui::Text(infoLine.c_str());
            ImGui::End();

            // Second overlay window
            ImGui::SetNextWindowPos(ImVec2(WINDOW_WIDTH-480, 20)); // Adjust the position to place it next to the first window
            ImGui::SetNextWindowSize(ImVec2(475, 53)); // Match the size (or use a different size if needed)

            ImGui::Begin("ColourPicker", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            if (ImGui::ColorEdit3("Colour to replace", this->colourToReplace, ImGuiColorEditFlags_NoOptions))
            {
                this->delayDraw = 2;
                this->delayDraw = 0;
            }
            if (ImGui::ColorEdit3("New colour", this->newColour))
            {
                this->delayDraw = 2;
                this->delayDraw = 0;
            }

            if (this->imageSprite.getTextureRect().width > 0)
            {
                for (auto& pal : this->paletteColours)
                    window.draw(pal);
            }
        }
        else if (this->functionType == FUNCTION_TYPE::SLICER)
        {
            if (ImGui::SliderInt("Slicer Width", &this->slicerWidth, MIN_SLICE_WIDTH, MAX_SLICE_WIDTH))
            {
                this->CreateSlicerBoxes();
                this->delayDraw = 2;
            }
            ImGui::SameLine();
            if (ImGui::InputInt("SliceWidth", &this->slicerWidth))
            {
                if (this->slicerWidth < MIN_SLICE_WIDTH) this->slicerWidth = MIN_SLICE_WIDTH;
                else if (this->slicerWidth > MAX_SLICE_WIDTH) this->slicerWidth = MAX_SLICE_WIDTH;
                
                this->CreateSlicerBoxes();

                // remove?
                this->delayDraw = 2;
            }
            ImGui::End();
            // Second overlay window
            ImGui::SetNextWindowPos(ImVec2(480, 20)); // Adjust the position to place it next to the first window
            ImGui::SetNextWindowSize(ImVec2(475, 53)); // Match the size (or use a different size if needed)

            bool updateActiveSliceVisual = false;

            ImGui::Begin("Slice Options", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::Text("Slice Settings");


            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            if (ImGui::Button("Cut Slices"))
            {
                this->cutSlices = true;
                this->CreateSlicerBoxes();
            }
            ImGui::SameLine();
            ImGui::Text(" |         ");
            ImGui::SameLine();
            ImGui::Text("Slice Height: ");
            ImGui::SameLine();
            int sliceHeight = this->slicerBoxes[this->activeSliceID].getSize().y;
            if (ImGui::InputInt("Slice Height", &sliceHeight))
            {

                this->slicerBoxes[this->activeSliceID].setSize(sf::Vector2f(this->slicerBoxes[this->activeSliceID].getSize().x, sliceHeight));
                updateActiveSliceVisual = true;
                this->delayDraw = 2;
            }


            if (ImGui::SliderInt("Select Slice", &this->activeSliceID, 0, this->slicerBoxes.size() - 1))
                updateActiveSliceVisual = true;

            ImGui::SameLine();

            if (ImGui::InputInt("SliceInput", &this->activeSliceID))
                updateActiveSliceVisual = true;

            this->activeSliceID = std::min(this->activeSliceID, static_cast<int>(this->slicerBoxes.size()-1));
            this->activeSliceID = std::max(0, this->activeSliceID);

            if (updateActiveSliceVisual)
            {
                this->UpdateSlicerVisualSelected();
            }

            for (auto& sliceBox : this->slicerBoxes)
                window.draw(sliceBox);

            ImGui::End();
            // Second overlay window
            ImGui::SetNextWindowPos(ImVec2(960, 20)); // Adjust the position to place it next to the first window
            ImGui::SetNextWindowSize(ImVec2(320, 53)); // Match the size (or use a different size if needed)

            ImGui::Begin("Image Info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            
            ImageData* imgDat = this->imageData[this->activeImageID].get();

            std::string infoLine = "Image Info: " + imgDat->fileName;
            ImGui::Text(infoLine.c_str());

            infoLine = "Dimensions: (" + std::to_string(imgDat->img.getSize().x) + "x" + std::to_string(imgDat->img.getSize().y)+")";
            ImGui::Text(infoLine.c_str());
        }
        ImGui::End();

    }
   








    // Show dialog
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlg"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
            // Do something with the selected file
        }

        if (ImGuiFileDialog::Instance()->IsOk())
        {
            // Get selected files as a map (filename -> full path)
            auto selectedFiles = ImGuiFileDialog::Instance()->GetSelection();

            // change this.. doesn't even need a vector
            std::vector<std::string> filesToLoad;
            for (const auto& [filename, filepath] : selectedFiles)
            {
                filesToLoad.emplace_back(filename);
            }
            if (!filesToLoad.empty())
                this->LoadImages(filesToLoad);
        }
        this->delayDraw = 2;
        ImGuiFileDialog::Instance()->Close();
    }



    // Draw ImGui Menu
    this->DrawImGuiMenu();
}

void UI::Click(sf::Vector2i pos, bool rightClick)
{
    if (!this->imageData.empty())
    {
        for (auto& pal : this->paletteColours)
        {
            if (pal.getGlobalBounds().contains(sf::Vector2f(pos)))
            {
                if (rightClick)
                {
                    this->newColour[0] == static_cast<float>(pal.getFillColor().r) / 255.0f;
                    this->newColour[1] == static_cast<float>(pal.getFillColor().g) / 255.0f;
                    this->newColour[2] == static_cast<float>(pal.getFillColor().b) / 255.0f;
                }
                else
                    this->ClickSelectPaletteColour(pal.getFillColor());
            }
        }

        if (this->imageSprite.getGlobalBounds().contains(sf::Vector2f(pos)))
        {
            sf::Vector2f localPos = this->imageSprite.getInverseTransform().transformPoint(sf::Vector2f(pos));

            sf::Color px = this->imageData[this->activeImageID]->img.getPixel(static_cast<unsigned int>(localPos.x), static_cast<unsigned int>(localPos.y));

            if (rightClick)
            {
                this->newColour[0] = static_cast<float>(px.r) / 255.0f;
                this->newColour[1] = static_cast<float>(px.g) / 255.0f;
                this->newColour[2] = static_cast<float>(px.b) / 255.0f;
            }
            else
            {
                this->colourToReplace[0] = static_cast<float>(px.r) / 255.0f;
                this->colourToReplace[1] = static_cast<float>(px.g) / 255.0f;
                this->colourToReplace[2] = static_cast<float>(px.b) / 255.0f;
            }
        }
    }
}

void UI::UpdateSlicerVisualSelected()
{
    for (auto& sliceBox : this->slicerBoxes)
        sliceBox.setOutlineColor(sf::Color::Red);

    this->slicerBoxes[this->activeSliceID].setOutlineColor(sf::Color::Green);
}

void UI::UpdateColourChanges(sf::Color colourToReplace, sf::Color newColour)
{
    if (this->imageData.empty()) return;

    std::unique_ptr<ColourChanges> change = std::make_unique<ColourChanges>();

    for (sf::Uint16 i = 0; i < this->imageData.size(); i++)
    {
        sf::Image& img = this->imageData[i]->img;

        for (int x = 0; x < img.getSize().x; x++)
        {
            for (int y = 0; y < img.getSize().y; y++)
            {
                sf::Color pxColour = img.getPixel(x, y);
                if (pxColour == colourToReplace)
                {
                    img.setPixel(x, y, newColour);
                    std::unique_ptr<ColourChangeData> changeData = std::make_unique<ColourChangeData>();
                    changeData->oldColour = pxColour;
                    changeData->x = x;
                    changeData->y = y;
                    changeData->imageIndex = i;
                    change->changeData.emplace_back(std::move(changeData));
                }
            }
        }
        this->imageData[i]->texture.loadFromImage(img);
    }

    change->newColour = newColour;
    this->colourChanges.emplace_back(std::move(change));

    this->imageSprite.setTexture(this->imageData[this->activeImageID]->texture, true);
    this->CreateImagePalette();
    this->delayDraw = 0;
}

void UI::Undo()
{
    if (this->colourChanges.empty()) return;

    ColourChanges* change = this->colourChanges[this->colourChanges.size() - 1].get();

    for (auto& action : change->changeData)
    {
        this->imageData[action->imageIndex]->img.setPixel(action->x, action->y, action->oldColour);
    }

    for (auto& imgDat : this->imageData)
    {
        imgDat->texture.loadFromImage(imgDat->img);
    }
    this->colourChanges.pop_back();

    this->imageSprite.setTexture(this->imageData[this->activeImageID]->texture, true);

    this->CreateImagePalette();

    this->delayDraw = 0;
}

void UI::Save()
{
    if (this->imageData.empty()) return;
    if (this->colourChanges.empty())
    {
        this->ShowPopUP("Uhh Ohh..", { "You haven't changed anything yet." });
        return;
    }
    if (!std::filesystem::exists("recolours/"))
    {
        std::filesystem::create_directory("recolours/");
        this->popUpDialogs.emplace_back("Created recolours/");
    }

    for (auto& imgDat : this->imageData)
    {
        std::string name = "recolours/" + imgDat->fileName + "-recolour.bmp";
        if (imgDat->img.saveToFile(name))
        {
            this->popUpDialogs.emplace_back("Saved file " + name);
        }

        this->ShowPopUP("Images Saved!");
    }
}
