cashbox
=======
This is the backend code running a donation box

The box is configured with two IR break beam sensors that run normally high. Once something is inserted the beam breaks, pulls the pin low, and triggers an interrupt. This interrupt sets the show time and starts the whole thing in motion.

The rest of the box consists of two gear motors to pull the bills in. Four strips of RGB LED lights, and a pair of fans. All of which are driven by MOSFETS (IRL540).

