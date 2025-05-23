#include "../include/performancemonitor.h"

// local pointers
adlx::IADLXSystemMetricsPtr systemMetrics;
adlx::IADLXGPUMetricsPtr gpuMetrics;
adlx::IADLXGPUMetricsSupportPtr gpuMetricsSupport;
adlx::IADLXSystemMetricsSupportPtr systemMetricsSupport;
adlx::IADLXAllMetricsPtr allMetrics;
adlx::IADLXPerformanceMonitoringServicesPtr perfMonitoringService;
adlx::IADLXGPUPtr oneGPU;

ADLXHelper helper;

// function to intialize ADLX helper
void initializeHelper() {
	ADLX_RESULT res = ADLX_FAIL;

	// initialize ADLX
	res = helper.Initialize();

	if (ADLX_SUCCEEDED(res))
	{
		//std::cout << "ADLX init successful." << std::endl;
	}
	else
		std::cout << "ADLX init failed." << std::endl;

}

// function to terminate the helper
void terminateHelper() {
	ADLX_RESULT res = ADLX_FAIL;

	// destroy ADLX
	res = helper.Terminate();
	//std::cout << "Destroy ADLX result: " << res << std::endl;
}

// function to init and setup adlx services for getting performance metrics
void setupServices() {

	// get performance monitoring services
	ADLX_RESULT res = helper.GetSystemServices()->GetPerformanceMonitoringServices(&perfMonitoringService);
	if (ADLX_SUCCEEDED(res))
	{
		adlx::IADLXGPUListPtr gpus;
		// get GPU list
		res = helper.GetSystemServices()->GetGPUs(&gpus);
		if (ADLX_SUCCEEDED(res))
		{
			// use the first GPU in the list
			res = gpus->At(gpus->Begin(), &oneGPU);
			if (ADLX_SUCCEEDED(res))
			{
				//std::cout << "Get particular GPU successful." << std::endl;
			}
			else
				std::cout << "Get particular GPU failed." << std::endl;
		}
		else
			std::cout << "Get GPU list failed." << std::endl;
	}
	else
		std::cout << "Get performance monitoring services failed." << std::endl;

	// get system metrics support
	res = perfMonitoringService->GetSupportedSystemMetrics(&systemMetricsSupport);
	if (ADLX_SUCCEEDED(res)) {
		//std::cout << "CPU/System metrics supported." << std::endl;
	}
	else {
		std::cout << "CPU/System metrics not supported." << std::endl;
	}

	// get GPU metrics support
	res = perfMonitoringService->GetSupportedGPUMetrics(oneGPU, &gpuMetricsSupport);
	if (ADLX_SUCCEEDED(res)) {
		//std::cout << "GPU metrics supported." << std::endl;
	}
	else {
		std::cout << "GPU metrics not supported." << std::endl;
	}

	// get current all metrics
	ADLX_RESULT res1 = perfMonitoringService->GetCurrentAllMetrics(&allMetrics);
	if (ADLX_SUCCEEDED(res1))
	{
		// get current GPU metrics
		res1 = allMetrics->GetGPUMetrics(oneGPU, &gpuMetrics);
		if (ADLX_SUCCEEDED(res) && ADLX_SUCCEEDED(res1))
		{
			//std::cout << "GPU metrics ready to be called." << std::endl;
		}
		else {
			std::cout << "ERROR: GPU metrics cannot be called." << std::endl;
		}

		// get current CPU/system metrics
		ADLX_RESULT res1 = allMetrics->GetSystemMetrics(&systemMetrics);
		if (ADLX_SUCCEEDED(res1))
		{
			//std::cout << "CPU/System metrics ready to be called." << std::endl;
		}
		else {
			std::cout << "ERROR: CPU/System metrics cannot be called." << std::endl;
		}
	}
}

// function to release all pointers and terminate adlx helper object
void releaseAndTerminate() {
	// release pointers before terminating
	perfMonitoringService = nullptr;
	oneGPU = nullptr;
	allMetrics = nullptr;
	systemMetrics = nullptr;
	systemMetricsSupport = nullptr;
	gpuMetrics = nullptr;
	gpuMetricsSupport = nullptr;

	terminateHelper();
}

