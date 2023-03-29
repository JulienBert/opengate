/* --------------------------------------------------
   Copyright (C): OpenGATE Collaboration
   This software is distributed under the terms
   of the GNU Lesser General  Public Licence (LGPL)
   See LICENSE.md for further details
   -------------------------------------------------- */

#ifndef GateTDigiAttribute_h
#define GateTDigiAttribute_h

#include "../GateHelpers.h"
#include "../GateUniqueVolumeID.h"
#include "GateVDigiAttribute.h"
#include <pybind11/stl.h>

template <class T> class GateTDigiAttribute : public GateVDigiAttribute {
public:
  explicit inline GateTDigiAttribute(std::string vname);

  inline ~GateTDigiAttribute() override;

  virtual inline int GetSize() const override;

  virtual inline std::vector<double> &GetDValues() override;

  virtual inline std::vector<int> &GetIValues() override;

  virtual inline std::vector<std::string> &GetSValues() override;

  virtual inline std::vector<G4ThreeVector> &Get3Values() override;

  virtual inline std::vector<GateUniqueVolumeID::Pointer> &
  GetUValues() override;

  const inline std::vector<T> &GetValues() const;

  virtual inline void FillToRoot(size_t index) const override;

  virtual inline void FillDValue(double v) override;

  virtual inline void FillSValue(std::string v) override;

  virtual inline void FillIValue(int v) override;

  virtual inline void Fill3Value(G4ThreeVector v) override;

  virtual inline void FillUValue(GateUniqueVolumeID::Pointer v) override;

  virtual inline void Fill(GateVDigiAttribute *input, size_t index) override;

  virtual inline void FillDigiWithEmptyValue() override;

  virtual inline void Clear() override;

  virtual inline std::string Dump(int i) const override;

protected:
  struct threadLocal_t {
    std::vector<T> fValues;
  };
  G4Cache<threadLocal_t> threadLocalData;

  void InitDefaultProcessHitsFunction();
};

#include "GateTDigiAttribute.icc"

#endif // GateTDigiAttribute_h
