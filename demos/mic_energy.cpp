/*
 * Copyright 2016 <Admobilize>
 * All rights reserved.
 */
#include <gflags/gflags.h>
#include <wiringPi.h>

#include <unistd.h>
#include <iostream>
#include <string>
#include <valarray>

#include "../cpp/driver/everloop.h"
#include "../cpp/driver/everloop_image.h"
#include "../cpp/driver/microphone_array.h"
#include "../cpp/driver/wishbone_bus.h"

DEFINE_bool(big_menu, true, "Include 'advanced' options in the menu listing");
DEFINE_int32(sampling_frequency, 16000, "Sampling Frequency");

namespace hal = matrix_hal;

int main(int argc, char* agrv[]) {
  google::ParseCommandLineFlags(&argc, &agrv, true);

  hal::WishboneBus bus;
  bus.SpiInit();

  hal::MicrophoneArray mics;
  mics.Setup(&bus);

  hal::Everloop everloop;
  everloop.Setup(&bus);

  hal::EverloopImage image1d;
  int j = 0;
  uint64_t instantE = 0;
  uint64_t avgEnergy = 0;
  std::valarray<uint64_t> localAverage(20);
  localAverage = 0;

  int sampling_rate = FLAGS_sampling_frequency;
  mics.SetSamplingRate(sampling_rate);
  mics.ShowConfiguration();

  while (true) {
    mics.Read(); /* Reading 8-mics buffer from de FPGA */
    instantE = 0;
    for (uint32_t s = 0; s < mics.NumberOfSamples(); s++) {
      instantE = instantE + (mics.At(s, 0)) * (mics.At(s, 0));
    }

    localAverage[j % 20] = instantE;
    avgEnergy = 0;
    for (auto& data : localAverage) {
      avgEnergy = (avgEnergy + data);
    }

    avgEnergy = avgEnergy / 20;

    for (auto& led : image1d.leds) {
      led.red = avgEnergy >> 24;
    }
    everloop.Write(&image1d);

    j++;
  }

  return 0;
}