// get GPU usage (in %)
std::optional<adlx_double> getGPUUsage()
{
	adlx_bool supported = false;
	// get GPU usage if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUUsage(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_double usage = 0;
			res = gpuMetrics->GPUUsage(&usage);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU usage." << std::endl;
				return std::ceil(usage);
			}
			else
				std::cout << "Failure: could not fetch GPU usage." << std::endl;
		}
		else {
			std::cout << "GPU usage is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU temperature (degrees C)
std::optional<adlx_double> getGPUTemperature()
{
	adlx_bool supported = false;
	// get the GPU temperature if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUTemperature(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_double temperature = 0;
			res = gpuMetrics->GPUTemperature(&temperature);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU temperature." << temperature << std::endl;
				return temperature;
			}
			else
				std::cout << "Failure: could not fetch GPU temperature." << std::endl;
		}
		else {
			std::cout << "GPU temperature is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU hotspot temperature (degrees C)
std::optional<adlx_double> getGPUHotspotTemperature()
{
	adlx_bool supported = false;
	// get the GPU hotspot temperature if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUHotspotTemperature(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_double hotspotTemperature = 0;
			res = gpuMetrics->GPUHotspotTemperature(&hotspotTemperature);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU hotspot temperature." << std::endl;
				return hotspotTemperature;
			}
			else
				std::cout << "Failure: could not fetch GPU hotspot temperature." << std::endl;
		}
		else {
			std::cout << "GPU hotspot temperature is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU power (W)
std::optional<adlx_double> getGPUPower()
{
	adlx_bool supported = false;
	// get GPU power if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUPower(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_double power = 0;
			res = gpuMetrics->GPUPower(&power);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU power." << std::endl;
				return power;
			}
			else
				std::cout << "Failure: could not fetch GPU power." << std::endl;
		}
		else {
			std::cout << "GPU power is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU voltage (V)
std::optional<adlx_double> getGPUVoltage() {
	adlx_bool supported = false;
	// get GPU voltage if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUVoltage(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int voltage = 0;
			res = gpuMetrics->GPUVoltage(&voltage);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU volage." << std::endl;
				return static_cast<adlx_double>(voltage);
			}
			else
				std::cout << "Failure: could not fetch GPU voltage." << std::endl;
		}
		else {
			std::cout << "GPU voltage is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// display GPU clock speed (MHz)
std::optional<adlx_double> getGPUClockSpeed()
{
	adlx_bool supported = false;
	// display GPU clock speed if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUClockSpeed(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int gpuClock = 0;
			res = gpuMetrics->GPUClockSpeed(&gpuClock);
			if (ADLX_SUCCEEDED(res)) {
				return gpuClock;
			}
			else
				std::cout << "Failure: could not fetch GPU clock speed." << std::endl;
		}
		else {
			std::cout << "GPU clock speed is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU fan speed (RPM)
std::optional<adlx_double> getGPUFanSpeed()
{
	adlx_bool supported = false;
	// get the GPU fan speed if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUFanSpeed(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int fanSpeed = 0;
			res = gpuMetrics->GPUFanSpeed(&fanSpeed);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU fan speed." << std::endl;
				return fanSpeed;
			}
			else
				std::cout << "Failure: could not fetch GPU fan speed." << std::endl;
		}
		else {
			std::cout << "GPU fan speed is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU VRAM (MB)
std::optional<adlx_double> getGPUVRAM()
{
	adlx_bool supported = false;
	// get the GPU VRAM if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUVRAM(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int VRAM = 0;
			res = gpuMetrics->GPUVRAM(&VRAM);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU VRAM." << std::endl;
				return VRAM;
			}
			else
				std::cout << "Failure: could not fetch GPU VRAM." << std::endl;
		}
		else {
			std::cout << "GPU VRAM is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get GPU VRAM clock speed (MHz)
std::optional<adlx_double> getGPUVRAMClockSpeed()
{
	adlx_bool supported = false;
	// get the GPU VRAM clock speed if supported
	ADLX_RESULT res = gpuMetricsSupport->IsSupportedGPUVRAMClockSpeed(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int memoryClock = 0;
			res = gpuMetrics->GPUVRAMClockSpeed(&memoryClock);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched GPU VRAM clock speed." << std::endl;
				return memoryClock;
			}
			else
				std::cout << "Failure: could not fetch GPU VRAM clock speed." << std::endl;
		}
		else {
			std::cout << "GPU VRAM clock speed is not supported." << std::endl;
		}
	}
	return std::nullopt;
}

// get CPU usage (%)
std::optional<adlx_double> getCPUUsage()
{
	adlx_bool supported = false;
	// get CPU usage if supported
	ADLX_RESULT res = systemMetricsSupport->IsSupportedCPUUsage(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_double cpuUsage = 0;
			res = systemMetrics->CPUUsage(&cpuUsage);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched CPU usage." << std::endl;
				return cpuUsage;
			}
			else
				std::cout << "Failure: could not fetch CPU usage." << std::endl;
		}
	}
	return std::nullopt;
}

// get system RAM (MB)
std::optional<adlx_double> getSystemRAM()
{
	adlx_bool supported = false;
	// get system RAM if supported
	ADLX_RESULT res = systemMetricsSupport->IsSupportedSystemRAM(&supported);
	if (ADLX_SUCCEEDED(res))
	{
		if (supported)
		{
			adlx_int systemRAM = 0;
			res = systemMetrics->SystemRAM(&systemRAM);
			if (ADLX_SUCCEEDED(res)) {
				//std::cout << "Success: fetched system RAM." << std::endl;
				return systemRAM;
			}
			else
				std::cout << "Failure: could not fetch system RAM." << std::endl;
		}
	}
	return std::nullopt;
}