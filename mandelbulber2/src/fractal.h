/**
 * Mandelbulber v2, a 3D fractal generator
 *
 * cFractal class - container for fractal formula parameters
 *
 * Copyright (C) 2014 Krzysztof Marczak
 *
 * This file is part of Mandelbulber.
 *
 * Mandelbulber is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Mandelbulber is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with Mandelbulber. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Krzysztof Marczak (buddhi1980@gmail.com)
 */

#ifndef FRACTAL_H_
#define FRACTAL_H_

#include "algebra.hpp"
#include "fractal_list.hpp"
#include "parameters.hpp"

const int IFS_VECTOR_COUNT = 9;
const int HYBRID_COUNT = 5;
const int MANDELBOX_FOLDS = 2;

namespace fractal
{
enum enumCalculationMode
{
	calcModeNormal = 0, calcModeColouring = 1, calcModeFake_AO = 2, calcModeDeltaDE1 = 3, calcModeDeltaDE2 = 4, calcModeOrbitTrap = 5
};

enum enumGeneralizedFoldBoxType
{
	foldTet = 0, foldCube = 1, foldOct = 2, foldDodeca = 3, foldOctCube = 4, foldIcosa = 5, foldBox6 = 6, foldBox5 = 7
};

enum enumOCLDEMode
{
	ocl_DEcalculated = 0, ocl_deltaDE = 1, ocl_noDE = 2
};
}

struct sFractalGeneralizedFoldBox
{
	fractal::enumGeneralizedFoldBoxType type;
	CVector3 Nv_tet[4];
	CVector3 Nv_cube[6];
	CVector3 Nv_oct[8];
	CVector3 Nv_oct_cube[14];
	CVector3 Nv_dodeca[12];
	CVector3 Nv_icosa[20];
	CVector3 Nv_box6[8];
	CVector3 Nv_box5[7];
	int sides_tet;
	int sides_cube;
	int sides_oct;
	int sides_oct_cube;
	int sides_dodeca;
	int sides_icosa;
	int sides_box6;
	int sides_box5;
};

struct sFractalIFS
{
	bool absX, absY, absZ;
	bool enabled[IFS_VECTOR_COUNT];
	bool foldingMode; // Kaleidoscopic IFS folding mode
	bool mengerSpongeMode;
	bool rotationEnabled;
	bool edgeEnabled;
	CRotationMatrix mainRot;
	CRotationMatrix rot[IFS_VECTOR_COUNT];
	CVector3 direction[IFS_VECTOR_COUNT];
	CVector3 edge;
	CVector3 offset;
	CVector3 rotations[IFS_VECTOR_COUNT];
	double distance[IFS_VECTOR_COUNT];
	double intensity[IFS_VECTOR_COUNT];
	CVector3 rotation;
	double scale;
	int foldingCount;
};

struct sFractalMandelboxVary4D
{
	double fold;
	double minR;
	double scaleVary;
	double wadd;
	double rPower;
};

struct sFractalMandelbox
{
	CVector3 rotationMain;
	CVector3 rotation[MANDELBOX_FOLDS][3];
	CVector3 colorFactor;
	double colorFactorR;
	double colorFactorSp1;
	double colorFactorSp2;
	double scale;
	double foldingLimit;
	double foldingValue;
	double foldingSphericalMin;
	double foldingSphericalFixed;
	double sharpness;
	double solid;
	double melt;
	CVector3 offset;
	bool rotationsEnabled;
	bool mainRotationEnabled;
	CRotationMatrix mainRot;
	CRotationMatrix rot[MANDELBOX_FOLDS][3];
	CRotationMatrix rotinv[MANDELBOX_FOLDS][3];

	double fR2;
	double mR2;
	double mboxFactor1;
};

struct sFractalBoxFoldBulbPow2
{
	double zFactor;
	double foldfactor;
};


struct sFractalMandelbulb
{
	double power;
	double alphaAngleOffset;
	double betaAngleOffset;
	double gammaAngleOffset;
};

struct sFractalAexion
{
	double cadd;
};

