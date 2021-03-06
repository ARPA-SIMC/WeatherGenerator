/*!
    \copyright 2018 Fausto Tomei, Gabriele Antolini,
    Alberto Pistocchi, Marco Bittelli, Antonio Volta, Laura Costantini

    This file is part of CRITERIA3D.
    CRITERIA3D has been developed under contract issued by ARPAE Emilia-Romagna

    CRITERIA3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CRITERIA3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with CRITERIA3D.  If not, see <http://www.gnu.org/licenses/>.

    contacts:
    fausto.tomei@gmail.com
    ftomei@arpae.it
    gantolini@arpae.it
*/

#include <QString>
#include <math.h>

#include "commonConstants.h"
#include "basicMath.h"
#include "cropDbTools.h"
#include "water1D.h"
#include "criteria1DCase.h"
#include "soilFluxes3D.h"


Crit1DOutput::Crit1DOutput()
{
    this->initialize();
}

void Crit1DOutput::initialize()
{
    this->dailyPrec = NODATA;
    this->dailyDrainage = NODATA;
    this->dailySurfaceRunoff = NODATA;
    this->dailyLateralDrainage = NODATA;
    this->dailyIrrigation = NODATA;
    this->dailySoilWaterContent = NODATA;
    this->dailySurfaceWaterContent = NODATA;
    this->dailyEt0 = NODATA;
    this->dailyEvaporation = NODATA;
    this->dailyMaxTranspiration = NODATA;
    this->dailyMaxEvaporation = NODATA;
    this->dailyTranspiration = NODATA;
    this->dailyAvailableWater = NODATA;
    this->dailyFractionAW = NODATA;
    this->dailyReadilyAW = NODATA;
    this->dailyCapillaryRise = NODATA;
    this->dailyWaterTable = NODATA;
}


Crit1DCase::Crit1DCase()
{
    minLayerThickness = 0.02;           /*!< [m] layer thickness (default = 2 cm)  */
    geometricFactor = 1.2;              /*!< [-] factor for geometric progression of thickness  */
    ploughedSoilDepth = 0.5;            /*!< [m] depth of ploughed soil (working layer) */

    soilLayers.clear();
    prevWaterContent.clear();
}


bool Crit1DCase::initializeNumericalFluxes(std::string &myError)
{
    int nrLayers = soilLayers.size();
    int lastLayer = nrLayers-1;
    int nrlateralLinks = 0;

    int result = soilFluxes3D::initialize(nrLayers, nrLayers, nrlateralLinks, true, false, false);
    if (result != CRIT3D_OK)
    {
        myError = "Error in initialize numerical fluxes";
        return false;
    }

    float horizontalConductivityRatio = 10.0;
    soilFluxes3D::setHydraulicProperties(fittingOptions.waterRetentionCurve, MEAN_LOGARITHMIC, horizontalConductivityRatio);
    soilFluxes3D::setNumericalParameters(1, 3600, 100, 10, 12, 3);

    // set soil properties (unit: MKS)
    int soilIndex = 0;
    for (unsigned int horizonIndex = 0; horizonIndex < mySoil.nrHorizons; horizonIndex++)
    {
        soil::Crit3DHorizon horizon = mySoil.horizon[horizonIndex];
        float soilFraction = (1.0 - horizon.coarseFragments);
        result = soilFluxes3D::setSoilProperties(soilIndex, horizonIndex,
                            horizon.vanGenuchten.alpha * GRAVITY,
                            horizon.vanGenuchten.n, horizon.vanGenuchten.m,
                            horizon.vanGenuchten.he / GRAVITY,
                            horizon.vanGenuchten.thetaR * soilFraction,
                            horizon.vanGenuchten.thetaS * soilFraction,
                            horizon.waterConductivity.kSat / 100.0 / DAY_SECONDS,
                            horizon.waterConductivity.l,
                            horizon.organicMatter, horizon.texture.clay / 100.0);
        if (result != CRIT3D_OK)
        {
            myError = "Error in setSoilProperties, horizon nr: " + std::to_string(horizonIndex);
            return false;
        }
    }

    // set surface properties
    double maxSurfaceWater = myCrop.getSurfaceWaterPonding();   // [mm]
    maxSurfaceWater /= 1000.0;                                  // [m]
    double roughnessManning = 0.024;                            // [s m^-0.33]
    int surfaceIndex = 0;
    soilFluxes3D::setSurfaceProperties(surfaceIndex, roughnessManning, maxSurfaceWater);

    // field structure (baulatura)
    float x0 = 0;
    float y0 = 0;
    double z0 = 0;
    double lx = 25;                 // [m]
    double ly = 200;                // [m]
    double area = lx * ly;          // [m^2]
    double slope = 0.002;           // [m m^-1]

    // set surface (node 0)
    bool isSurface = true;
    int nodeIndex = 0;
    soilFluxes3D::setNode(nodeIndex, x0, y0, z0, area, isSurface, true, BOUNDARY_RUNOFF, slope, ly);
    soilFluxes3D::setNodeSurface(nodeIndex, surfaceIndex);
    soilFluxes3D::setNodeLink(nodeIndex, nodeIndex + 1, DOWN, area);

    // set nodes
    isSurface = false;
    for (int i = 1; i < nrLayers; i++)
    {
        double volume = area * soilLayers[i].thickness;             // [m^3]
        double z = z0 - soilLayers[i].depth;                        // [m]
        if (i == lastLayer)
        {
            if (unit.useWaterTableData)
                soilFluxes3D::setNode(i, x0, y0, z, volume, isSurface, true, BOUNDARY_PRESCRIBEDTOTALPOTENTIAL, slope, area);
            else
                soilFluxes3D::setNode(i, x0, y0, z, volume, isSurface, true, BOUNDARY_FREEDRAINAGE, slope, area);
        }
        else
        {
            double boundaryArea = ly * soilLayers[i].thickness;
            soilFluxes3D::setNode(i, x0, y0, z, volume, isSurface, true, BOUNDARY_FREELATERALDRAINAGE, slope, boundaryArea);
        }

        // set soil
        int horizonIndex = mySoil.getHorizonIndex(soilLayers[i].depth);
        soilFluxes3D::setNodeSoil(i, soilIndex, horizonIndex);

        // set links
        soilFluxes3D::setNodeLink(i, i-1, UP, area);
        if (i != lastLayer)
        {
            soilFluxes3D::setNodeLink(i, i+1, DOWN, area);
        }
    }

    return true;
}


