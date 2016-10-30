#pragma once

#include "cVec.h"
#include "cMat.h"
#include <string>

#define SAFE_DELETE(x) do{delete x; x = nullptr;} while(0);
#define SAFE_DELETE_array(x) do{delete[] x; x = nullptr;} while(0);

#define E_PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062

const float pi = (float)E_PI;
const float pi_2 = (float)(E_PI * 2.0);
const float pi_d2 = (float)(E_PI * 0.5);
const float pi_3d2 = (float)(E_PI * 1.5);

const double d_pi = E_PI;
const double d_pi_2 = (E_PI * 2.0);
const double d_pi_d2 = (E_PI * 0.5);
const double d_pi_3d2 = (E_PI * 1.5);

template <class T>
inline const T& min(const T& a, const T& b)
{
	return a < b ? a : b;
}


template <class T>
inline const T& min(const T& a, const T& b, const T& c)
{
	return a < b ? min(a, c) : min(b, c);
}

template <class T>
inline const T& max(const T& a, const T& b)
{
	return a > b ? a : b;
}


template <class T>
inline const T& max(const T& a, const T& b, const T& c)
{
	return a > b ? max(a, c) : max(b, c);
}


template <class T>
inline const T& abs(const T& a)
{
	return a > 0 ? a : -a;
}


bool textFileRead(const char *fileName, std::string &data);

void sleepMS(int ms);