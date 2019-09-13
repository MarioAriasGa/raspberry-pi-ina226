# What it is

Use INA226 chip from Raspberry Pi to measure DC voltage, current and power consumption of a load.

# How to compile

Enable i2c using `raspi-config` or make sure that this line is uncommented in the `/boot/config.txt` and reboot if necessary.

```dtparam=i2c_arm=on```

```
$ sudo apt-get install wiringpi i2c-tools libi2c-dev
$ make
```

# How to connect.

Connect GND, SDA, SCL pins from the INA226 chip to the corresponding i2c Raspberry Pi pins. See https://pinout.xyz/

Connect Vcc to one of the 3.3v pins of the Raspberry (Note: the chip can handle 5V but the pull-ups would give the raspberry pi 5V which is not desirable, even through a resistor)

Connect the V+ pin to our Power supply (Make sure not to put more than 36V DC!! Never AC!)

Connect the V- pin to the + of the load to be measured.

Connect VBus to V- pin to measure voltage of the load.

Connect GND of the load to GND of the Pi (and the INA226 chip).

# How to configure

The INA226 chip needs a shunt resistor, a very small resistor that goes in series of the load between pins V+ and V-. Check where it is in your PCB and find the ohms from the number written (Tip: it is usually the largest size in volume and smallest in value) or measure the resistance between V+ and V- with a multimeter with everything else disconnected. In my case it was 0.1 Ohm.

The INA226 chip measures the voltage drop between V+ and V- to calculate the current by using Ohm's law.

Some parameters you may want to change in the code:
* I2C address of the chip, 0x40 by default. Use `i2cdetect -y 1` to find out.
* Shunt resistor value.
* Max expected current, to set the resolution of the ADC.
* Time measuring the Bus Voltage (Load) and Shunt Voltage ( Current ). More time is smoother and less is quicker.
* How many values to average between 1 and 1024 (use the constants, not numbers). More averages has less noise but takes longer.
* Sleep value in the for loop.

# Output

The output is continuous CSV with Voltage in V, Current in mA, Power in mW, Shunt Voltage in mV, Energy per year in kWh.

For example the "Hello World" of a led+resistor connected to USB cable:

```
date,time,timestamp,bus voltage(V),current (mA),power (mW),shunt voltage (mV),energy (kWh year)
2019-09-13,21:11:49,1568405509,5.024,19.906,100.003,1.990,0.877
2019-09-13,21:11:50,1568405510,5.024,19.937,100.156,1.992,0.878
2019-09-13,21:11:51,1568405511,5.025,19.937,100.181,1.992,0.878
```
