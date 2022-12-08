/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#include "GateDigitizerAdderActor.h"
#include "../GateHelpersDict.h"
#include "GateDigiAdderInVolume.h"
#include "GateDigiCollectionManager.h"
#include <iostream>

GateDigitizerAdderActor::GateDigitizerAdderActor(py::dict &user_info)
    : GateVDigitizerWithOutputActor(user_info, true) {
  // actions (in addition of the ones in GateVDigitizerWithOutputActor)
  fActions.insert("EndOfEventAction");

  // policy
  fPolicy = AdderPolicy::Error;
  auto policy = DictGetStr(user_info, "policy");
  if (policy == "EnergyWinnerPosition")
    fPolicy = AdderPolicy::EnergyWinnerPosition;
  else if (policy == "EnergyWeightedCentroidPosition")
    fPolicy = AdderPolicy::EnergyWeightedCentroidPosition;
  if (fPolicy == AdderPolicy::Error) {
    std::ostringstream oss;
    oss << "Error in GateDigitizerAdderActor: unknown policy. Must be "
           "EnergyWinnerPosition or EnergyWeightedCentroidPosition"
        << " while '" << policy << "' is read.";
    Fatal(oss.str());
  }

  // init
  fGroupVolumeDepth = -1;
}

GateDigitizerAdderActor::~GateDigitizerAdderActor() = default;

void GateDigitizerAdderActor::SetGroupVolumeDepth(int depth) {
  fGroupVolumeDepth = depth;
}

void GateDigitizerAdderActor::StartSimulationAction() {
  // Init output
  GateVDigitizerWithOutputActor::StartSimulationAction();
  // check required attributes
  CheckRequiredAttribute(fInputDigiCollection, "TotalEnergyDeposit");
  CheckRequiredAttribute(fInputDigiCollection, "PostPosition");
  CheckRequiredAttribute(fInputDigiCollection, "PreStepUniqueVolumeID");
  CheckRequiredAttribute(fInputDigiCollection, "GlobalTime");
}

void GateDigitizerAdderActor::InitializeComputation(
    const std::vector<std::string> &attributes_not_in_filler) {
  // remote the attributes that will be computed here
  std::vector<std::string> att = attributes_not_in_filler;
  att.push_back("TotalEnergyDeposit");
  att.push_back("PostPosition");
  att.push_back("GlobalTime");
  GateVDigitizerWithOutputActor::InitializeComputation(att);

  // Get thread local variables
  auto &l = fThreadLocalData.Get();

  // set output pointers to the attributes needed for computation
  fOutputEdepAttribute =
      fOutputDigiCollection->GetDigiAttribute("TotalEnergyDeposit");
  fOutputPosAttribute = fOutputDigiCollection->GetDigiAttribute("PostPosition");
  fOutputGlobalTimeAttribute =
      fOutputDigiCollection->GetDigiAttribute("GlobalTime");

  // set input pointers to the attributes needed for computation
  auto &lr = fThreadLocalVDigitizerData.Get();
  lr.fInputIter = fInputDigiCollection->NewIterator();
  lr.fInputIter.TrackAttribute("TotalEnergyDeposit", &l.edep);
  lr.fInputIter.TrackAttribute("PostPosition", &l.pos);
  lr.fInputIter.TrackAttribute("PreStepUniqueVolumeID", &l.volID);
  lr.fInputIter.TrackAttribute("GlobalTime", &l.time);
}

void GateDigitizerAdderActor::EndOfEventAction(const G4Event * /*unused*/) {
  // loop on all hits to group per volume ID
  auto &lr = fThreadLocalVDigitizerData.Get();
  auto &iter = lr.fInputIter;
  iter.GoToBegin();
  while (!iter.IsAtEnd()) {
    AddDigiPerVolume();
    iter++;
  }

  // create the output hits collection for grouped hits
  auto &l = fThreadLocalData.Get();
  for (auto &h : l.fMapOfDigiInVolume) {
    auto &hit = h.second;
    // terminate the merge
    hit.Terminate(fPolicy);
    // Don't store if edep is zero
    if (hit.fFinalEdep > 0) {
      // (all "Fill" calls are thread local)
      fOutputEdepAttribute->FillDValue(hit.fFinalEdep);
      fOutputPosAttribute->Fill3Value(hit.fFinalPosition);
      fOutputGlobalTimeAttribute->FillDValue(hit.fFinalTime);
      lr.fDigiAttributeFiller->Fill(hit.fFinalIndex);
    }
  }

  // reset the structure of hits
  l.fMapOfDigiInVolume.clear();
}

void GateDigitizerAdderActor::AddDigiPerVolume() {
  auto &l = fThreadLocalData.Get();
  auto &lr = fThreadLocalVDigitizerData.Get();
  const auto &i = lr.fInputIter.fIndex;
  if (*l.edep == 0)
    return;
  auto uid = l.volID->get()->GetIdUpToDepth(fGroupVolumeDepth);
  if (l.fMapOfDigiInVolume.count(uid) == 0) {
    l.fMapOfDigiInVolume[uid] = GateDigiAdderInVolume();
  }
  l.fMapOfDigiInVolume[uid].Update(fPolicy, i, *l.edep, *l.pos, *l.time);
}
