This is the user interface of the weather generator 2D of Arpae.
The main body of the algorithm is the translation of the Mulgets weather generator
from Matlab to C/C++ programming language.
The algorithm is totally open.
A simple interface is provided in order to test the software.
The interface provided needs some Qt libraries.
To compile and run you can use the project Makeall_WG2D which includes all dependencies and libraries used by the weather generator and its interface

Input:

1) the subfolder inputData harbours 10 input weather files. Format file .txt
2) the first lines of main.cpp must be modified in order to insert the parameters of your simulations.
Input parameters are divided from the rest of the code.

Copmutation:
the only five functions which must be called to run the weather generator are
    WG2D.initializeData;
    WG2D.setObservedData;
	WG2D.initializeParameters;
    WG2D.computeWeatherGenerator2D;
    WG2D.getWeatherGeneratorOutput; 


Output:
All outputs are stored in the subfolder "outputData"
The default output is given by the synthetic series (daily data of prec,tmax and tmin). One output file for each station/cell.
If required the software computes a monthly statistical analysis providing the main results for precipitation and temperature.
 