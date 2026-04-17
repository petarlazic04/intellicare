# IntelliCare - Comprehensive Test Scenarios Documentation

## Overview
This document describes 25+ test scenarios designed to validate system functionality across the full range of operating conditions, including edge cases, boundary conditions, and combined emergency scenarios.

## Category 1: Normal Operation

### 1. normal_operation.json
**Purpose**: Baseline testing - verify system operates correctly with all sensors in normal ranges
- All rooms at normal temperature (20-25°C)
- No smoke or CO detected
- No motion detected
- Wristband vitals normal (HR: 72, SpO2: 98, BP: 120/80)
- All actuators inactive
- Door locked
- **Expected Outcome**: System should remain idle, no alerts

---

## Category 2: Fire/Hazard Detection

### 2. extreme_fire_temperature.json
**Purpose**: Test maximum temperature scenario - living room fire at extreme levels
- Temperature: 155.8°C (exceeds typical sensor max)
- Smoke: 999.0 ppm (maximum detected)
- CO: 500 ppm (critical level)
- **Expected Outcome**: Emergency alert, sprinklers activate, lights alert, family notified

### 3. smoke_only.json
**Purpose**: Test slow burn/smoldering fire detection
- Temperature: 45.5°C (warm but not extreme)
- Smoke: 350.0 ppm (moderate-high)
- CO: 15 ppm (low)
- Motion: Detected
- **Expected Outcome**: Warning alert, increased monitoring

### 4. high_co_level.json
**Purpose**: Test carbon monoxide poisoning scenario
- Temperature: 28.0°C (ambient)
- Smoke: 50.0 ppm (low)
- CO: 200 ppm (critical)
- **Expected Outcome**: CO alert, ambulance consideration

### 5. multi_room_fire.json
**Purpose**: Test worst-case scenario - fire spreading through multiple rooms
- Kitchen: 95°C, 450 ppm smoke, 80 ppm CO
- Living room: 82°C, 380 ppm smoke, 60 ppm CO
- Hallway: 70°C, 200 ppm smoke, 35 ppm CO
- **Expected Outcome**: Full emergency protocol, all sprinklers activate, evacuation alerts

### 6. extreme_temperature_room.json
**Purpose**: Test high temperature from non-fire source (HVAC malfunction, weather)
- Temperature: 65°C (high but plausible non-fire heat)
- Smoke: 5.0 ppm (minimal)
- CO: 2 ppm (negligible)
- **Expected Outcome**: Warning only, no fire emergency

---

## Category 3: Health Vitals - Individual Parameters

### 7. minimum_vitals.json
**Purpose**: Test critically LOW vital signs
- Heart Rate: 35 BPM (severe bradycardia)
- SpO2: 82% (critical hypoxemia)
- Systolic: 75 mmHg (shock range)
- Diastolic: 45 mmHg
- **Expected Outcome**: Ambulance alert, fall risk assessment

### 8. maximum_vitals.json
**Purpose**: Test critically HIGH vital signs
- Heart Rate: 180 BPM (severe tachycardia)
- SpO2: 85% (hypoxemia)
- Systolic: 210 mmHg (hypertensive crisis)
- Diastolic: 130 mmHg
- **Expected Outcome**: Cardiac emergency alert, ambulance + family notification

### 9. low_oxygen_saturation.json
**Purpose**: Test respiratory crisis (SpO2 focus)
- SpO2: 78% (critical respiratory distress)
- Heart Rate: 105 BPM (compensatory tachycardia)
- Blood Pressure: Normal
- **Expected Outcome**: Respiratory emergency, oxygen saturation monitoring

### 10. rapid_heart_rate.json
**Purpose**: Test extreme tachycardia without fall
- Heart Rate: 165 BPM (sustained tachycardia)
- SpO2: 96% (normal)
- Blood Pressure: 148/95 (elevated but responsive)
- Motion: Detected (normal activity)
- **Expected Outcome**: Cardiac monitoring, possible stress/panic response

### 11. slow_heart_rate.json
**Purpose**: Test extreme bradycardia (heart block risk)
- Heart Rate: 38 BPM (critical bradycardia)
- SpO2: 97% (adequate)
- Blood Pressure: Normal
- **Expected Outcome**: Cardiac emergency alert

### 12. high_blood_pressure.json
**Purpose**: Test Stage 3 hypertension (emergency level)
- Systolic: 220 mmHg (hypertensive crisis)
- Diastolic: 140 mmHg
- Heart Rate: 95 BPM
- **Expected Outcome**: Cardiac emergency, stroke risk assessment

### 13. low_blood_pressure.json
**Purpose**: Test hypotensive shock risk
- Systolic: 78 mmHg (shock range)
- Diastolic: 42 mmHg
- Heart Rate: 110 BPM (compensatory tachycardia)
- **Expected Outcome**: Shock alert, emergency response

### 14. oxygen_saturation_boundaries.json
**Purpose**: Test moderate hypoxemia (boundary conditions)
- SpO2: 89% (critical but not extreme)
- Heart Rate: 98 BPM
- Blood Pressure: Normal
- **Expected Outcome**: Respiratory alert, supplemental oxygen consideration

### 15. asymmetric_blood_pressure.json
**Purpose**: Test measurement anomalies or severe shock indicators
- Systolic: 210 mmHg (high)
- Diastolic: 50 mmHg (critically low - unusual ratio)
- Heart Rate: 115 BPM
- **Expected Outcome**: Shock/measurement error alert for validation

---

## Category 4: Fall Detection

