//	LifeGame.h
//	4simb

#pragma once

// default path
#define PATH "img.bmp"

// cells
#define EMPTY 0
#define ALIVE 1
#define WALL 2

// special keys
#define KEY_SPACE 32
#define KEY_ESC 27
#define KEY_ENTER 13

// default functions
template<typename T>
float sum(T const& xs) {
	float s = 0.f;
	for (auto const& x : xs) {
		s += x;
	}
	return s;
}

float map(float x, float in_min, float in_max, float out_min, float out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float crdX(float coord, int _winWidth) {
	return map(coord, 0, _winWidth, -1, 1);
}

float crdY(float coord, int _winHeight) {
	return map(coord, 0, _winHeight, -1, 1);
}