bool Crit1DCase::initializeSoil(std::string &myError)
{
    soilLayers.clear();

    double factor = 1.0;
    if (unit.isGeometricLayers) factor = geometricFactor;

    if (! mySoil.setSoilLayers(minLayerThickness, factor, soilLayers, myError))
        return false;

    if (unit.isNumericalInfiltration)
    {
        if (! initializeNumericalFluxes(myError))
            return false;
    }

    initializeWater(soilLayers);

    return true;
}


void Crit1DCase::saveWaterContent()
{
    prevWaterContent.clear();
    prevWaterContent.resize(soilLayers.size());
    for (unsigned int i = 0; i < soilLayers.size(); i++)
    {
        prevWaterContent[i] = soilLayers[i].waterContent;
    }
}


void Crit1DCase::restoreWaterContent()
{
    for (unsigned int i = 0; i < soilLayers.size(); i++)
    {
        soilLayers[i].waterContent = prevWaterContent[i];
    }
}


/*!
 * \brief compute water fluxes
 * \param dailyWaterInput [mm] sum of precipitation and irrigation
 */
bool Crit1DCase::computeWaterFluxes(double dailyWaterInput)
{
    if (unit.isNumericalInfiltration)
    {
        // TODO
    }
    else
    {
        // WATERTABLE
        output.dailyCapillaryRise = 0;
        if (unit.useWaterTableData)
        {
            output.dailyCapillaryRise = computeCapillaryRise(soilLayers, output.dailyWaterTable);
        }

        // INFILTRATION
        output.dailyDrainage = computeInfiltration(soilLayers, dailyWaterInput, ploughedSoilDepth);

        // LATERAL DRAINAGE
        output.dailyLateralDrainage = computeLateralDrainage(soilLayers);
    }

    return true;
}


