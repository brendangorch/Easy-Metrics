#include <SFML/Graphics.hpp>
#include "../include/metricsoverlay.h"
#include <iostream>
#include <thread>
#include "imgui.h"
#include "imgui-SFML.h"
#include "../resource.h"

// get screen resolution
sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
sf::Vector2u screen = desktop.size;
unsigned int screenWidth = screen.x;
unsigned int screenHeight = screen.y;

unsigned int selectedOptionsBinary = 0;

// overlay selector base colors
float overlaySelectorColor[3] = { 0.0f, 0.0f, 0.0f };
float labelSelectorColor[3] = { 1.0f, 0.0f, 0.0f };
float valueSelectorColor[3] = { 0.0f, 1.0f, 0.0f };

// overlay stuff
static int overlayTextSize = 24;
static float overlayTransparency = 0.5f;

// base resolution and text size
const float baseResolutionY = 1080.0f;
const float baseResolutionX = 1920.0f;
int baseTextSize = 24;

// dynamically scaled font size based on screen height
float scaleFactorY = static_cast<float>(screenHeight) / baseResolutionY;
float scaleFactorX = static_cast<float>(screenWidth) / baseResolutionX;
int fontSize = static_cast<int>(baseTextSize * scaleFactorX);

// func declarations
void createMainWindow(sf::RenderWindow& window);
void setStyleAndColors(ImGuiStyle& style);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    // create a render window for the main window
    sf::RenderWindow window;
    createMainWindow(window);

    return 0;
}

