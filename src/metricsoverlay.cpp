#include "../include/metricsoverlay.h"
#include <functional>

// global vars
std::atomic<bool> isOverlayOpen = false;
std::atomic<bool> terminateOverlay = false;

// local vars
int numOfMetricsSelected = 0; // actual number of selected
unsigned int selectedMetrics = 0; // binary representation
sf::Color overlayColor;
sf::Color labelColor;
sf::Color valueColor;
int textSize;
int alpha;

// for metric updates
sf::Clock updateClock;
const sf::Time updateInterval = sf::seconds(1);

// struct to hold metric info
struct MetricInfo {
    int id;
    sf::String label;
    sf::String unit;
    std::function<std::optional<adlx_double>()> getValue;
};

// vector to hold metric IDs and functions
std::vector<MetricInfo> metrics = {
    {0, "GPU Usage", "%", getGPUUsage},
    {1, "GPU Temperature" , "°C", getGPUTemperature},
    {2, "GPU Hotspot Temperature", "°C", getGPUHotspotTemperature},
    {3, "GPU Power", " W", getGPUPower},
    {4, "GPU Voltage", " mV", getGPUVoltage},
    {5, "GPU Clock Speed", " MHz", getGPUClockSpeed},
    {6, "GPU Fan Speed", " RPM", getGPUFanSpeed},
    {7, "GPU VRAM", " MB", getGPUVRAM},
    {8, "GPU VRAM Clock Speed",  " MHz", getGPUVRAMClockSpeed},
    {9, "CPU Usage", "%", getCPUUsage},
    {10, "System RAM", " MB", getSystemRAM}
};

#pragma region Overlay Window Creation

// function to create the overlay window
void createOverlayWindow() {
    // set global bools
    isOverlayOpen = true;
    terminateOverlay = false;

    // get current screen resolution
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    unsigned int screenWidth = desktopMode.size.x;
    unsigned int screenHeight = desktopMode.size.y;

    // base resolution
    const float baseResolution = 1080.0f;

    // dynamically scaled font size based on screen height
    float scaleFactor = static_cast<float>(screenHeight) / baseResolution;
    int fontSize = static_cast<int>(textSize * scaleFactor); // 24 is base font size for 1080p

    // vertical space between lines
    int lineSpacing = static_cast<int>(fontSize * 0.4f);
    // height of each line of text
    int lineHeight = fontSize + lineSpacing;

    // margins
    const int marginTop = static_cast<int>(20 * scaleFactor);
    const int marginBottom = marginTop;
    const int marginLeft = static_cast<int>(20 * scaleFactor);
    const int marginRight = marginLeft;

    // load font
    sf::Font font;

    if (!font.openFromMemory(Inter_UI_Regular_otf, Inter_UI_Regular_otf_len)) {
        std::cerr << "Failed to load font" << std::endl;
    }   

    // compute height of window (margins + total vertical space needed for text lines)
    int windowHeight = marginTop + (lineHeight * numOfMetricsSelected) + marginBottom;

    // estimate window width based on longest possible string
    sf::Text sample(font, "GPU VRAM Clock Speed: 20000 MHz", fontSize);
    int windowWidth = static_cast<int>(sample.getLocalBounds().size.x + marginLeft + marginRight);

    // create window, set position and framerate
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), "Overlay", sf::Style::None);
    window.setPosition(sf::Vector2i(0, 0));
    window.setFramerateLimit(60);

    // hide overlay window from taskbar
    HWND hwnd = static_cast<HWND>(window.getNativeHandle());
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    exStyle |= WS_EX_TOOLWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // force Windows to reapply styles
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
        SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

    // make window on top layer of screen and transparent
    makeWindowAlwaysOnTopAndTransparent(window, 164);

    // create text lines
    initializeHelper(); // initialize adlx
    std::vector<sf::Text> metricLabels;
    std::vector<sf::Text> metricValues;
    drawLabels(metricLabels, font, fontSize, marginLeft, marginTop, lineHeight);
    drawValues(metricValues, font, fontSize, marginLeft, marginTop, lineHeight, windowWidth);

    while (window.isOpen())
    {
        // poll for exit window event
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) {
                releaseAndTerminate();
                window.close();
            } 
        }

        // if window is terminated from the main window
        if (terminateOverlay) {
            releaseAndTerminate();
            window.close();
        }

        // update metrics every second
        if (updateClock.getElapsedTime() >= updateInterval) {
            // clear the current lines
            metricValues.clear();

            // recall in case new fullscreen app was opened
            makeWindowAlwaysOnTopAndTransparent(window, alpha);
 
            drawValues(metricValues, font, fontSize, marginLeft, marginTop, lineHeight, windowWidth);
            updateClock.restart();
        }

        // draw here
        window.clear(overlayColor);
        for (const auto& t : metricLabels)
            window.draw(t);
        for (const auto& t : metricValues)
            window.draw(t);
        window.display();
    }

    releaseAndTerminate();
    isOverlayOpen = false;
}

