// OpenSTA, Static Timing Analyzer
// Copyright (c) 2022, Parallax Software, Inc.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "LumpedCapDelayCalc.hh"

namespace sta {

// Liberty table model lumped capacitance arc delay calculator.
// Effective capacitance is the pi model total capacitance (C1+C2).
// Wire delays are elmore delays.
// Driver slews are degraded to loads by rise/fall transition_degradation
// tables.
class SimpleRCDelayCalc : public LumpedCapDelayCalc
{
public:
  SimpleRCDelayCalc(StaState *sta);
  ArcDelayCalc *copy() override;
  ArcDcalcResult inputPortDelay(const Pin *port_pin,
                                float in_slew,
                                const RiseFall *rf,
                                const Parasitic *parasitic,
                                const LoadPinIndexMap &load_pin_index_map,
                                const DcalcAnalysisPt *dcalc_ap) override;
  ArcDcalcResult gateDelay(const Pin *drvr_pin,
                           const TimingArc *arc,
                           const Slew &in_slew,
                           float load_cap,
                           const Parasitic *drvr_parasitic,
                           const LoadPinIndexMap &load_pin_index_map,
                           const DcalcAnalysisPt *dcalc_ap) override;
  void gateDelay(const TimingArc *arc,
                 const Slew &in_slew,
                 float load_cap,
                 const Parasitic *parasitic,
                 float related_out_cap,
                 const Pvt *pvt,
                 const DcalcAnalysisPt *dcalc_ap,
                 // Return values.
                 ArcDelay &gate_delay,
                 Slew &drvr_slew) override;
  virtual void seedLoadsDelaySlew(const Parasitic *drvr_parasitic,
                                  const TimingArc *arc,
                                  const LoadPinIndexMap &loads,
                                  ArcDcalcResult &result);
  virtual void seedLoadsDelaySlew(const Parasitic *drvr_parasitic,
                                  const LibertyLibrary *lib,
                                  const RiseFall *rf,
                                  const LoadPinIndexMap &loads,
                                  ArcDcalcResult &result);
  virtual void seedLoadDelaySlew(const Pin *load_pin,
                                 Slew drvr_slew,
                                 const Parasitic *drvr_parasitic,
                                 const LibertyLibrary *lib,
                                 const RiseFall *drvr_rf,
                                 uint64_t pin_index,
                                 ArcDcalcResult &result);
};

ArcDelayCalc *
makeSimpleRCDelayCalc(StaState *sta);

}  // namespace sta
