#pragma once
#include "../../core/DataModel.hpp"
#include "../../core/JSONAdapter.hpp"
#include "../../core/PayloadContracts.hpp"
#include "../../core/EnumTraits.hpp"
#include "../Hub.hpp"
#include "../hazards/HazardDetection.hpp"

class Hub;

// handlers.h
namespace handlers {
  
  void handleHealthPayload(const Message& msg, Hub& hub);
  void handleMotionPayload(const Message& msg, Hub& hub);
  void handleFirePayload(const Message& msg, Hub& hub);
  void handlePIRPayload(const Message& msg, Hub& hub);
  void handleCommand(const Message& msg, Hub& hub);
}