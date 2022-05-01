# lora-receiver
Based on Jim Booth's Boat Monitor hardware and software. Thanks, Jim!

This software runs the "base station" of Jim's transmitter / receiver setup, where each thing that's being monitored (a boat, a truck, a swimming pool) has a transmitter that gathers the data and sends it out into the ether (that's how LoRa works), and a receiver that's always waiting for data from transmitters. Initially, each transmitter was fairly customized for what it was monitoring, and the receiver was VERY customized for what it's hoping to receive.

Version 2.0 (1 May, 2022) of the receiver is completely generic for the receipt, processing, and display of data from multiple transmitters - it doesn't know nor care what the data is nor where it comes from, as long as it's from a transmitter on the correct networkID and in the correct address range. For example, if you want to add a transmitter that monitors the water level and temperature in your swimming pool, all of that is done in the transmitter code, and nothing has to be done in the receiver code.

The only thing that's still transmitter-specific in v2.00 of the receiver is the data that's being sent to Jim's website - that is specific to the sensors that were in place when he gave me my own hardware to start playing with (3 batteries, and the data from the BME280 that's connected to the receiver hardware). The next major version will send all data to some IoT website where it will be easy to send and display new data from new transmitters, all completely generically.

![image](https://user-images.githubusercontent.com/15186790/166153259-cdd14a00-4b45-4179-99bb-5f8d250c6018.png)

Here's the receiver "under the hood", with the BME280 temp, pressure, and humidity sensor on the end of the 4-wire cable.
