


# ESP32 HUB75 LED Panel from AliExpress (Clockwise Clone)

> Reverse engineering an undocumented ESP32 + HUB75 LED clock purchased from AliExpress.

---
![esp32matrix1](https://github.com/user-attachments/assets/9122d3a7-0c69-4042-b326-7ec08ad73e53)  
![esp32matrix2](https://github.com/user-attachments/assets/a6f5e992-4ea1-4b2f-8697-119879209f23)
## About

This repository documents the reverse engineering process of a cheap 64×64 HUB75 RGB LED panel sold on AliExpress as a fully assembled digital clock inspired by the **Clockwise** project.

Unfortunately, the original AliExpress listing no longer exists, and the manufacturer never released any documentation, schematics, source code, or firmware.

After flashing custom firmware, the original factory firmware was lost forever because no backup had been made.

Rather than giving up, this repository became an attempt to understand the hardware and make it usable again.

Hopefully it will save someone else many hours of frustration.

---

# Hardware

The unit contains:

- ESP32-WROOM-32
- 64×64 HUB75 RGB LED Matrix
- FM6126A compatible panel
- Built-in switching power supply
- RTC battery
- Large electrolytic capacitor
- Custom PCB

Unlike many HUB75 panels sold individually, this one comes already assembled inside a finished enclosure.

---

# GPIO Mapping

```text
R1  = GPIO 25
G1  = GPIO 26
B1  = GPIO 27

R2  = GPIO 14
G2  = GPIO 12
B2  = GPIO 13

A   = GPIO 23
B   = GPIO 19
C   = GPIO 5
D   = GPIO 17
E   = GPIO 32

LAT = GPIO 4
OE  = GPIO 15
CLK = GPIO 16
```

---

# The Problem

Initially everything appeared to be working.

Simple tests looked perfect:

- solid colors
- refresh
- brightness
- addressing

However, as soon as real graphics were drawn, everything fell apart.

Typical symptoms included:

- sprites becoming distorted
- graphics breaking when crossing the middle of the screen
- random colored rectangles
- corrupted lower half
- incorrect colors
- strange artifacts while objects moved

Curiously, filling the entire display with a single color worked perfectly.

This suggested that the hardware itself was probably not damaged.

---

# Investigation

During the debugging process, many different possibilities were tested.

## Hardware

- damaged HUB75 panel
- broken LEDs
- power supply
- loose connections
- PCB damage

---

## Software

Several driver configurations were tested.

Including:

- FM6126A
- SHIFTREG
- SM5266P
- multiple CLKPHASE values
- latch blanking values
- DMA double buffer on/off
- brightness
- refresh timing
- scan timing

---

## RGB permutations

Different channel mappings were tested:

- RGB
- RBG
- GRB
- GBR
- BRG
- BGR

Some combinations produced better colors than others but none solved the corruption problem.

---

## Library versions

This turned out to be the most important discovery.

Different versions of:

- ESP32 Arduino Core
- ESP32 HUB75 DMA library

produce completely different results on this particular hardware.

Some versions compile correctly but display corrupted graphics.

Others produce stable output.

The panel is much more sensitive to software versions than expected.

---

# Current Status

The display is now stable.

Current achievements:

- Stable image
- No random rectangles
- No half-screen corruption
- Stable refresh
- Correct sprite rendering
- Correct addressing
- Smooth animation

Remaining work:

- Finish documenting the hardware
- Verify exact RGB channel mapping
- Compare behavior with the original Clockwise firmware
- Improve examples

---

# Lessons Learned

If you own one of these clocks:

## BACK UP THE ORIGINAL FIRMWARE FIRST.

Seriously.

Do it before changing anything.

Once erased, there is currently no public copy of the factory firmware.

Many inexpensive Chinese products are shipped with multiple undocumented hardware revisions under exactly the same product listing.

Two visually identical clocks may require different drivers or timing parameters.

Never assume that someone else's configuration will work on your unit.

---

# Known Working Environment

This repository currently uses:

| Component | Version |
|-----------|----------|
| ESP32 Arduino Core | 2.0.17 |
| HUB75 DMA Library | Version included in this repository |
| Driver | FM6126A |
| Panel | 64×64 HUB75 |
| MCU | ESP32-WROOM-32 |

Using newer versions may require additional changes.

---

# Why this repository exists

The goal is not simply to display graphics.

The goal is to document the entire reverse engineering process so anyone who buys one of these mysterious AliExpress clocks has a starting point instead of beginning from zero.

If this repository saves someone an entire weekend of trial and error, then it has already accomplished its purpose.

---

# Repository Goals

- Document the hardware
- Document the PCB
- Document the GPIO mapping
- Create working examples
- Understand the HUB75 timing
- Restore as much original functionality as possible
- Make the hardware useful again

---

# Contributing

If you own the same hardware, contributions are welcome.

Useful information includes:

- original firmware backups
- PCB photos
- hardware revisions
- panel driver identification
- oscilloscope captures
- timing measurements
- successful configurations

Every new piece of information helps build a better understanding of this undocumented hardware.

---

# A Final Warning

If you found this repository **before** flashing your clock...

**Stop.**

Make a complete backup of the ESP32 flash first.

Future-you will be extremely grateful.

---

# License

MIT License

---

# Acknowledgements

Thanks to:

- The Clockwise project for inspiring these devices.
- The ESP32 community.
- Everyone sharing information about undocumented HUB75 panels.

This repository focuses specifically on the reverse engineering of the undocumented AliExpress clone hardware, which differs from the official Clockwise project in several important ways.




