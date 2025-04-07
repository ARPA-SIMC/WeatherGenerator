# WeatherGenerator
A suite of programs aimed at producing temporal series based on the observational dataset. Daily data of precipitation, maximum temperature and minimum temperature are produced. Two different algorithms have been developed: 1) generation of single point weather data series. 2) generation of a spatially coherent two-dimensional data series.
Each algorithm has a specific directory including all needed dependencies. Both algorithms depend on the agrolib library.

# WG1D
One dimensional Weather Generator (Richardson scheme), based on *Jovanovic, NZ, Annandale, JG, Benade, N. & Campbell, GS, CLIMGEN-UP: A user-friendly weather data generator, South African Journal of Plant and Soil 20.4 (2003): 203-208.*

See [last release](https://github.com/ARPA-SIMC/WeatherGenerator/releases) for installation and usage. 
# WG2D
The algorithm is based on the original weather generator Mulgets https://www.mathworks.com/matlabcentral/fileexchange/47537-multi-site-stochstic-weather-generator-mulgets
The code was first translated in C/C++

## How to compile WG1D / WG2D
Dependencies:
- [Qt libraries](https://www.qt.io/download-qt-installer): Qt 5.x or following.
- Only for Qt 6.x : download also *Qt5 Compatibility Module*

Compile *WG1D/Makeall_WG1D/Makeall_WG1D.pro*  or *WG2D/Makeall_WG2D/Makeall_WG2D.pro*

## License
Agrolib libraries have been developed under contract issued by 
[ARPAE Hydro-Meteo-Climate Service](https://github.com/ARPA-SIMC), Emilia-Romagna, Italy.

agrolib is released under the GNU LGPL license.

## Authors
- Fausto Tomei <ftomei@arpae.it>
- Antonio Volta		<avolta@arpae.it>
- Laura Costantini  <laura.costantini0@gmail.com>

### Contributions
- Gabriele Antolini	 <gantolini@arpae.it>
- Giulia Villani <gvillani@arpae.it>
- Rodica Tomozeiu <rtomozeiu@arpae.it>
- Valentina Pavan <vpavan@arpae.it>
- Marco Turco <marco.turco@um.es>

