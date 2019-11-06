# NixieClockFW

Custom firmware for NCS314 **HW2.0**. The goal of this project is to make the firmware easy to read, and easy to customize.

Originally I wanted to modify the [original firmware of NCS314](https://github.com/afch/NixeTubesShieldNCS314), but I had hard time to understand the code. I've decided to scrap the idea of modifying the firmware, and decided to write my own.

# Required Libraries

* https://github.com/bhagman/Tone
* https://github.com/PaulStoffregen/Time

# Disclaimer

I cannot guarantee this will work with other hardware versions, and I would strongly advise you not to try. Using this firmware with other hardware versions(other than HW2.0) might cause damage to the shield or the arduino.

# Current Features

* **Timezones with DST:** DST can be set to None, EU, or US modes.
* **GPS synchronization:** Firmware is able to detect the baud rate by itself, should be compatible with most GPS modules using NMEA protocol over serial.
* **Anti cathode poisoning (ACD):** The ACD include various modes with customizable intervals)
* **Display Interface:** Provides easy control over the nixie tubes.
* **RTTL player:** The RTTL player itself is completed, but needs Alarm implementation before its usable.

# TODO

* Menu(Buttons + IR Remote)
* LED Control
* Temperature
* Alarm