// function to create main window
void createMainWindow(sf::RenderWindow& window) {
    // create the window relative to screen resolution
    sf::Vector2u windowSize(screenWidth / 1.4, screenHeight / 1.15);
    window.create(sf::VideoMode(windowSize), "Easy Metrics", sf::Style::Titlebar | sf::Style::Close);

    // set top-left icon and taskbar icon
    HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1));
    SetClassLongPtr(window.getNativeHandle(), GCLP_HICON, (LONG_PTR)hIcon);

    // initialize ImGui with a valid window
    ImGui::SFML::Init(window);

    // load the font
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryTTF(
        (void*)Inter_UI_Regular_otf,
        Inter_UI_Regular_otf_len,
        fontSize,                  
        &config
    );
    ImGui::SFML::UpdateFontTexture(); 
    
    // prevent saving into imgui.ini
    io.IniFilename = nullptr;

    // colors and styling
    ImGuiStyle& style = ImGui::GetStyle();
    setStyleAndColors(style);

    // set framerate
    window.setFramerateLimit(60);

    sf::Clock deltaClock;

    while (window.isOpen()) {
        // poll window events
        while (const auto event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);

            // window close
            if (event->is<sf::Event::Closed>()) {
                window.close();
                terminateOverlay = true;
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());


        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("MainUI", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings
        );

        // get window width for centering
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize("EasyMetrics").x;       

        // split drawlist into 4 channels
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->ChannelsSplit(4);

        // channel 1 (ui)
        drawList->ChannelsSetCurrent(1); 

        // centered title
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.0f));
        textWidth = ImGui::CalcTextSize("EasyMetrics").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("EasyMetrics");
        ImGui::PopStyleColor();

        // for background rect
        float backRectPaddingX = 50.f * scaleFactorX;
        ImVec2 backRectMin = ImVec2(backRectPaddingX, ImGui::GetCursorScreenPos().y);

        // centered label
        ImGui::Spacing();
        textWidth = ImGui::CalcTextSize("Select metrics for display:").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("Select metrics for display:");

        ImGui::Spacing();

        // bools for checkboxes
        static bool selectAll = false;
        static bool options[11] = {};
        // checkbox labels
        const char* optionLabels[11] = {
            "GPU Usage",
            "GPU Temperature",
            "GPU Hotspot Temperature",
            "GPU Power",
            "GPU Voltage",
            "GPU Clock Speed",
            "GPU Fan Speed",
            "GPU VRAM",
            "GPU VRAM Clock Speed",
            "CPU Usage",
            "System RAM"
        };

        // checkboxes disabled if overlay is running
        ImGui::BeginDisabled(isOverlayOpen);
        // individual checkboxes
        for (size_t i = 0; i < sizeof(options); i++) {
            // centering
            float optionWidth = ImGui::CalcTextSize(optionLabels[i]).x + ImGui::GetStyle().FramePadding.x * 4;
            ImGui::SetCursorPosX((windowWidth - optionWidth) * 0.5f);

            // store previous state to detect a transition from unchecked -> checked
            bool prevState = options[i];

            // checkbox clicked
            if (ImGui::Checkbox(optionLabels[i], &options[i])) {
                // sync "Select All" checkbox
                bool allSelected = true;
                for (bool val : options) {
                    if (!val) {
                        allSelected = false;
                        break;
                    }
                }
                selectAll = allSelected;

                // if checkbox was just checked
                if (!prevState && options[i]) {
                    // update the selected binary
                    selectedOptionsBinary += pow(2, i);
                    //std::cout << selectedOptionsBinary << std::endl;
                }

                // if checkbox was unchecked
                if (prevState && !options[i]) {
                    // update the selected binary
                    selectedOptionsBinary -= pow(2, i);
                    //std::cout << selectedOptionsBinary << std::endl;
                }
            }
        }

        // "Select All" checkbox
        textWidth = ImGui::CalcTextSize("Select All").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        if (ImGui::Checkbox("Select All", &selectAll)) {
            for (bool& opt : options)
                opt = selectAll;

            // override selectedOptionsBinary if selectAll is checked/unchecked
            if (!selectAll) {
                selectedOptionsBinary = 0;
                //std::cout << selectedOptionsBinary << std::endl;
            } else {
                selectedOptionsBinary = 0;
                for (size_t i = 0; i < sizeof(options); i++) {
                    selectedOptionsBinary += pow(2, i);
                }
                //std::cout << selectedOptionsBinary << std::endl;
            }
        }
        ImGui::EndDisabled();

        // center the buttons
        ImGui::Spacing();
        float buttonWidth = 200.0f * scaleFactorX;
        float totalButtonWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX((windowWidth - totalButtonWidth) * 0.5f);

        // disable display button if no options checked or overlay already exists
        ImGui::BeginDisabled(!selectedOptionsBinary || isOverlayOpen); 
        if (ImGui::Button("Display Overlay", ImVec2(buttonWidth, 0))) {
            // set selected metrics to display
            setSelectedMetrics(selectedOptionsBinary);
            // set overlay prefs
            setPreferences(overlaySelectorColor, labelSelectorColor, valueSelectorColor, overlayTransparency, overlayTextSize);
            // create the overlay on a new thread and run independently
            std::thread overlayThread(createOverlayWindow);
            overlayThread.detach();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        // disable terminate button if overlay is not already open
        ImGui::BeginDisabled(!isOverlayOpen);
        if (ImGui::Button("Terminate Overlay", ImVec2(buttonWidth, 0))) {
            // terminate the overlay window and thread
            terminateOverlay = true;
            // reset all checkboxes and binary
            for (size_t i = 0; i < sizeof(options); i++) {
                options[i] = false;
            }
            selectAll = false;
            selectedOptionsBinary = 0;
        }
        ImGui::EndDisabled();

        // channel 0 (background rect)
        drawList->ChannelsSetCurrent(0);

        // back rect
        ImVec2 backRectMax = ImVec2(windowWidth - backRectPaddingX, ImGui::GetCursorScreenPos().y + (10.f * scaleFactorY));
        ImGui::GetWindowDrawList()->AddRectFilled(
            backRectMin,
            backRectMax,
            IM_COL32(255, 255, 240, 255),
            16.f
        );

        #pragma region Overlay Preferences

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        // channel 3 (ui)
        drawList->ChannelsSetCurrent(3);
        
        // for background rect
        backRectPaddingX = 20.f * scaleFactorX;
        backRectMin = ImVec2(backRectPaddingX, ImGui::GetCursorScreenPos().y - (15.f * scaleFactorY));

        // title centering
        textWidth = ImGui::CalcTextSize("Overlay Preferences:").x;
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5);
        ImGui::Text("Overlay Preferences:");

        ImGui::Spacing();

        // center the color pickers with labels
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float offsetFromLeft = 50.f * scaleFactorX;

        // calculate the starting X position to center everything 
        ImGui::SetCursorPosX(offsetFromLeft);

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        // overlay Background Color Picker
        ImGui::ColorEdit3("Overlay Background", overlaySelectorColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs);
        ImVec2 colorEditSize1 = ImGui::GetItemRectSize();
        ImGui::SameLine();

        // metric Label Color Picker
        ImGui::ColorEdit3("Metric Label Color", labelSelectorColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs);
        ImVec2 colorEditSize2 = ImGui::GetItemRectSize();
        ImGui::SameLine();

        // metric Value Color Picker
        ImGui::ColorEdit3("Metric Value Color", valueSelectorColor, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoInputs);
        ImVec2 colorEditSize3 = ImGui::GetItemRectSize();

        ImGui::PopStyleColor();

        ImGui::Spacing();

        // calculate total width for all color pickers and their labels
        float totalWidth = colorEditSize1.x + colorEditSize2.x + colorEditSize3.x + (spacing * 2);

        float sliderSize = 200.f * scaleFactorX;
        ImGui::PushItemWidth(sliderSize);

        // set up slider labels
        textWidth = ImGui::CalcTextSize("Overlay Transparency").x;
        float dif = sliderSize - textWidth;
        ImGui::SetCursorPosX(offsetFromLeft + (dif / 2));
        ImGui::Text("Overlay Transparency");
        textWidth = ImGui::CalcTextSize("Text Size").x;
        dif = sliderSize - textWidth;
        ImGui::SameLine(totalWidth + offsetFromLeft - sliderSize + (dif / 2));
        ImGui::Text("Text Size");

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.584f, 0.70f, 0.72f, 1.0f));

        // transparency slider
        ImGui::SetCursorPosX(offsetFromLeft);
        ImGui::SliderFloat("##xx", &overlayTransparency, 0.0f, 1.0f, "%.2f");
        
        // text size slider
        ImGui::SameLine(totalWidth + offsetFromLeft - sliderSize);
        ImGui::SliderInt("##", &overlayTextSize, 10, 38);

        ImGui::PopItemWidth();
        ImGui::PopStyleColor();

        #pragma endregion

        #pragma region Overlay Preview

        ImGui::Spacing();        

        // apply font scaling and alpha blending
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, overlayTransparency);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::SetWindowFontScale(overlayTextSize / 18.0f);

        const char* label = "Example Metric:";
        const char* value = "99%";

        // measure both texts
        ImVec2 labelSize = ImGui::CalcTextSize(label);
        ImVec2 valueSize = ImGui::CalcTextSize(value);
        totalWidth = labelSize.x + 8 + valueSize.x;
        float textHeight = std::max(labelSize.y, valueSize.y);
        ImVec2 padding = ImVec2(10 * scaleFactorX, 6 * scaleFactorY);

        // compute rect for background
        float startX = windowWidth - (500 * scaleFactorX);
        float startY = ImGui::GetCursorScreenPos().y - (100 * scaleFactorY);
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 scroll = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());

        // get back rect max before rescaling of preview
        backRectMax = ImVec2(windowWidth - backRectPaddingX, ImGui::GetCursorScreenPos().y + (10.f * scaleFactorY));

        // rect position
        ImVec2 rectMin = ImVec2(
            windowPos.x + startX - padding.x - scroll.x,
            windowPos.y + startY - padding.y - scroll.y
        );
        ImVec2 rectMax = ImVec2(
            windowPos.x + startX + totalWidth + padding.x - scroll.x,
            windowPos.y + startY + textHeight + padding.y - scroll.y
        );

        // add rect to draw list
        drawList = ImGui::GetWindowDrawList();
        ImU32 bgCol = ImColor(overlaySelectorColor[0], overlaySelectorColor[1], overlaySelectorColor[2], overlayTransparency);
        drawList->AddRectFilled(rectMin, rectMax, bgCol, 6.0f);

        // draw text on same line
        ImGui::SetCursorPos(ImVec2(startX, startY));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(labelSelectorColor[0], labelSelectorColor[1], labelSelectorColor[2], 1.0f));
        ImGui::Text("%s", label);
        ImGui::SameLine();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(valueSelectorColor[0], valueSelectorColor[1], valueSelectorColor[2], 1.0f));
        ImGui::Text("%s", value);
        ImGui::PopStyleColor();

        // revert styles
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleVar(2);

        // channel 2 (background rect)
        drawList->ChannelsSetCurrent(2);

        // back rect
        ImGui::GetWindowDrawList()->AddRectFilled(
            backRectMin,
            backRectMax,
            IM_COL32(255, 255, 240, 255),
            16.f
        );

        // merge draw list channels
        drawList->ChannelsMerge();

        ImGui::End();

        // background color
        window.clear(sf::Color(18, 33, 43));
        ImGui::SFML::Render();
        window.display();
        
    }
    ImGui::SFML::Shutdown();
}

// setup for UI
void setStyleAndColors(ImGuiStyle& style) {
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.f, 0.f, 0.f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.439f, 0.149f, 0.196f, 1.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.251f, 0.263f, 0.306f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.569f, 0.184f, 0.251f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(1.f, 1.f, 1.f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.584f, 0.70f, 0.72f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.569f, 0.184f, 0.251f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(1.f, 1.f, 1.f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.f, 1.f, 0.9f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.439f, 0.149f, 0.196f, 0.80f);
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.FramePadding = ImVec2(10 * scaleFactorX, 6 * scaleFactorY);
    style.ItemSpacing = ImVec2(10 * scaleFactorX, 8* scaleFactorY);
}
