// Emotion Manager - Maps device states to emotions
#pragma once
#include "emotion_bitmaps.h"
#include "../application.h"
#ifdef __cplusplus
extern "C" {
#endif
Emotion get_emotion_for_state(DeviceState state);
Emotion get_emotion_for_pet_action(int action_id);
#ifdef __cplusplus
}
#endif