struct sFractalBuffalo
{
	bool preabsx;
	bool preabsy;
	bool preabsz;
	bool absx;
	bool absy;
	bool absz;
  bool posz;
};

struct sMsltoeSym2
{
	double y_multiplier;
};
  //----------------------Structure Transform---------------------------------
struct sTransformControl
{
  bool enabled;
  bool weightEnabled;
  bool oldWeightEnabled;
  int startIterations;
  int stopIterations;
  double weight;
  double oldWeight;
};

struct sTransformAdditionConstant
{
  sTransformControl control;
  CVector3 additionConstant;
  // for 4D
  CVector4 additionConstant4D;

};
struct sTransformBoxConstantMultiplier
{
  sTransformControl control;
  bool boxConstantMultiplierFabsEnabledX;
  bool boxConstantMultiplierFabsEnabledY;
  bool boxConstantMultiplierFabsEnabledZ;
  CVector3 boxConstantMultiplier;
};
struct sTransformBoxFoldOriginal
{
  sTransformControl control;
  double foldingLimit;
  double foldingValue;
};
struct sTransformBoxFold
{
  sTransformControl control;
  double foldingLimit;
  double foldingValue;
};
struct sTransformBoxOffset
{
  sTransformControl control;
  CVector3 boxOffset;
};
struct sTransformConstantMultiplierOriginal
{
  sTransformControl control;
  CVector3 constantMultiplierVect;
  //extra for 4D
  CVector4 constantMultiplierVect4D;
};
struct sTransformConstantMultiplier
{
  sTransformControl control;
  CVector3 constantMultiplierVect;
  //extra for 4D
  CVector4 constantMultiplierVect4D;
};
struct sTransformFabsAddConstant
{
  sTransformControl control;
  bool fabsAddConstantEnabledx;
  bool fabsAddConstantEnabledy;
  bool fabsAddConstantEnabledz;
  bool fabsAddConstantEnabledw;
  CVector3 fabsAddConstantA;
  CVector3 fabsAddConstantB;
  CVector3 fabsAddConstantC;
  // for 4D
  CVector4 fabsAddConstant4DA;
  CVector4 fabsAddConstant4DB;
  CVector4 fabsAddConstant4DC;
};
struct sTransformFabsFormulaAB
{
  sTransformControl control;
  bool fabsFormulaABEnabledx;
  bool fabsFormulaABEnabledy;
  bool fabsFormulaABEnabledz;
  bool fabsFormulaABEnabledw;
  CVector3 fabsFormulaABConstantA;
  CVector3 fabsFormulaABConstantB;
};
struct sTransformFabsFormulaABCD
{
  sTransformControl control;
  bool fabsFormulaABCDEnabledAx;
  bool fabsFormulaABCDEnabledAy;
  bool fabsFormulaABCDEnabledAz;
  bool fabsFormulaABCDEnabledBx;
  bool fabsFormulaABCDEnabledBy;
  bool fabsFormulaABCDEnabledBz;
  CVector3 fabsFormulaABCDA;
  CVector3 fabsFormulaABCDB;
  CVector3 fabsFormulaABCDC;
  CVector3 fabsFormulaABCDD;
  // for 4D
  CVector4 fabsFormulaABCD4DA;
  CVector4 fabsFormulaABCD4DB;
  CVector4 fabsFormulaABCD4DC;
  CVector4 fabsFormulaABCD4DD;
  bool fabsFormulaABCDEnabledAw;
  bool fabsFormulaABCDEnabledBw;
};
struct sTransformFabsFormulaZAB
{
  sTransformControl control;
  bool fabsFormulaZABEnabledx;
  bool fabsFormulaZABEnabledy;
  bool fabsFormulaZABEnabledz;
  CVector3 fabsFormulaZABConstantA;
  CVector3 fabsFormulaZABConstantB;
  CVector3 fabsFormulaZABConstantC;
};
struct sTransformFabsSubConstant
{
  sTransformControl control;
  bool fabsSubConstantEnabledx;
  bool fabsSubConstantEnabledy;
  bool fabsSubConstantEnabledz;
  bool fabsSubConstantEnabledw;
  CVector3 fabsSubConstantA;
  CVector3 fabsSubConstantB;
};

