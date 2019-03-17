# bp-covertcomm
bp-covertcomm is an ascii message transmitter and receiver which takes advantage of shared branch predictor resources. This code is written by Nolan Shah for the Computer Architecture (COSC 3330) course at the University of Houston and is based on the paper "Covert Channels Through Branch Predictors: A Feasibility Study" by Evtyushkin, Ponomarev, and Abu-Ghazaleh.  

## Running
There are no dependencies for the covert communications programs. Running *make* will generate two programs: *spy* (receiver) and *trojan* (transmitter). Spy has no arguments, trojan has one: the message. If the message has whitespace (is more than one word long), encapsulate the message with quotation marks.  

If something goes wrong, you can edit the spy to output the signal's value (think of this as the analog value to a digital signal in signal processing), then redirect the output to *out.csv*. Use the python script plot.py (requires mathplotlib) to plot the data in out.csv.