double Crit1DCase::checkIrrigationDemand(int doy, double currentPrec, double nextPrec, double maxTranspiration)
{
    // update days since last irrigation
    if (myCrop.daysSinceIrrigation != NODATA)
        myCrop.daysSinceIrrigation++;

    // check irrigated crop
    if (myCrop.idCrop == "" || ! myCrop.isLiving || isEqual(myCrop.irrigationVolume, NODATA) || isEqual(myCrop.irrigationVolume, 0))
        return 0;

    // check irrigation period
    if (myCrop.doyStartIrrigation != NODATA && myCrop.doyEndIrrigation != NODATA)
    {
        if (doy < myCrop.doyStartIrrigation || doy > myCrop.doyEndIrrigation)
            return 0;
    }
    if (myCrop.degreeDaysStartIrrigation != NODATA && myCrop.degreeDaysEndIrrigation != NODATA)
    {
        if (myCrop.degreeDays < myCrop.degreeDaysStartIrrigation || myCrop.degreeDays > myCrop.degreeDaysEndIrrigation)
            return 0;
    }

    // check forecast (today and tomorrow)
    double waterNeeds = myCrop.irrigationVolume / myCrop.irrigationShift;
    double todayWater = currentPrec + soilLayers[0].waterContent;
    double twoDaysWater = todayWater + nextPrec;
    if (todayWater >= waterNeeds) return 0;
    if (twoDaysWater >= 2*waterNeeds) return 0;

    // check water stress (before infiltration)
    double threshold = 1. - myCrop.stressTolerance;

    double waterStress = 0;
    myCrop.computeTranspiration(maxTranspiration, soilLayers, waterStress);
    if (waterStress <= threshold)
        return 0;

    // check irrigation shift
    if (myCrop.daysSinceIrrigation != NODATA)
    {
        // stress too high -> forced irrigation
        if ((myCrop.daysSinceIrrigation < myCrop.irrigationShift) && (waterStress < (threshold + 0.1)))
            return 0;
    }

    // check irrigation quantity
    double irrigation = myCrop.irrigationVolume;
    if (myCrop.irrigationShift > 1)
        irrigation -= floor(twoDaysWater);

    if (unit.isOptimalIrrigation)
        irrigation = MINVALUE(irrigation, myCrop.getCropWaterDeficit(soilLayers));

    // reset irrigation shift
    myCrop.daysSinceIrrigation = 0;
    return irrigation;
}



/*!
 * \brief run model (daily cycle)
 * \param myDate
 */
bool Crit1DCase::computeDailyModel(Crit3DDate myDate, std::string &myError)
{
    output.initialize();

    int doy = getDoyFromDate(myDate);

    // check daily meteo data
    if (! meteoPoint.existDailyData(myDate))
    {
        myError = "Missing weather data: " + myDate.toStdString();
        return false;
    }

    double prec = double(meteoPoint.getMeteoPointValueD(myDate, dailyPrecipitation));
    double tmin = double(meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMin));
    double tmax = double(meteoPoint.getMeteoPointValueD(myDate, dailyAirTemperatureMax));

    if (isEqual(prec, NODATA) || isEqual(tmin, NODATA) || isEqual(tmax, NODATA))
    {
        myError = "Missing weather data: " + myDate.toStdString();
        return false;
    }

    // check on wrong data
    if (prec < 0) prec = 0;
    output.dailyPrec = prec;

    // water table
    output.dailyWaterTable = double(meteoPoint.getMeteoPointValueD(myDate, dailyWaterTableDepth));
    // check
    if (output.dailyWaterTable != NODATA)
        output.dailyWaterTable = MAXVALUE(output.dailyWaterTable, 0.01);

    // prec forecast
    double precTomorrow = double(meteoPoint.getMeteoPointValueD(myDate.addDays(1), dailyPrecipitation));
    if (isEqual(precTomorrow, NODATA)) precTomorrow = 0;

    // ET0
    output.dailyEt0 = double(meteoPoint.getMeteoPointValueD(myDate, dailyReferenceEvapotranspirationHS));
    if (isEqual(output.dailyEt0, NODATA) || output.dailyEt0 <= 0)
        output.dailyEt0 = ET0_Hargreaves(TRANSMISSIVITY_SAMANI_COEFF_DEFAULT, meteoPoint.latitude, doy, tmax, tmin);

    // update LAI and root depth
    if (! myCrop.dailyUpdate(myDate, meteoPoint.latitude, soilLayers, tmin, tmax, output.dailyWaterTable, myError))
        return false;

    // Evaporation / transpiration
    output.dailyMaxEvaporation = myCrop.getMaxEvaporation(output.dailyEt0);
    output.dailyMaxTranspiration = myCrop.getMaxTranspiration(output.dailyEt0);

    // WATER FLUXES
    saveWaterContent();
    if (! computeWaterFluxes(output.dailyPrec)) return false;

    // IRRIGATION
    output.dailyIrrigation = 0;
    double irrigation = checkIrrigationDemand(doy, prec, precTomorrow, output.dailyMaxTranspiration);

    // assign irrigation: optimal (subirrigation) or add to precipitation (sprinkler)
    if (irrigation > 0)
    {
        restoreWaterContent();
        double totalWaterInput;
        if (unit.isOptimalIrrigation)
        {
            output.dailyIrrigation = assignOptimalIrrigation(soilLayers, irrigation);
            totalWaterInput = prec;
        }
        else
        {
            output.dailyIrrigation = irrigation;
            totalWaterInput = prec + irrigation;
        }
        // recompute water fluxes
        if (! computeWaterFluxes(totalWaterInput)) return false;
    }

    // EVAPORATION
    output.dailyEvaporation = computeEvaporation(soilLayers, output.dailyMaxEvaporation);

    // RUNOFF (after evaporation)
    output.dailySurfaceRunoff = computeSurfaceRunoff(myCrop, soilLayers);

    // adjust irrigation losses
    if (! unit.isOptimalIrrigation)
    {
        if ((output.dailySurfaceRunoff > 1) && (output.dailyIrrigation > 0))
        {
            output.dailyIrrigation -= floor(output.dailySurfaceRunoff);
            output.dailySurfaceRunoff -= floor(output.dailySurfaceRunoff);
        }
    }

    // TRANSPIRATION
    double waterStress = 0;
    output.dailyTranspiration = myCrop.computeTranspiration(output.dailyMaxTranspiration, soilLayers, waterStress);

    // assign transpiration
    if (output.dailyTranspiration > 0)
    {
        for (unsigned int i = unsigned(myCrop.roots.firstRootLayer); i <= unsigned(myCrop.roots.lastRootLayer); i++)
        {
            soilLayers[i].waterContent -= myCrop.layerTranspiration[i];
        }
    }

    // output variables
    output.dailySurfaceWaterContent = soilLayers[0].waterContent;
    output.dailySoilWaterContent = getSoilWaterContent(soilLayers, 1.0);
    output.dailyAvailableWater = getSoilAvailableWater(soilLayers, 1.0);
    output.dailyFractionAW = getSoilFractionAW(soilLayers, 1.0);
    output.dailyReadilyAW = getReadilyAvailableWater(myCrop, soilLayers);

    return true;
}


