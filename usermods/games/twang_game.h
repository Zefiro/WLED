// twang_game.h
#pragma once
#include <stdint.h>

void twangInit(uint16_t ledCount);
void twangStep(float joystickValue, bool attackPressed, uint32_t nowMs);
void twangRender(uint8_t *r, uint8_t *g, uint8_t *b, uint16_t ledCount);
bool twangIsRunning();
void twangStartNewGame();