// function to create vector of metric labels
void drawLabels(std::vector<sf::Text>& lines, sf::Font& font, int fontSize, const int marginLeft, const int marginTop, int lineHeight) {
    int verticalOffset = 0;

    // populate text vector
    for (size_t i = 0; i < metrics.size(); i++) {\
        // only use selected metrics
        if (selectedMetrics & (1 << i)) {
            sf::Text text(font);
            text.setCharacterSize(fontSize);
            text.setFillColor(labelColor);
            text.setPosition(sf::Vector2f(static_cast<float>(marginLeft), static_cast<float>(marginTop + verticalOffset * lineHeight)));
            text.setString(metrics[i].label + ": ");
            lines.push_back(text);
            verticalOffset++;
        }
    }    
}

// function to create the vector of metric values
void drawValues(std::vector<sf::Text>& lines, sf::Font& font, int fontSize, const int marginLeft, const int marginTop, int lineHeight, int windowWidth) {

    // setup adlx services 
    setupServices();

    int verticalOffset = 0;

    // populate text vector
    for (size_t i = 0; i < metrics.size(); i++) {
        if (selectedMetrics & (1 << i)) {
            sf::Text text(font);
            text.setCharacterSize(fontSize);
            text.setFillColor(valueColor);

            // get the width of the value text
            auto value = metrics[i].getValue();
            std::string valueStr = value.has_value() ? std::to_string(std::lround(value.value())) + metrics[i].unit : "N/A";
            text.setString(valueStr);
            float textWidth = text.getLocalBounds().size.x; 

            // position on the right side of the window
            float x = static_cast<float>(windowWidth - marginLeft - textWidth);

            // set position based on the margin and vertical offset
            float y = static_cast<float>(marginTop + verticalOffset * lineHeight);
            text.setPosition(sf::Vector2f(x, y));

            lines.push_back(text);
            verticalOffset++;
        }
    }
}

// function to make window always on top
void makeWindowAlwaysOnTopAndTransparent(sf::RenderWindow& window, int alpha) {
    HWND hwnd = window.getNativeHandle();

    // get existing style
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);

    // add needed styles: always on top, layered, transparent, no taskbar
    exStyle |= WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);

    // make the window layered with alpha transparency
    SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);

    // ensure it stays on top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

#pragma endregion

#pragma region Functions called from main

// function to set the selected metrics for display
void setSelectedMetrics (unsigned int selectedMetricsBinary) {
    selectedMetrics = selectedMetricsBinary;

    // reset number selected
    numOfMetricsSelected = 0;

    // determine number of selected metrics
    for (int i = 0; i < 11; i++) {
        if (selectedMetricsBinary & (1 << i)) {
            numOfMetricsSelected++;
        }
    }
}

// function for setting overlay prefs
void setPreferences(float overlayCol[3], float labelCol[3], float valueCol[3], float alph, int txtSize) {
    // set colors
    overlayColor.r = static_cast<int>(overlayCol[0] * 255);
    overlayColor.g = static_cast<int>(overlayCol[1] * 255);
    overlayColor.b = static_cast<int>(overlayCol[2] * 255);
    labelColor.r = static_cast<int>(labelCol[0] * 255);
    labelColor.g = static_cast<int>(labelCol[1] * 255);
    labelColor.b = static_cast<int>(labelCol[2] * 255);
    valueColor.r = static_cast<int>(valueCol[0] * 255);
    valueColor.g = static_cast<int>(valueCol[1] * 255);
    valueColor.b = static_cast<int>(valueCol[2] * 255);

    // set transparency and text size
    alpha = static_cast<int>(alph * 255 + 0.5f);
    textSize = txtSize;
}

#pragma endregion
