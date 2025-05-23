#ifndef METRICSOVERLAY_H
#define METRICSOVERLAY_H

#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "../include/performancemonitor.h"
#include "../include/inter.h"


// bools to keep track of overlay window status
extern std::atomic<bool> isOverlayOpen;
extern std::atomic<bool> terminateOverlay;

// function to create the overlay window
void createOverlayWindow();

void drawLabels(std::vector<sf::Text>& lines, sf::Font& font, int fontSize, const int marginLeft, const int marginTop, int lineHeight);
void drawValues(std::vector<sf::Text>& lines, sf::Font& font, int fontSize, const int marginLeft, const int marginTop, int lineHeight, int windowWidth);

// functions for overlay window properties
void makeWindowAlwaysOnTopAndTransparent(sf::RenderWindow& window, int alpha);\
void setPreferences(float overlayColor[3], float labelColor[3], float valueColor[3], float alpha, int textSize);

// functions for metrics
void setSelectedMetrics(unsigned int selectedMetricsBinary);


#endif