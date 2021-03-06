#include "Generator.hpp"
#include "Manual.hpp"
#include "Hardware.hpp"
#include "max2871.hpp"
#include "Si5351C.hpp"

static constexpr uint32_t BandSwitchFrequency = 25000000;

void Generator::Setup(Protocol::GeneratorSettings g) {
	if(g.activePort == 0) {
			// both ports disabled, no need to configure PLLs
			HW::SetMode(HW::Mode::Idle);
			return;
	}
	Protocol::ManualControl m;
	// LOs not required
	m.LO1CE = 0;
	m.LO1Frequency = 1000000000;
	m.LO1RFEN = 0;
	m.LO1RFEN = 0;
	m.LO2EN = 0;
	m.LO2Frequency = 60000000;
	m.Port1EN = 0;
	m.Port2EN = 0;
	m.RefEN = 0;
	m.Samples = 131072;
	m.WindowType = (int) FPGA::Window::None;
	// Select correct source
	if(g.frequency < BandSwitchFrequency) {
		m.SourceLowEN = 1;
		m.SourceLowFrequency = g.frequency;
		m.SourceHighCE = 0;
		m.SourceHighRFEN = 0;
		m.SourceHighFrequency = BandSwitchFrequency;
		m.SourceHighLowpass = (int) FPGA::LowpassFilter::M947;
		m.SourceHighPower = (int) MAX2871::Power::n4dbm;
		m.SourceHighband = false;
	} else {
		m.SourceLowEN = 0;
		m.SourceLowFrequency = BandSwitchFrequency;
		m.SourceHighCE = 1;
		m.SourceHighRFEN = 1;
		m.SourceHighFrequency = g.frequency;
		if(g.frequency < 900000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M947;
		} else if(g.frequency < 1800000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M1880;
		} else if(g.frequency < 3500000000UL) {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::M3500;
		} else {
			m.SourceHighLowpass = (int) FPGA::LowpassFilter::None;
		}
		m.SourceHighband = true;
	}
	switch(g.activePort) {
	case 1:
		m.AmplifierEN = 1;
		m.PortSwitch = 0;
		break;
	case 2:
		m.AmplifierEN = 1;
		m.PortSwitch = 1;
		break;
	}
	// Set level (not very accurate)
	if(g.cdbm_level > -1000) {
		// use higher source power (approx 0dbm with no attenuation)
		m.SourceHighPower = (int) MAX2871::Power::p5dbm;
		m.SourceLowPower = (int) Si5351C::DriveStrength::mA8;
	} else {
		// use lower source power (approx -10dbm with no attenuation)
		m.SourceHighPower = (int) MAX2871::Power::n4dbm;
		m.SourceLowPower = (int) Si5351C::DriveStrength::mA4;
		g.cdbm_level += 1000;
	}
	// calculate required attenuation
	uint16_t attval = -g.cdbm_level / 25;
	if(attval > 127) {
		attval = 127;
	}
	m.attenuator = attval;
	Manual::Setup(m);
}
