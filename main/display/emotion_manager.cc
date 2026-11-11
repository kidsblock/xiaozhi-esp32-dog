// Emotion Manager Implementation
#include "emotion_manager.h"

Emotion get_emotion_for_state(DeviceState state) {
    switch (state) {
        case kDeviceStateIdle:              return EMOJI_HAPPY;
        case kDeviceStateListening:         return EMOJI_EXCITED;
        case kDeviceStateSpeaking:          return EMOJI_SPEAKING;
        case kDeviceStateConnecting:        return EMOJI_THINKING;
        case kDeviceStateStarting:          return EMOJI_SQUINT;
        case kDeviceStateWifiConfiguring:   return EMOJI_SONFUSED;
        case kDeviceStateUpgrading:         return EMOJI_THINKING;
        case kDeviceStateFatalError:        return EMOJI_SAD;
        case kDeviceStateUnknown:            return EMOJI_HAPPY;
        default:                             return EMOJI_HAPPY;
    }
}

Emotion get_emotion_for_pet_action(int action_id) {
    switch (action_id) {
        case 7:  return EMOJI_EXCITED; // wave
        case 1:  return EMOJI_SQUINT;  // sleep
        case 0:  return EMOJI_HAPPY;   // walk
        case 4:  return EMOJI_HAPPY;   // walk back
        case 5:  return EMOJI_HAPPY;   // turn left
        case 6:  return EMOJI_HAPPY;   // turn right
        case 10: return EMOJI_SONFUSED;
        case 11: return EMOJI_LOVE;
        default: return EMOJI_HAPPY;
    }
}
