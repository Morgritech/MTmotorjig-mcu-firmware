// Copyright (C) 2025 Morgritech
//
// Licensed under GNU General Public License v3.0 (GPLv3) License.
// See the LICENSE file in the project root for full license details.

/// @file input_manager.cpp
/// @brief Class that handles user input (buttons, serial, etc.).

#include "input_manager.h"

#include <Arduino.h>
#include <ArduinoLog.h>

#include "configuration.h"

namespace mtmotor_jig {

InputManager::InputManager() {}

InputManager::~InputManager() {}

void InputManager::Begin() {
  encoder_button_.set_long_press_option(configuration_.kLongPressOption_);
  controller_button_.set_long_press_option(configuration_.kLongPressOption_);
  limit_switch_.set_long_press_option(configuration_.kLongPressOption_);
}

Configuration::ControlAction InputManager::Check(Configuration::ControlMode control_mode) {
  Configuration::ControlAction control_action = Configuration::ControlAction::kIdle;

  // Check for encoder dial rotation.
  mt::RotaryEncoder::RotationDirection rotation_direction = encoder_dial_.DetectRotation();
  // Check for button presses.
  mt::MomentaryButton::PressType encoder_button_press_type = encoder_button_.DetectPressType();
  mt::MomentaryButton::PressType controller_button_press_type = controller_button_.DetectPressType();
  mt::MomentaryButton::PressType limit_switch_press_type = limit_switch_.DetectPressType();

  // Process the encoder dial rotation, button presses, and serial input (one character at a time).
  if (rotation_direction == mt::RotaryEncoder::RotationDirection::kPositive) {
    control_action = Configuration::ControlAction::kSelectNext;
    Log.noticeln(F("Encoder dial clockwise rotation"));
  } 
  else if (rotation_direction == mt::RotaryEncoder::RotationDirection::kNegative) {
    control_action = Configuration::ControlAction::kSelectPrevious;
    Log.noticeln(F("Encoder dial counter-clockwise rotation"));
  }
  else if (controller_button_press_type == mt::MomentaryButton::PressType::kShortPress) {
    control_action = Configuration::ControlAction::kCycleSpeed;
    Log.noticeln(F("Controller button short press"));
  }
  else if (encoder_button_press_type == mt::MomentaryButton::PressType::kShortPress) {
    switch (control_mode) {
      case Configuration::ControlMode::kContinuousMenu: {
        control_action = Configuration::ControlAction::kToggleDirection;
        break;
      }
      case Configuration::ControlMode::kOscillateMenu: {
        control_action = Configuration::ControlAction::kCycleAngle;
        break;
      }
    }

    Log.noticeln(F("Encoder button short press"));
  }
  else if (controller_button_press_type == mt::MomentaryButton::PressType::kLongPress
           || encoder_button_press_type == mt::MomentaryButton::PressType::kLongPress) {
    control_action = Configuration::ControlAction::kToggleMotion;
    Log.noticeln(F("Button long press"));
  }
  else if (limit_switch_press_type == mt::MomentaryButton::PressType::kShortPress) {
    control_action = Configuration::ControlAction::kResetHome;
    Log.noticeln(F("Limit switch short press"));
  }
  else if (limit_switch_press_type == mt::MomentaryButton::PressType::kLongPress) {
    control_action = Configuration::ControlAction::kGoHome;
    Log.noticeln(F("Limit switch long press"));
  }
  else if (MTMOTOR_JIG_SERIAL.available() > 0) {
    char serial_input = MTMOTOR_JIG_SERIAL.read();
    control_action = static_cast<Configuration::ControlAction>(serial_input);
    Log.noticeln(F("Serial input: %c"), serial_input);
  }
  else {
    control_action = Configuration::ControlAction::kIdle;
  }

  return control_action;
}

} // namespace mtmotor_jig