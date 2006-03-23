/*
 *  See header file for a description of this class.
 *
 *  $Date: 2006/03/14 13:02:42 $
 *  $Revision: 1.3 $
 *  \author G. Cerminara - INFN Torino
 */

#include "RecoLocalMuon/DTRecHit/src/DTLinearDriftAlgo.h"
#include "RecoLocalMuon/DTRecHit/interface/DTTTrigBaseSync.h"
#include "DataFormats/MuonDetId/interface/DTWireId.h"
#include "Geometry/DTGeometry/interface/DTLayer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"

using namespace std;
using namespace edm;

DTLinearDriftAlgo::DTLinearDriftAlgo(const ParameterSet& config) :
  DTRecHitBaseAlgo(config) {
    // Get the Drift Velocity from parameter set. 
    vDrift = config.getParameter<double>("driftVelocity"); // FIXME: Default was 0.00543 cm/ns

    minTime = config.getParameter<double>("minTime"); // FIXME: Default was -3 ns

    maxTime = config.getParameter<double>("maxTime"); // FIXME: Default was 415 ns

    hitResolution = config.getParameter<double>("hitResolution"); // FIXME: Default is 
    // Set verbose output
    debug = config.getUntrackedParameter<bool>("debug");
    
  }



DTLinearDriftAlgo::~DTLinearDriftAlgo(){}



void DTLinearDriftAlgo::setES(const EventSetup& setup) {
  theSync->setES(setup);
}



// First Step
bool DTLinearDriftAlgo::compute(const DTLayer* layer,
				const DTDigi& digi,
				LocalPoint& leftPoint,
				LocalPoint& rightPoint,
				LocalError& error) const {
  // Get Wire position
  LocalPoint locWirePos(layer->specificTopology().wirePosition(digi.wire()), 0, 0);
  const GlobalPoint globWirePos = layer->surface().toGlobal(locWirePos);
  
  return compute(layer, digi, globWirePos, leftPoint, rightPoint, error, 1); 
}


// Second step: the same as 1st step
bool DTLinearDriftAlgo::compute(const DTLayer* layer,
				const DTDigi& digi,
				const float& angle,
				LocalPoint& leftPoint,
				LocalPoint& rightPoint,
				LocalError& error) const {
  // Get Wire position
  LocalPoint locWirePos(layer->specificTopology().wirePosition(digi.wire()), 0, 0);
  const GlobalPoint globWirePos = layer->surface().toGlobal(locWirePos);
  
  return compute(layer, digi, globWirePos, leftPoint, rightPoint, error, 2); 
}



// Third step.
bool DTLinearDriftAlgo::compute(const DTLayer* layer,
				const DTDigi& digi,
				const float& angle,
				const GlobalPoint& globPos, 
				LocalPoint& leftPoint,
				LocalPoint& rightPoint,
				LocalError& error) const {
  return compute(layer, digi, globPos, leftPoint, rightPoint, error, 3); 
}



// Do the actual work.
bool DTLinearDriftAlgo::compute(const DTLayer* layer,
				const DTDigi& digi,
				const GlobalPoint& globPos, 
				LocalPoint& leftPoint,
				LocalPoint& rightPoint,
				LocalError& error,
				int step) const {
  // Get the layerId
  DTLayerId layerId = layer->id();//FIXME: pass it instead of get it from layer
  const DTWireId wireId(layerId, digi.wire());


  // Subtract the offset to the digi time accordingly to the DTTTrigBaseSync concrete instance
  float driftTime = digi.time() - theSync->offset(layer, wireId, globPos); 
  
  // check for out-of-time
  if (driftTime < minTime || driftTime > maxTime) {
    if (debug) cout << "[DTLinearDriftAlgo]*** Drift time out of window for in-time hits "
			      << driftTime << endl;
    // Hits are interpreted as coming from out-of-time pile-up and recHit
    // is ignored.
    return false;
  }

  // Small negative times interpreted as hits close to the wire.
  if (driftTime<0.) driftTime=0;

  // Compute the drift distance
  float drift = driftTime * vDrift;

  // Get Wire position
  LocalPoint locWirePos(layer->specificTopology().wirePosition(digi.wire()), 0, 0);
  //Build the two possible points and the error on the position
  leftPoint  = LocalPoint(locWirePos.x()-drift,
                            locWirePos.y(),
                            locWirePos.z());
  rightPoint = LocalPoint(locWirePos.x()+drift,
                            locWirePos.y(),
                            locWirePos.z());
  error = LocalError(hitResolution*hitResolution,0.,0.);


  if(debug) {
    cout << "[DTLinearDriftAlgo] Compute drift distance, for digi at wire: " << wireId << endl
	 << "       Step:           " << step << endl
	 << "       Digi time:      " << digi.time() << endl
	 << "       Drift time:     " << driftTime << endl
	 << "       Drift distance: " << drift << endl
	 << "       Hit Resolution: " << hitResolution << endl
	 << "       Left point:     " << leftPoint << endl
	 << "       Right point:    " << rightPoint << endl
	 << "       Error:          " << error << endl;
   }
  
  return true;
  
}


float DTLinearDriftAlgo::vDrift;

  
float DTLinearDriftAlgo::hitResolution;

  
float DTLinearDriftAlgo::minTime;

  
float DTLinearDriftAlgo::maxTime;

  
bool DTLinearDriftAlgo::debug;