struct sTransformIterationWeight
{
  sTransformControl control;
  int iterationWeightIterationA;
  int iterationWeightIterationB;
  double iterationWeightConstantZ;
  double iterationWeightConstantA;
  double iterationWeightConstantB;
};
struct sTransformMandelbulbOriginal
{
  sTransformControl control;
  double power;
  double alphaAngleOffset;
  double betaAngleOffset;
  double gammaAngleOffset;
};
struct sTransformMandelbulb
{
  sTransformControl control;
  double mandelbulbPower;
  double mandelbulbAlphaAngleOffset;
  double mandelbulbBetaAngleOffset;
};


struct sTransformMainRotation
{
  sTransformControl control;
  CVector3 mainRotation;
  CRotationMatrix mainRot;
};

struct sTransformMengerSpongeOriginal
{
  sTransformControl control;
  double mengerSpongeOriginalConstantZ;
  CVector3 mengerSpongeOriginalFactorConstantVect;
};
struct sTransformMengerSponge
{
  sTransformControl control;
  double mengerSpongeConstantZ;
  CVector3 mengerSpongeFactorConstantVect;
};
struct sTransformQuaternionOriginal
{
  sTransformControl control;
  CVector4  quaternionOriginalFactorConstant;
};
struct sTransformQuaternion
{
  sTransformControl control;
  CVector4 quaternionFactorConstant;
};
struct sTransformScaleOriginal
{
  sTransformControl control;
  double scaleOriginal;
};
struct sTransformScale
{
  sTransformControl control;
  double scale;
};
struct sTransformSphericalFoldOriginal
{
  sTransformControl control;
  double radMin;
  double radFixed;
  double fR2;
  double mR2;
  double mboxFactor1;
};

struct sTransformSphericalFold
{
  sTransformControl control;
  double radMin;
  double radFixed;
  double fR2;
  double mR2;
  double mboxFactor1;
};

struct sTransformSphericalOffset
{
  sTransformControl control;
  double offsetRadius;
  double scale;
};




struct sFractalTransform
{
  sTransformAdditionConstant additionConstant1;
  sTransformAdditionConstant additionConstant2;
  sTransformBoxConstantMultiplier boxConstantMultiplier1;
  sTransformBoxConstantMultiplier boxConstantMultiplier2;
  sTransformBoxFoldOriginal boxFoldOriginal1;
  sTransformBoxFold boxFold1;
  sTransformBoxFold boxFold2;
  sTransformBoxOffset boxOffset1;
  sTransformBoxOffset boxOffset2;
  sTransformConstantMultiplierOriginal constantMultiplierOriginal1;
  sTransformConstantMultiplier  constantMultiplier1;
  sTransformConstantMultiplier  constantMultiplier2;

  sTransformFabsAddConstant fabsAddConstant1;
  sTransformFabsFormulaAB fabsFormulaAB1;
  sTransformFabsFormulaAB fabsFormulaAB2;
  sTransformFabsFormulaABCD fabsFormulaABCD1;
  sTransformFabsFormulaABCD fabsFormulaABCD2;
  sTransformFabsFormulaZAB fabsFormulaZAB1;
  sTransformFabsSubConstant fabsSubConstant1;
  sTransformIterationWeight iterationWeight1;
  sTransformMandelbulbOriginal  mandelbulbOriginal1;
  sTransformMandelbulb  mandelbulb1;
  sTransformMainRotation mainRotation1;
  sTransformMainRotation mainRotation2;
  sTransformMainRotation mainRotation3;
  sTransformMengerSpongeOriginal mengerSpongeOriginal1;
  sTransformMengerSponge mengerSponge1;
  sTransformMengerSponge mengerSponge2;
  sTransformQuaternionOriginal quaternionOriginal1;
  sTransformQuaternion quaternion1;
  sTransformScaleOriginal scaleOriginal1;
  sTransformScale scale1;
  sTransformScale scale2;
  sTransformSphericalFoldOriginal sphericalFoldOriginal1;
  sTransformSphericalFold sphericalFold1;
  sTransformSphericalFold sphericalFold2;
  sTransformSphericalOffset sphericalOffset1;
  sTransformSphericalOffset sphericalOffset2;

