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

#include "SimpleRCDelayCalc.hh"
#include <cassert>

#include "DcalcAnalysisPt.hh"
#include "Liberty.hh"
#include "Network.hh"
#include "Parasitics.hh"
#include "TimingArc.hh"

namespace sta {

ArcDelayCalc *
makeSimpleRCDelayCalc(StaState *sta)
{
  return new SimpleRCDelayCalc(sta);
}

SimpleRCDelayCalc::SimpleRCDelayCalc(StaState *sta) :
  LumpedCapDelayCalc(sta)
{
}

ArcDelayCalc *
SimpleRCDelayCalc::copy()
{
  return new SimpleRCDelayCalc(this);
}

ArcDcalcResult
SimpleRCDelayCalc::inputPortDelay(const Pin *port_pin,
                                  float in_slew,
                                  const RiseFall *rf,
                                  const Parasitic *parasitic,
                                  const LoadPinIndexMap &load_pin_index_map,
                                  const DcalcAnalysisPt *dcalc_ap)
{
  ArcDcalcResult &&res = LumpedCapDelayCalc::inputPortDelay(port_pin, in_slew, rf, parasitic, load_pin_index_map, dcalc_ap);
  LibertyLibrary *lib = network_->defaultLibertyLibrary();
  seedLoadsDelaySlew(parasitic, lib, rf, load_pin_index_map, res);
  return res;
}

ArcDcalcResult
SimpleRCDelayCalc::gateDelay(const Pin *drvr_pin,
                             const TimingArc *arc,
                             const Slew &in_slew,
                             float load_cap,
                             const Parasitic *drvr_parasitic,
                             const LoadPinIndexMap &load_pin_index_map,
                             const DcalcAnalysisPt *dcalc_ap)
{
  ArcDcalcResult &&res = LumpedCapDelayCalc::gateDelay(drvr_pin, arc, in_slew, load_cap, drvr_parasitic, load_pin_index_map, dcalc_ap);
  seedLoadsDelaySlew(drvr_parasitic, arc, load_pin_index_map, res);
  return res;
}

void
SimpleRCDelayCalc::gateDelay(const TimingArc *,
                             const Slew &,
                             float,
                             const Parasitic *,
                             float,
                             const Pvt *,
                             const DcalcAnalysisPt *,
                             ArcDelay &,
                             Slew &)
{
  assert(0);
}

void
SimpleRCDelayCalc::seedLoadsDelaySlew(const Parasitic *drvr_parasitic,
                                      const TimingArc *arc,
                                      const LoadPinIndexMap &loads,
                                      ArcDcalcResult &result)
{
  LibertyLibrary *lib = arc->to()->libertyLibrary();
  RiseFall *rf = arc->toEdge()->asRiseFall();
  seedLoadsDelaySlew(drvr_parasitic, lib, rf, loads, result);
}

void
SimpleRCDelayCalc::seedLoadsDelaySlew(const Parasitic *drvr_parasitic,
                                      const LibertyLibrary *lib,
                                      const RiseFall *rf,
                                      const LoadPinIndexMap &loads,
                                      ArcDcalcResult &result)
{
  Slew drvr_slew = result.drvrSlew();
  for (auto [p, ind] : loads)
    seedLoadDelaySlew(p, drvr_slew, drvr_parasitic, lib, rf, ind, result);
}

void
SimpleRCDelayCalc::seedLoadDelaySlew(const Pin *load_pin,
                                     Slew drvr_slew,
                                     const Parasitic *drvr_parasitic,
                                     const LibertyLibrary *lib,
                                     const RiseFall *drvr_rf,
                                     uint64_t pin_index,
                                     ArcDcalcResult &result)
{
  ArcDelay wire_delay = 0.0;
  Slew load_slew = drvr_slew;
  bool elmore_exists = false;
  float elmore = 0.0;
  if (drvr_parasitic)
    parasitics_->findElmore(drvr_parasitic, load_pin, elmore, elmore_exists);
  if (elmore_exists) {
    if (lib && lib->wireSlewDegradationTable(drvr_rf)) {
      wire_delay = elmore;
      load_slew = lib->degradeWireSlew(drvr_rf,
                                       delayAsFloat(drvr_slew),
                                       delayAsFloat(wire_delay));
    }
    else if (parasitics_->isReducedParasiticNetwork(drvr_parasitic)) {
      assert(thresholdLibrary(load_pin) && "Current We Always Assume Lib Exists");
      dspfWireDelaySlew(load_pin, drvr_rf, drvr_slew, elmore, wire_delay, load_slew);
    }
    else {
      wire_delay = elmore;
      load_slew = drvr_slew;
    }
  }
  thresholdAdjust(load_pin, lib, drvr_rf, wire_delay, load_slew);
  result.setLoadSlew(pin_index, load_slew);
  result.setWireDelay(pin_index, wire_delay);
}

}  // namespace sta
