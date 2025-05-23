#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include "../include/ADLXHelper.h"
#include "IPerformanceMonitoring.h"
#include "IPerformanceMonitoring2.h"
#include <iostream>
#include <optional>

// functions to initialize/terminate the helper object
void initializeHelper();
void terminateHelper();

// function to get metrics
void setupServices();
void releaseAndTerminate();

// helper functions for showing each individual metric
std::optional<adlx_double> getGPUUsage();
std::optional<adlx_double> getGPUTemperature();
std::optional<adlx_double> getGPUHotspotTemperature();
std::optional<adlx_double> getGPUPower();
std::optional<adlx_double> getGPUVoltage();
std::optional<adlx_double> getGPUClockSpeed();
std::optional<adlx_double> getGPUFanSpeed();
std::optional<adlx_double> getGPUVRAM();
std::optional<adlx_double> getGPUVRAMClockSpeed();
std::optional<adlx_double> getCPUUsage();
std::optional<adlx_double> getSystemRAM();

#endif