### 16. fall_with_panic.json
**Purpose**: Test fall with stress response (elevated vitals from panic rather than injury)
- Acceleration magnitude: 8.8 m/s² (fall pattern)
- Gyro rotation: Detected (rolling/tumbling)
- Heart Rate: 145 BPM (panic response)
- Blood Pressure: 165/105 (elevated from stress)
- Location: Bathroom
- **Expected Outcome**: Fall alert + cardiac stress monitoring

### 17. fall_immobile_critical_vitals.json
**Purpose**: Test severe fall with inability to move and critical vitals
- Acceleration: Minimal after impact (suggests immobility)
- Heart Rate: 42 BPM (severe bradycardia from shock)
- SpO2: 88% (hypoxemia)
- Blood Pressure: Low (88/52 shock range)
- **Expected Outcome**: CRITICAL alert - immediate ambulance + family + emergency responders

### 18. no_motion_high_vitals.json
**Purpose**: Test stationary person with cardiac distress (bedridden)
- PIR Motion: NOT detected
- Heart Rate: 140 BPM (tachycardia)
- Blood Pressure: 175/110 (hypertension)
- **Expected Outcome**: Cardiac emergency alert despite no fall, assess immobility risk

---

## Category 5: Combined/Complex Scenarios

### 19. combined_fire_health_crisis.json
**Purpose**: Test WORST CASE - simultaneous fire emergency and cardiac event
- Temperature: 105°C, 550 ppm smoke, 150 ppm CO (major fire)
- Heart Rate: 170 BPM (cardiac distress)
- SpO2: 82% (critical from both fire smoke + cardiac)
- Blood Pressure: 200/125 (hypertensive crisis)
- **Expected Outcome**: Dual emergency protocol - activate all systems, immediate evacuation + ambulance

### 20. rapid_condition_change.json
**Purpose**: Test sudden collapse/stroke scenario
- Initial: Person moving, then sudden deterioration
- Acceleration: High magnitude (~12.5 m/s²) - fall pattern
- Heart Rate: 55 BPM (post-collapse bradycardia)
- SpO2: 80% (hypoxemia)
- Blood Pressure: 95/50 (shock)
- **Expected Outcome**: Emergency protocol, immediate ambulance

### 21. recovery_sequence.json
**Purpose**: Test system behavior during recovery after emergency
- Vitals returning to normal
- Motion resumed
- Lights activated
- Door unlocked (if safe)
- **Expected Outcome**: Alert cancellation, transition to monitoring state

---

## Category 6: Sensor Reliability & Edge Cases

### 22. false_motion_detection.json
**Purpose**: Test PIR sensor false alarms
- Multiple rooms: Motion detected
- All hazard sensors: Normal
- Vitals: Normal
- **Expected Outcome**: Log false alarms, no emergency response, motion tracking only

### 23. zero_values_sensor_failure.json
**Purpose**: Test potential sensor malfunction (all zero readings)
- All sensors reporting 0 or null
- Could indicate sensor failure or initialization state
- **Expected Outcome**: Sensor health alert, request recalibration

### 24. edge_case_normal_boundaries.json
**Purpose**: Test exactly at threshold boundaries
- Temperature: 30°C (high normal)
- Smoke: 100.0 ppm (lower warning threshold)
- CO: 50 ppm (low concern)
- Heart Rate: 60 BPM (lower normal)
- **Expected Outcome**: No alert, system operates normally at boundaries

### 25. stress_test_load.json
**Purpose**: Performance test - all sensors reporting simultaneously
- 10 rooms/zones active
- Multiple alert-level values
- Wristband + environmental sensors combined
- **Expected Outcome**: All processes complete within response time, no data loss

---

## Medical Reference Ranges Used in Tests

### Heart Rate (BPM)
- Normal: 60-100
- Tachycardia (mild): 100-120
- Tachycardia (severe): >150
- **Critical**: <40 or >180

### SpO2 (Oxygen Saturation %)
- Normal: >95%
- Mild hypoxemia: 90-94%
- Moderate hypoxemia: 88-89%
- **Critical**: <88%

### Blood Pressure (mmHg)
- Normal: 120/80
- Elevated: 130-139 / 80-89
- Stage 2 Hypertension: 140-159 / 90-99
- Hypertensive Crisis: >180 / >120
- Hypotension: <90 / <60
- **Critical Shock**: <70 / <40

### Temperature (°C)
- Normal room: 20-24
- Warm (non-fire): 25-40
- Warning zone: 40-60
- Fire risk: 60-90
- **Critical fire**: >90

### Smoke Level (ppm)
- Normal: 0-5
- Light: 5-50
- Moderate: 50-200
- Heavy: 200-500
- **Critical**: >500

### CO Level (ppm, 8-hour TWA)
- Safe: <9
- Caution: 9-35
- Warning: 35-400
- **Danger**: >400

---

## Testing Workflow Recommendation

### Phase 1: Baseline (Run 1-6)
Test basic functionality, individual sensor response

### Phase 2: Individual Parameters (Run 7-15)
Test each vital sign parameter at critical levels

### Phase 3: Fall Detection (Run 16-18)
Test fall detection with various vital conditions

### Phase 4: Complex Scenarios (Run 19-21)
Test combined emergencies and recovery

### Phase 5: Reliability (Run 22-25)
Test edge cases and sensor health

---

## Success Criteria Per Category

**Fire Detection**: Alert triggered within 2 second, correct room identified
**Health Monitoring**: Threshold detection within 1 second, correct alert level
**Fall Detection**: Fall confirmed within 3 seconds with motion + acceleration pattern
**Multi-Emergency**: All hazard responses activated without conflicts
**Sensor Health**: Invalid data logged, system continues operating safely

