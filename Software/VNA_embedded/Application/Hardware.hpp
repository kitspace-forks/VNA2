#pragma once

#include <cstdint>
#include "Protocol.hpp"
#include "FPGA/FPGA.hpp"

#define USE_DEBUG_PINS

#ifdef USE_DEBUG_PINS
#define DEBUG1_GPIO		GPIOA
#define DEBUG1_PIN		GPIO_PIN_13
#define DEBUG2_GPIO		GPIOA
#define DEBUG2_PIN		GPIO_PIN_14

#define DEBUG1_LOW() do {DEBUG1_GPIO->BSRR = DEBUG1_PIN<<16; }while(0)
#define DEBUG1_HIGH() do {DEBUG1_GPIO->BSRR = DEBUG1_PIN; }while(0)
#define DEBUG2_LOW() do {DEBUG2_GPIO->BSRR = DEBUG2_PIN<<16; }while(0)
#define DEBUG2_HIGH() do {DEBUG2_GPIO->BSRR = DEBUG2_PIN; }while(0)
#else
#define DEBUG1_LOW()
#define DEBUG1_HIGH()
#define DEBUG2_LOW()
#define DEBUG2_HIGH()
#endif

namespace HW {

static constexpr uint32_t ADCSamplerate = 800000;
static constexpr uint32_t IF1 = 62000000;
static constexpr uint32_t IF2 = 250000;
static constexpr uint32_t LO1_minFreq = 25000000;
static constexpr uint32_t MaxSamples = 130944;
static constexpr uint32_t MinSamples = 16;
static constexpr uint32_t PLLRef = 100000000;

static constexpr uint8_t ADCprescaler = FPGA::Clockrate / ADCSamplerate;
static_assert(ADCprescaler * ADCSamplerate == FPGA::Clockrate, "ADCSamplerate can not be reached exactly");
static constexpr uint16_t DFTphaseInc = 4096 * IF2 / ADCSamplerate;
static_assert(DFTphaseInc * ADCSamplerate == 4096 * IF2, "DFT can not be computed for 2.IF");

static constexpr Protocol::DeviceInfo Info = {
		.ProtocolVersion = Protocol::Version,
		.FW_major = FW_MAJOR,
		.FW_minor = FW_MINOR,
		.FW_patch = FW_PATCH,
		.HW_Revision = HW_REVISION,
	    .extRefAvailable = 0,
	    .extRefInUse = 0,
	    .FPGA_configured = 0,
	    .source_locked = 0,
	    .LO1_locked = 0,
	    .ADC_overload = 0,
		.temp_source = 0,
		.temp_LO1 = 0,
		.temp_MCU = 0,
		.limits_minFreq = 0,
		.limits_maxFreq = 6000000000,
		.limits_minIFBW = ADCSamplerate / MaxSamples,
		.limits_maxIFBW = ADCSamplerate / MinSamples,
		.limits_maxPoints = FPGA::MaxPoints,
		.limits_cdbm_min = -4000,
		.limits_cdbm_max = 0,
		.limits_minRBW = (uint32_t) (ADCSamplerate * 2.23f / MaxSamples),
		.limits_maxRBW = (uint32_t) (ADCSamplerate * 2.23f / MinSamples),
};

enum class Mode {
	Idle,
	Manual,
	VNA,
	SA,
};

bool Init();
void SetMode(Mode mode);
void SetIdle();
void Work();

bool GetTemps(uint8_t *source, uint8_t *lo);
void fillDeviceInfo(Protocol::DeviceInfo *info, bool updateEvenWhenBusy = false);
namespace Ref {
	bool available();
	// reference won't change until update is called
	void set(Protocol::ReferenceSettings s);
	void update();
}

}