  //TODO here will be more transforms
};

struct sFractalMandelbulb5
{




    bool mandelbulb1Enabled;
    //bool mandelbulb1WeightEnabled;
    int mandelbulb1StartIterations;
    int mandelbulb1StopIterations;
    double mandelbulb1Power;
    double mandelbulb1AlphaAngleOffset;
    double mandelbulb1BetaAngleOffset;
    //double mandelbulb1Weight;

    bool constantMultiplier1Enabled;
    bool constantMultiplier1WeightEnabled;
    int constantMultiplier1StartIterations;
    int constantMultiplier1StopIterations;
    CVector3 constantMultiplier1Vect;
    double constantMultiplier1Weight;

    bool fabsSubConstant1Enabled;
    bool fabsSubConstant1WeightEnabled;
    bool fabsSubConstant1OldWeightEnabled;
    bool fabsSubConstant1Enabledx;
    bool fabsSubConstant1Enabledy;
    bool fabsSubConstant1Enabledz;
    int fabsSubConstant1StartIterations;
    int fabsSubConstant1StopIterations;
    CVector3 fabsSubConstant1A;
    CVector3 fabsSubConstant1B;
    double fabsSubConstant1Weight;
    double fabsSubConstant1OldWeight;

    bool fabsFormulaZAB1Enabled;
    bool fabsFormulaZAB1WeightEnabled;
    bool fabsFormulaZAB1OldWeightEnabled;
    bool fabsFormulaZAB1Enabledx;
    bool fabsFormulaZAB1Enabledy;
    bool fabsFormulaZAB1Enabledz;
    int fabsFormulaZAB1StartIterations;
    int fabsFormulaZAB1StopIterations;
    CVector3 fabsFormulaZAB1A;
    CVector3 fabsFormulaZAB1B;
    CVector3 fabsFormulaZAB1C;
    double fabsFormulaZAB1Weight;
    double fabsFormulaZAB1OldWeight;

    bool additionConstant1Enabled;
    bool additionConstant1WeightEnabled;
    int additionConstant1StartIterations;
    int additionConstant1StopIterations;
    CVector3 additionConstant1;
    double additionConstant1Weight;

    bool fabsFormulaAB1Enabled;
    bool fabsFormulaAB1WeightEnabled;
    bool fabsFormulaAB1OldWeightEnabled;
    bool fabsFormulaAB1Enabledx;
    bool fabsFormulaAB1Enabledy;
    bool fabsFormulaAB1Enabledz;
    int fabsFormulaAB1StartIterations;
    int fabsFormulaAB1StopIterations;
    CVector3 fabsFormulaAB1A;
    CVector3 fabsFormulaAB1B;
    double fabsFormulaAB1Weight;
    double fabsFormulaAB1OldWeight;

    bool mainRotation2Enabled;
    bool mainRotation2WeightEnabled;
    int mainRotation2StartIterations;
    int mainRotation2StopIterations;
    CVector3 mainRotation2;
    CRotationMatrix mainRot2;
    double mainRotation2Weight;

    bool fabsFormulaABCD1Enabled;
    bool fabsFormulaABCD1WeightEnabled;
    bool fabsFormulaABCD1OldWeightEnabled;
    bool fabsFormulaABCD1EnabledAx;
    bool fabsFormulaABCD1EnabledAy;
    bool fabsFormulaABCD1EnabledAz;
    bool fabsFormulaABCD1EnabledBx;
    bool fabsFormulaABCD1EnabledBy;
    bool fabsFormulaABCD1EnabledBz;
    int fabsFormulaABCD1StartIterations;
    int fabsFormulaABCD1StopIterations;
    CVector3 fabsFormulaABCD1A;
    CVector3 fabsFormulaABCD1B;
    CVector3 fabsFormulaABCD1C;
    CVector3 fabsFormulaABCD1D;
    double fabsFormulaABCD1Weight;
    double fabsFormulaABCD1OldWeight;


