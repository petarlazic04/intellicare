#pragma once
#include "../../core/DataModel.hpp"


namespace hazards{

  bool isCriticalHealth(const HealthData& vitals);
  bool isFireHazard(const FireDetectorData& fire);
  bool isFall(const MotionData& motion);
  bool areVitalsStale(long lastVitalsTimestamp);
  bool shouldProcessFall(long lastFallTimestamp, long currentTimestamp);

}


bool hazards::isCriticalHealth(const HealthData& vitals){
   return vitals.heartRate < CRITICAL_HEART_RATE_LOW ||
           vitals.heartRate > CRITICAL_HEART_RATE_HIGH ||
           vitals.spo2 < CRITICAL_SPO2_LOW ||
           vitals.systolic < CRITICAL_SYSTOLIC_LOW ||   
           vitals.systolic > CRITICAL_SYSTOLIC_HIGH ||
           vitals.diastolic < CRITICAL_DIASTOLIC_LOW || 
           vitals.diastolic > CRITICAL_DIASTOLIC_HIGH;
}

bool hazards::isFall(const MotionData& motion){
  return motion.magnitude > FALL_MAGNITUDE_THRESHOLD;
}

bool hazards::isFireHazard(const FireDetectorData& fire){
  return fire.temperature > FIRE_TEMP_THRESHOLD || 
         fire.smokeLevel > FIRE_SMOKE_THRESHOLD ||
         fire.coLevel > FIRE_CO_THRESHOLD;
}

bool hazards::shouldProcessFall(long lastFallTimestamp, long currentTimestamp){
  return (currentTimestamp - lastFallTimestamp) >= FALL_DEBOUNCE_MS;
}