// ads1x15_volt_meter.cpp example

#include <Arduino.h>

//#define SERIAL_DEBUG_DISABLED

#define USE_LIB_WEBSOCKET true

#include "sensesp_app.h"
#include "sensors/ads1x15.h"
#include "signalk/signalk_output.h"
#include "transforms/ads1x15_voltage.h"
#include "transforms/linear.h"
#include "transforms/voltage_multiplier.h"

ReactESP app([]() {
#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  sensesp_app = new SensESPApp();

  // Create an instance of a ADS1115 Analog-to-Digital Converter (ADC).
  ADS1x15CHIP_t chip = ADS1115chip;
  adsGain_t gain = GAIN_TWOTHIRDS;
  ADS1115* ads1115 = new ADS1115(0x48, gain);

  // Create an instance of ADS1115Value to read a specific channel of the ADC,
  // the channel on the physical chip that the input is connected to.
  uint8_t channel_12V = 0;
  uint read_delay_12V = 500;

  auto* alt_12v_voltage = new ADS1115Value(ads1115, channel_12V, read_delay_12V,
                                           "/12V_Alt/ADC read delay");

  // The output from the ADS1115 needs to be sent through some transforms to get
  // it ready to display in Signal K:
  // - ADS1x15Voltage() takes the output from the ADS1115 and converts it back
  // into the voltage that was read by the chip.
  // - VoltageMultiplier() reverses the effect of the physical voltage divider
  // that was used to step the source voltage
  //   down to less than 5 volts, which is the maximum the ADC can read. The
  //   output is the original voltage.
  // - SKOutputNumber() is a specialized transport to send a float value to the
  // Signal K server.
  //
  // To find valid Signal K Paths that fits your need you look at this link:
  // https://signalk.org/specification/1.4.0/doc/vesselsBranch.html#vesselsregexpelectrical

  alt_12v_voltage->connect_to(new ADS1x15Voltage(chip, gain))
      ->connect_to(new VoltageMultiplier(
          10000, 4730,
          "/12V_Alt/VoltMultiplier"))  // Measured ohm values of R1 and R2 in the
                                      // physical voltage divider
      ->connect_to(new SKOutputNumber("electrical.alternators.12V.voltage",
                                      "/12V_Alt/skPath"));

  // Create a second instance of ADS1115Value to read from the same physical
  // ADS1115, but from channel 1 instead of 0.
  uint8_t channel_24V = 1;
  uint read_delay_24V = 500;

  auto* alt_24v_voltage = new ADS1115Value(ads1115, channel_24V, read_delay_24V,
                                           "/24V_Alt/ADC read delay");

  alt_24v_voltage->connect_to(new ADS1x15Voltage(chip, gain))
      ->connect_to(new VoltageMultiplier(21800, 4690, "/24V_Alt/VoltMultiplier"))
      ->connect_to(new SKOutputNumber("electrical.alternators.24V.voltage",
                                      "/24V_Alt/skPath"));

  sensesp_app->enable();
});