    bool mandelbulb2Enabled;
    //bool mandelbulb2WeightEnabled;
    int mandelbulb2StartIterations;
    int mandelbulb2StopIterations;
    double mandelbulb2Power;
    double mandelbulb2AlphaAngleOffset;
    double mandelbulb2BetaAngleOffset;
    //double mandelbulb2Weight;

    bool mainRotation3Enabled;
    bool mainRotation3WeightEnabled;
    int mainRotation3StartIterations;
    int mainRotation3StopIterations;
    CVector3 mainRotation3;
    CRotationMatrix mainRot3;
    double mainRotation3Weight;

    bool constantMultiplier2Enabled;
    bool constantMultiplier2WeightEnabled;
    int constantMultiplier2StartIterations;
    int constantMultiplier2StopIterations;
    CVector3 constantMultiplier2Vect;
    double constantMultiplier2Weight;

    bool additionConstant2Enabled;
    bool additionConstant2WeightEnabled;
    int additionConstant2StartIterations;
    int additionConstant2StopIterations;
    CVector3 additionConstant2;
    double additionConstant2Weight;


    bool fabsFormulaAB2Enabled;
    bool fabsFormulaAB2WeightEnabled;
    bool fabsFormulaAB2OldWeightEnabled;
    bool fabsFormulaAB2Enabledx;
    bool fabsFormulaAB2Enabledy;
    bool fabsFormulaAB2Enabledz;
    int fabsFormulaAB2StartIterations;
    int fabsFormulaAB2StopIterations;
    CVector3 fabsFormulaAB2A;
    CVector3 fabsFormulaAB2B;
    double fabsFormulaAB2Weight;
    double fabsFormulaAB2OldWeight;

    bool boxFold2Enabled;
    bool boxFold2WeightEnabled;
    bool boxFold2OldWeightEnabled;
    int boxFold2StartIterations;
    int boxFold2StopIterations;
    double boxFold2FoldingLimit;
    double boxFold2FoldingValue;
    double boxFold2Weight;
    double boxFold2OldWeight;

};

struct sFractalMandelbox103
{
    bool sphericalFold1Enabled;
    bool sphericalFold1WeightEnabled;
    double sphericalFold1RadMin;
    double sphericalFold1RadFixed;
    double sphericalFold1fR2;
    double sphericalFold1mR2;
    double sphericalFold1MboxFactor1;
    int sphericalFold1StartIterations;
    int sphericalFold1StopIterations;
    double sphericalFold1Weight;

    bool scale1Enabled;
    double scale1;
    int scale1StartIterations;
    int scale1StopIterations;

    bool mainRotation1Enabled;
    bool mainRotation1WeightEnabled;
    int mainRotation1StartIterations;
    int mainRotation1StopIterations;
    CVector3 mainRotation1;
    CRotationMatrix mainRot1;
    double mainRotation1Weight;


    bool constantMultiplier1Enabled;
    bool constantMultiplier1WeightEnabled;
    int constantMultiplier1StartIterations;
    int constantMultiplier1StopIterations;
    CVector3 constantMultiplier1Vect;
    double constantMultiplier1Weight;

    bool additionConstant1Enabled;
    bool additionConstant1WeightEnabled;
    int additionConstant1StartIterations;
    int additionConstant1StopIterations;
    CVector3 additionConstant1;
    double additionConstant1Weight;

    bool fabsFormulaABCD1Enabled;
    bool fabsFormulaABCD1WeightEnabled;
    bool fabsFormulaABCD1EnabledAx;
    bool fabsFormulaABCD1EnabledAy;
    bool fabsFormulaABCD1EnabledAz;
    bool fabsFormulaABCD1EnabledBx;
    bool fabsFormulaABCD1EnabledBy;
    bool fabsFormulaABCD1EnabledBz;
    int fabsFormulaABCD1StartIterations;
    int fabsFormulaABCD1StopIterations;
    CVector3 fabsFormulaABCD1A;
    CVector3 fabsFormulaABCD1B;
    CVector3 fabsFormulaABCD1C;
    CVector3 fabsFormulaABCD1D;
    double fabsFormulaABCD1Weight;

