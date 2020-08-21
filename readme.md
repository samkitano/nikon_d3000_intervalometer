## NIKON D3000 INTERVALOMETER

A home made intervalometer for Nikon D3000 photo cameras, built around an Arduino Nano board.

## SUMMARY

Intervalometers are the most useful instruments for time-lapse photography, particularly if your camera 
does not have a built-in one, and you don't want to mess around with the proprietary firmware. 
They are also relatively expensive. So, given my late interest in astro-photography, 
as a hobbyist, I've decided to give it a try and build a budget one. This one is specific for the Nikon D3000 camera.
I have NO IDEA if it works with other models/brands. So, please, do not ask. I really don't know.
  
Although the D3000 does not have a port to connect any sort of remote operation device, fortunately it does 
have an IR sensor, located right under the shutter button, which is meant to be used with a remote control. Given it's 
location (camera front), one can only guess it's main purpose is probably for "selfies". We can definitely take 
advantage of that sensor.

## DESCRIPTION

This project is meant to be cheap, simple, and effective, so we can set up:

- the number of shots we want to take
- the time interval between the shots
- a delay time before actually start taking the shots.

It uses 3 simple press buttons:

- one to browse between set up menus (long click to exit main screen, short click to browse between screens)
- one to increase values (short click slow, long click fast)
- one to decrease values (short click slow, long click fast)

And that's it.

Selected values are shown on main screen. Short click and it starts the shooting sequence.

A countdown based on the provided delay time (if any), will then show up, along with an audible signal which 
will become faster as the shooting sequence approaches. The screen will then show which picture # is taking, 
and will go back to the main screen when finished.

## LIMITATIONS 

- max values in the source code can be changed. 
They are in place mostly because of the limited space on the OLED display. 
Also because I don't think I will ever need something bigger.
Feel free to make any code (or hardware) changes you'd like to suit your own needs.

- You need to set your camera up in advance, in order to receive remote signals. 
Please notice that the **MAX TIME** this function 
will be enabled is **15 min**. And you must also set this time up, in your camera. 
After this period, the camera will go 
back to "normal" mode (click to shoot). Nothing we can do about this one, as far as I know :(

## COMPONENTS

- 1 x Arduino "nano"
- 3 x 10K&Omega; pull-down resistors (brown black orange) 250 mw 5%
- 1 x 220&Omega; current limiting resistor (red red brown) 250 mw 5%
- 3 x temporary press buttons
- 1 x piezoelectric buzzer (optional)
- 1 x OLED display 128X64 I2C 0.96"
- 1 x Infrared **emitting** LED in the 940mn range

## ACKNOWLEDGMENT

Shout out to **Aswan Korula** who took the time and effort to analise an actual 
remote control IR signal with his own instruments.
Since I do not own an oscilloscope, or any sort of signal analyzer for that matter, 
it would have been impossible for me to finish this project without his involuntary contribution, 
so BIG THANKS, mate!

Why not visit Aswan's blog, it has some interesting articles:

[Blog](https://bayesianadventures.wordpress.com)

[Remote Hack article](https://bayesianadventures.wordpress.com/2013/08/09/nikon-ml-l3-ir-remote-hack/)

## LICENSE

Intervalometer source code: [MIT license](https://opensource.org/licenses/MIT)

The **OneButton** library: [BSD](http://www.mathertel.de/License.aspx)

