erd_analyzer
====================

Copyright (C) Jaakko Julin 2014

This program uses depthfiles created by erd_depth and performs simple operations on them (e.g. averaging over an depth interval) and creates plotfiles necessary to plot the depth profiles by gnuplot.

INSTALLATION
---------------------

    $ make 
    $ sudo make install

USAGE
---------------------

    $ erd_analyzer

see the commands from the sources, sorry about the lack of documentation at this stage.

or try the script

    $ erd_depth depth123 ...
    $ ./analysis.sh 123 100 300

to get results from depth 100 to depth 300. The script is just an example, you need to set colors and axises to suit your preferences.