    // main mandelbox formula 2

    bool sphericalFold2Enabled;
    bool sphericalFold2WeightEnabled;
    double sphericalFold2RadMin;
    double sphericalFold2RadFixed;
    double sphericalFold2fR2;
    double sphericalFold2mR2;
    double sphericalFold2MboxFactor1;
    int sphericalFold2StartIterations;
    int sphericalFold2StopIterations;
    double sphericalFold2Weight;

    bool scale2Enabled;
    double scale2;
    int scale2StartIterations;
    int scale2StopIterations;

    bool mainRotation2Enabled;
    bool mainRotation2WeightEnabled;
    int mainRotation2StartIterations;
    int mainRotation2StopIterations;
    CVector3 mainRotation2;
    CRotationMatrix mainRot2;
    double mainRotation2Weight;

    bool constantMultiplier2Enabled;
    bool constantMultiplier2WeightEnabled;
    int constantMultiplier2StartIterations;
    int constantMultiplier2StopIterations;
    CVector3 constantMultiplier2Vect;
    double constantMultiplier2Weight;

    bool additionConstant2Enabled;
    bool additionConstant2WeightEnabled;
    int additionConstant2StartIterations;
    int additionConstant2StopIterations;
    CVector3 additionConstant2;
    double additionConstant2Weight;

    bool mainRotation3Enabled;
    bool mainRotation3WeightEnabled;
    int mainRotation3StartIterations;
    int mainRotation3StopIterations;
    CVector3 mainRotation3;
    CRotationMatrix mainRot3;
    double mainRotation3Weight;

    bool fabsFormulaABCD2Enabled;
    bool fabsFormulaABCD2WeightEnabled;
    bool fabsFormulaABCD2EnabledAx;
    bool fabsFormulaABCD2EnabledAy;
    bool fabsFormulaABCD2EnabledAz;
    bool fabsFormulaABCD2EnabledBx;
    bool fabsFormulaABCD2EnabledBy;
    bool fabsFormulaABCD2EnabledBz;
    int fabsFormulaABCD2StartIterations;
    int fabsFormulaABCD2StopIterations;
    CVector3 fabsFormulaABCD2A;
    CVector3 fabsFormulaABCD2B;
    CVector3 fabsFormulaABCD2C;
    CVector3 fabsFormulaABCD2D;
    double fabsFormulaABCD2Weight;

    bool iterationWeight1Enabled;
    bool iterationWeight1WeightEnabled;
    int iterationWeight1IterationA;
    int iterationWeight1IterationB;
    double iterationWeight1ConstantZ;
    double iterationWeight1ConstantA;
    double iterationWeight1ConstantB;
    double iterationWeight1Weight;

};
struct sFractalQuaternion104
{


};
    //----------------------------------------------------------
struct sFractalMengerSponge105
{

};


//----------------------------------------------------------
struct sFractalPlatonicSolid
{
	double frequency;
	double amplitude;
	double rhoMul;
};

class cFractal
{
public:
	cFractal(const cParameterContainer *par);
	void RecalculateFractalParams(void);

	fractal::enumFractalFormula formula;
	sFractalMandelbulb bulb;
	sFractalIFS IFS;
	sFractalMandelbox mandelbox;
	sFractalGeneralizedFoldBox genFoldBox;
	sFractalBoxFoldBulbPow2 foldingIntPow;
	sFractalMandelboxVary4D mandelboxVary4D;
	sFractalAexion aexion;
	sFractalBuffalo buffalo;
	sMsltoeSym2 msltoeSym2;
	sFractalMandelbulb5 mandelbulb5;
  sFractalMandelbox103 mandelbox103;
  sFractalQuaternion104 quaternion104;
  sFractalMengerSponge105 mengerSponge105;
  sFractalTransform transform;
	sFractalPlatonicSolid platonicSolid;

#ifdef CLSUPPORT
	double customParameters[15];
	double deltaDEStep;
	char customOCLFormulaName[100];
	fractal::enumOCLDEMode customOCLFormulaDEMode;
#endif


};

#endif /* FRACTAL_H_ */
