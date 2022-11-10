# battery-monitor (lora-receiver with a 128x128 OLED screen)
Based on Jim Booth's Boat Monitor hardware and software. Thanks, Jim!

This software runs the "base station" of Jim's transmitter / receiver setup, where each thing that's being monitored (a boat, a truck, a swimming pool) has a transmitter that gathers the data and sends it out into the ether (that's how LoRa works), and a receiver that's always waiting for data from transmitters. Initially, each transmitter was fairly customized for what it was monitoring, and the receiver was VERY customized for what it was hoping to receive.

Version 2.0 (1 May, 2022) of the receiver is completely generic in the receipt, processing, and display of data from multiple transmitters - it doesn't know nor care what the data is nor where it comes from, as long as it's from a transmitter on the correct networkID and in the correct address range. For example, if you want to add a transmitter that monitors the water level and temperature in your swimming pool, all of that is done in the transmitter code, and nothing has to be done in the receiver code.

Versions 2.1 and 2.2 set up "processing new incoming LoRa packets" and "sending new data to InfluxDB" with tasks and queues.

Version 2.3 (9 October, 2022) fixed a bug that sometimes resulted in an alarm email/text being sent every minute.

In October and November 2022, I finally got a 3D printer and started playing with it, and my first project is to add a larger OLED display to this project. It will involve a lot of coding changes, so I've mirrored the "lora-receiver" repository to this new repository, and this is where I'll do all of the work to utilize the new, bigger (128x128 vs. 128x64) display.