/*!
 * \brief get volumetric water content at specific depth
 * \param depth = computation soil depth  [cm]
 * \return volumetric water content [-]
 */
double Crit1DCase::getWaterContent(double depth)
{
    depth /= 100;                                   // [cm] --> [m]
    if (depth <= 0 || depth > mySoil.totalDepth)
        return NODATA;

    double upperDepth, lowerDepth;
    for (unsigned int i = 1; i < soilLayers.size(); i++)
    {
        upperDepth = soilLayers[i].depth - soilLayers[i].thickness * 0.5;
        lowerDepth = soilLayers[i].depth + soilLayers[i].thickness * 0.5;
        if (depth >= upperDepth && depth <= lowerDepth)
        {
            return soilLayers[i].waterContent / (soilLayers[i].thickness * 1000);
        }
    }

    return NODATA;
}


/*!
 * \brief get water potential at specific depth
 * \param depth = computation soil depth  [cm]
 * \return water potential [kPa]
 */
double Crit1DCase::getWaterPotential(double depth)
{
    depth /= 100;                                   // [cm] --> [m]
    if (depth <= 0 || depth > mySoil.totalDepth)
        return NODATA;

    double upperDepth, lowerDepth;
    for (unsigned int i = 1; i < soilLayers.size(); i++)
    {
        upperDepth = soilLayers[i].depth - soilLayers[i].thickness * 0.5;
        lowerDepth = soilLayers[i].depth + soilLayers[i].thickness * 0.5;
        if (depth >= upperDepth && depth <= lowerDepth)
        {
            return soilLayers[i].getWaterPotential();
        }
    }

    return NODATA;
}


/*!
 * \brief getSoilWaterDeficit
 * \param depth = computation soil depth  [cm]
 * \return sum of water deficit from zero to depth (mm)
 */
double Crit1DCase::getSoilWaterDeficit(double depth)
{
    depth /= 100;                           // [cm] --> [m]
    double lowerDepth, upperDepth;          // [m]
    double waterDeficitSum = 0;             // [mm]

    for (unsigned int i = 1; i < soilLayers.size(); i++)
    {
        lowerDepth = soilLayers[i].depth + soilLayers[i].thickness * 0.5;

        if (lowerDepth < depth)
        {
            waterDeficitSum += soilLayers[i].FC - soilLayers[i].waterContent;
        }
        else
        {
            // fraction of last layer
            upperDepth = soilLayers[i].depth - soilLayers[i].thickness * 0.5;
            double layerDeficit = soilLayers[i].FC - soilLayers[i].waterContent;
            double depthFraction = (depth - upperDepth) / soilLayers[i].thickness;
            return waterDeficitSum + layerDeficit * depthFraction;
        }
    }

    return waterDeficitSum;
}


