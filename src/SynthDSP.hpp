//***********************************************************************************************
// TwinSlide DSP Components
// Ported verbatim from Slide (Open303 by Robin Schmidt / RS-MET / ROSIC)
// Filter coefficients from mystran (Teemu Voipio) and kunn
// Wavetable oscillator from Open303 MipMappedWaveTable + BlendOscillator
//***********************************************************************************************

#pragma once

#include "rack.hpp"
#include <cmath>
#include <cstring>
#include <cfloat>

using namespace rack;


// ============================================================================
// Ooura FFT - Minimal implementation for wavetable generation
// From fft4g.c by Takuya OOURA (public domain)
// ============================================================================

namespace OouraFFT {

static void bitrv2(int n, int *ip, double *a);

static void makewt(int nw, int *ip, double *w) {
	int j, nwh;
	double delta, x, y;
	ip[0] = nw;
	ip[1] = 1;
	if (nw > 2) {
		nwh = nw >> 1;
		delta = std::atan(1.0) / nwh;
		w[0] = 1;
		w[1] = 0;
		w[nwh] = std::cos(delta * nwh);
		w[nwh + 1] = w[nwh];
		if (nwh > 2) {
			for (j = 2; j < nwh; j += 2) {
				x = std::cos(delta * j);
				y = std::sin(delta * j);
				w[j] = x;
				w[j + 1] = y;
				w[nw - j] = y;
				w[nw - j + 1] = x;
			}
			bitrv2(nw, ip + 2, w);
		}
	}
}

static void makect(int nc, int *ip, double *c) {
	int j, nch;
	double delta;
	ip[1] = nc;
	if (nc > 1) {
		nch = nc >> 1;
		delta = std::atan(1.0) / nch;
		c[0] = std::cos(delta * nch);
		c[nch] = 0.5 * c[0];
		for (j = 1; j < nch; j++) {
			c[j] = 0.5 * std::cos(delta * j);
			c[nc - j] = 0.5 * std::sin(delta * j);
		}
	}
}

static void bitrv2(int n, int *ip, double *a) {
	int j, j1, k, k1, l, m, m2;
	double xr, xi, yr, yi;
	ip[0] = 0;
	l = n;
	m = 1;
	while ((m << 3) < l) {
		l >>= 1;
		for (j = 0; j < m; j++) ip[m + j] = ip[j] + l;
		m <<= 1;
	}
	m2 = 2 * m;
	if ((m << 3) == l) {
		for (k = 0; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
				j1 += m2; k1 += 2 * m2;
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
				j1 += m2; k1 -= m2;
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
				j1 += m2; k1 += 2 * m2;
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
			}
			j1 = 2 * k + m2 + ip[k];
			k1 = j1 + m2;
			xr = a[j1]; xi = a[j1 + 1];
			yr = a[k1]; yi = a[k1 + 1];
			a[j1] = yr; a[j1 + 1] = yi;
			a[k1] = xr; a[k1 + 1] = xi;
		}
	} else {
		for (k = 1; k < m; k++) {
			for (j = 0; j < k; j++) {
				j1 = 2 * j + ip[k];
				k1 = 2 * k + ip[j];
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
				j1 += m2; k1 += m2;
				xr = a[j1]; xi = a[j1 + 1];
				yr = a[k1]; yi = a[k1 + 1];
				a[j1] = yr; a[j1 + 1] = yi;
				a[k1] = xr; a[k1 + 1] = xi;
			}
		}
	}
}

static void cft1st(int n, double *a, double *w) {
	int j, k1, k2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	x0r = a[0] + a[2]; x0i = a[1] + a[3];
	x1r = a[0] - a[2]; x1i = a[1] - a[3];
	x2r = a[4] + a[6]; x2i = a[5] + a[7];
	x3r = a[4] - a[6]; x3i = a[5] - a[7];
	a[0] = x0r + x2r; a[1] = x0i + x2i;
	a[4] = x0r - x2r; a[5] = x0i - x2i;
	a[2] = x1r - x3i; a[3] = x1i + x3r;
	a[6] = x1r + x3i; a[7] = x1i - x3r;
	wk1r = w[2];
	x0r = a[8] + a[10]; x0i = a[9] + a[11];
	x1r = a[8] - a[10]; x1i = a[9] - a[11];
	x2r = a[12] + a[14]; x2i = a[13] + a[15];
	x3r = a[12] - a[14]; x3i = a[13] - a[15];
	a[8] = x0r + x2r; a[9] = x0i + x2i;
	a[12] = x2i - x0i; a[13] = x0r - x2r;
	x0r = x1r - x3i; x0i = x1i + x3r;
	a[10] = wk1r * (x0r - x0i); a[11] = wk1r * (x0r + x0i);
	x0r = x3i + x1r; x0i = x3r - x1i;
	a[14] = wk1r * (x0i - x0r); a[15] = wk1r * (x0i + x0r);
	k1 = 0;
	for (j = 16; j < n; j += 16) {
		k1 += 2; k2 = 2 * k1;
		wk2r = w[k1]; wk2i = w[k1 + 1];
		wk1r = w[k2]; wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		x0r = a[j] + a[j + 2]; x0i = a[j + 1] + a[j + 3];
		x1r = a[j] - a[j + 2]; x1i = a[j + 1] - a[j + 3];
		x2r = a[j + 4] + a[j + 6]; x2i = a[j + 5] + a[j + 7];
		x3r = a[j + 4] - a[j + 6]; x3i = a[j + 5] - a[j + 7];
		a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
		x0r -= x2r; x0i -= x2i;
		a[j + 4] = wk2r * x0r - wk2i * x0i;
		a[j + 5] = wk2r * x0i + wk2i * x0r;
		x0r = x1r - x3i; x0i = x1i + x3r;
		a[j + 2] = wk1r * x0r - wk1i * x0i;
		a[j + 3] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i; x0i = x1i - x3r;
		a[j + 6] = wk3r * x0r - wk3i * x0i;
		a[j + 7] = wk3r * x0i + wk3i * x0r;
		wk1r = w[k2 + 2]; wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		x0r = a[j + 8] + a[j + 10]; x0i = a[j + 9] + a[j + 11];
		x1r = a[j + 8] - a[j + 10]; x1i = a[j + 9] - a[j + 11];
		x2r = a[j + 12] + a[j + 14]; x2i = a[j + 13] + a[j + 15];
		x3r = a[j + 12] - a[j + 14]; x3i = a[j + 13] - a[j + 15];
		a[j + 8] = x0r + x2r; a[j + 9] = x0i + x2i;
		x0r -= x2r; x0i -= x2i;
		a[j + 12] = -wk2i * x0r - wk2r * x0i;
		a[j + 13] = -wk2i * x0i + wk2r * x0r;
		x0r = x1r - x3i; x0i = x1i + x3r;
		a[j + 10] = wk1r * x0r - wk1i * x0i;
		a[j + 11] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i; x0i = x1i - x3r;
		a[j + 14] = wk3r * x0r - wk3i * x0i;
		a[j + 15] = wk3r * x0i + wk3i * x0r;
	}
}

static void cftmdl(int n, int l, double *a, double *w) {
	int j, j1, j2, j3, k, k1, k2, m, m2;
	double wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	m = l << 2;
	for (j = 0; j < l; j += 2) {
		j1 = j + l; j2 = j1 + l; j3 = j2 + l;
		x0r = a[j] + a[j1]; x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1]; x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
		a[j2] = x0r - x2r; a[j2 + 1] = x0i - x2i;
		a[j1] = x1r - x3i; a[j1 + 1] = x1i + x3r;
		a[j3] = x1r + x3i; a[j3 + 1] = x1i - x3r;
	}
	wk1r = w[2];
	for (j = m; j < l + m; j += 2) {
		j1 = j + l; j2 = j1 + l; j3 = j2 + l;
		x0r = a[j] + a[j1]; x0i = a[j + 1] + a[j1 + 1];
		x1r = a[j] - a[j1]; x1i = a[j + 1] - a[j1 + 1];
		x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
		x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
		a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
		a[j2] = x2i - x0i; a[j2 + 1] = x0r - x2r;
		x0r = x1r - x3i; x0i = x1i + x3r;
		a[j1] = wk1r * (x0r - x0i); a[j1 + 1] = wk1r * (x0r + x0i);
		x0r = x3i + x1r; x0i = x3r - x1i;
		a[j3] = wk1r * (x0i - x0r); a[j3 + 1] = wk1r * (x0i + x0r);
	}
	k1 = 0; m2 = 2 * m;
	for (k = m2; k < n; k += m2) {
		k1 += 2; k2 = 2 * k1;
		wk2r = w[k1]; wk2i = w[k1 + 1];
		wk1r = w[k2]; wk1i = w[k2 + 1];
		wk3r = wk1r - 2 * wk2i * wk1i;
		wk3i = 2 * wk2i * wk1r - wk1i;
		for (j = k; j < l + k; j += 2) {
			j1 = j + l; j2 = j1 + l; j3 = j2 + l;
			x0r = a[j] + a[j1]; x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1]; x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
			x0r -= x2r; x0i -= x2i;
			a[j2] = wk2r * x0r - wk2i * x0i;
			a[j2 + 1] = wk2r * x0i + wk2i * x0r;
			x0r = x1r - x3i; x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i; x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
		wk1r = w[k2 + 2]; wk1i = w[k2 + 3];
		wk3r = wk1r - 2 * wk2r * wk1i;
		wk3i = 2 * wk2r * wk1r - wk1i;
		for (j = k + m; j < l + (k + m); j += 2) {
			j1 = j + l; j2 = j1 + l; j3 = j2 + l;
			x0r = a[j] + a[j1]; x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1]; x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
			x0r -= x2r; x0i -= x2i;
			a[j2] = -wk2i * x0r - wk2r * x0i;
			a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
			x0r = x1r - x3i; x0i = x1i + x3r;
			a[j1] = wk1r * x0r - wk1i * x0i;
			a[j1 + 1] = wk1r * x0i + wk1i * x0r;
			x0r = x1r + x3i; x0i = x1i - x3r;
			a[j3] = wk3r * x0r - wk3i * x0i;
			a[j3 + 1] = wk3r * x0i + wk3i * x0r;
		}
	}
}

static void cftfsub(int n, double *a, double *w) {
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	l = 2;
	if (n > 8) {
		cft1st(n, a, w);
		l = 8;
		while ((l << 2) < n) {
			cftmdl(n, l, a, w);
			l <<= 2;
		}
	}
	if ((l << 2) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l; j2 = j1 + l; j3 = j2 + l;
			x0r = a[j] + a[j1]; x0i = a[j + 1] + a[j1 + 1];
			x1r = a[j] - a[j1]; x1i = a[j + 1] - a[j1 + 1];
			x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r; a[j + 1] = x0i + x2i;
			a[j2] = x0r - x2r; a[j2 + 1] = x0i - x2i;
			a[j1] = x1r - x3i; a[j1 + 1] = x1i + x3r;
			a[j3] = x1r + x3i; a[j3 + 1] = x1i - x3r;
		}
	} else {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1]; x0i = a[j + 1] - a[j1 + 1];
			a[j] += a[j1]; a[j + 1] += a[j1 + 1];
			a[j1] = x0r; a[j1 + 1] = x0i;
		}
	}
}

static void cftbsub(int n, double *a, double *w) {
	int j, j1, j2, j3, l;
	double x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
	l = 2;
	if (n > 8) {
		cft1st(n, a, w);
		l = 8;
		while ((l << 2) < n) {
			cftmdl(n, l, a, w);
			l <<= 2;
		}
	}
	if ((l << 2) == n) {
		for (j = 0; j < l; j += 2) {
			j1 = j + l; j2 = j1 + l; j3 = j2 + l;
			x0r = a[j] + a[j1]; x0i = -a[j + 1] - a[j1 + 1];
			x1r = a[j] - a[j1]; x1i = -a[j + 1] + a[j1 + 1];
			x2r = a[j2] + a[j3]; x2i = a[j2 + 1] + a[j3 + 1];
			x3r = a[j2] - a[j3]; x3i = a[j2 + 1] - a[j3 + 1];
			a[j] = x0r + x2r; a[j + 1] = x0i - x2i;
			a[j2] = x0r - x2r; a[j2 + 1] = x0i + x2i;
			a[j1] = x1r - x3i; a[j1 + 1] = x1i - x3r;
			a[j3] = x1r + x3i; a[j3 + 1] = x1i + x3r;
		}
	} else {
		for (j = 0; j < l; j += 2) {
			j1 = j + l;
			x0r = a[j] - a[j1]; x0i = -a[j + 1] + a[j1 + 1];
			a[j] += a[j1]; a[j + 1] = -a[j + 1] - a[j1 + 1];
			a[j1] = x0r; a[j1 + 1] = x0i;
		}
	}
}

static void rftfsub(int n, double *a, int nc, double *c) {
	int j, k, kk, ks, m;
	double wkr, wki, xr, xi, yr, yi;
	m = n >> 1; ks = 2 * nc / m; kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j; kk += ks;
		wkr = 0.5 - c[nc - kk]; wki = c[kk];
		xr = a[j] - a[k]; xi = a[j + 1] + a[k + 1];
		yr = wkr * xr - wki * xi; yi = wkr * xi + wki * xr;
		a[j] -= yr; a[j + 1] -= yi;
		a[k] += yr; a[k + 1] -= yi;
	}
}

static void rftbsub(int n, double *a, int nc, double *c) {
	int j, k, kk, ks, m;
	double wkr, wki, xr, xi, yr, yi;
	a[1] = -a[1]; m = n >> 1; ks = 2 * nc / m; kk = 0;
	for (j = 2; j < m; j += 2) {
		k = n - j; kk += ks;
		wkr = 0.5 - c[nc - kk]; wki = c[kk];
		xr = a[j] - a[k]; xi = a[j + 1] + a[k + 1];
		yr = wkr * xr + wki * xi; yi = wkr * xi - wki * xr;
		a[j] -= yr; a[j + 1] = yi - a[j + 1];
		a[k] += yr; a[k + 1] = yi - a[k + 1];
	}
	a[m + 1] = -a[m + 1];
}

static void rdft(int n, int isgn, double *a, int *ip, double *w) {
	int nw, nc;
	double xi;
	nw = ip[0];
	if (n > (nw << 2)) {
		nw = n >> 2;
		makewt(nw, ip, w);
	}
	nc = ip[1];
	if (n > (nc << 2)) {
		nc = n >> 2;
		makect(nc, ip, w + nw);
	}
	if (isgn >= 0) {
		if (n > 4) {
			bitrv2(n, ip + 2, a);
			cftfsub(n, a, w);
			rftfsub(n, a, nc, w + nw);
		} else if (n == 4) {
			cftfsub(n, a, w);
		}
		xi = a[0] - a[1];
		a[0] += a[1];
		a[1] = xi;
	} else {
		a[1] = 0.5 * (a[0] - a[1]);
		a[0] -= a[1];
		if (n > 4) {
			rftbsub(n, a, nc, w + nw);
			bitrv2(n, ip + 2, a);
			cftbsub(n, a, w);
		} else if (n == 4) {
			cftfsub(n, a, w);
		}
	}
}

} // namespace OouraFFT


// ============================================================================
// Helper functions from Open303 rosic_NumberManipulations.h / rosic_RealFunctions.h
// ============================================================================

inline int roundToInt(double x) {
	double xFloor = std::floor(x);
	double xFrac = x - xFloor;
	if (xFrac >= 0.5)
		return (int)xFloor + 1;
	else
		return (int)xFloor;
}

template<typename T>
inline T clip(T x, T min, T max) {
	if (x > max)
		return max;
	else if (x < min)
		return min;
	else
		return x;
}

// ============================================================================
// MipMappedWaveTable303 - Ported from Open303 rosic_MipMappedWaveTable
// Generates bandlimited wavetables with FFT-based mip-mapping
// ============================================================================

struct MipMappedWaveTable303 {
	static constexpr int tableLength = 2048;
	static constexpr int numTables = 12;

	enum Waveform { SAW303, SQUARE303 };

	double prototypeTable[tableLength];
	double tableSet[numTables][tableLength + 4];  // +4 for interpolation padding
	int* ip = nullptr;
	double* w = nullptr;
	bool initialized = false;

	double tanhShaperFactor = std::exp(36.9 * 0.11512925464970228420089957273422);  // dB2amp(36.9)
	double tanhShaperOffset = 4.37;
	double squarePhaseShift = 180.0;

	MipMappedWaveTable303() {
		allocate();
		initTables();
	}

	~MipMappedWaveTable303() {
		deallocate();
	}

	void allocate() {
		if (initialized) return;
		int ipSize = 4 + (int)std::ceil(std::sqrt((double)tableLength));
		ip = new int[ipSize];
		ip[0] = 0;
		w = new double[2 * tableLength];
		initialized = true;
	}

	void deallocate() {
		if (!initialized) return;
		delete[] ip; ip = nullptr;
		delete[] w; w = nullptr;
		initialized = false;
	}

	void initTables() {
		for (int i = 0; i < tableLength; i++)
			prototypeTable[i] = 0.0;
		for (int t = 0; t < numTables; t++)
			for (int i = 0; i < tableLength + 4; i++)
				tableSet[t][i] = 0.0;
	}

	void circularShift(double* buffer, int length, int numPositions) {
		int na = std::abs(numPositions);
		while (na > length)
			na -= length;
		double* tmp = new double[na];
		if (numPositions < 0) {
			std::memcpy(tmp, buffer, na * sizeof(double));
			std::memmove(buffer, &buffer[na], (length - na) * sizeof(double));
			std::memcpy(&buffer[length - na], tmp, na * sizeof(double));
		} else if (numPositions > 0) {
			std::memcpy(tmp, &buffer[length - na], na * sizeof(double));
			std::memmove(&buffer[na], buffer, (length - na) * sizeof(double));
			std::memcpy(buffer, tmp, na * sizeof(double));
		}
		delete[] tmp;
	}

	void generateMipMap() {
		double spectrum[tableLength];

		// Copy prototype to first table
		for (int i = 0; i < tableLength; i++)
			tableSet[0][i] = prototypeTable[i];
		// Interpolation padding
		tableSet[0][tableLength] = tableSet[0][0];
		tableSet[0][tableLength + 1] = tableSet[0][1];
		tableSet[0][tableLength + 2] = tableSet[0][2];
		tableSet[0][tableLength + 3] = tableSet[0][3];

		// Get spectrum from prototype - forward FFT
		for (int i = 0; i < tableLength; i++)
			spectrum[i] = prototypeTable[i];
		OouraFFT::rdft(tableLength, 1, spectrum, ip, w);

		// Ensure DC and Nyquist are zero (matches Open303)
		spectrum[0] = 0.0;
		spectrum[1] = 0.0;

		// Generate bandlimited versions
		for (int t = 1; t < numTables; t++) {
			int lowBin = tableLength / (1 << t);
			int highBin = tableLength / (1 << (t - 1));

			// Zero out bins above cutoff
			for (int i = lowBin; i < highBin; i++)
				spectrum[i] = 0.0;

			// Inverse FFT - copy spectrum with scaling
			// NOTE: No conjugation needed here because:
			// - Original: forward FFT conjugates, then inverse conjugates again (cancels)
			// - Our flow: forward FFT raw output is already in Ooura's expected format
			double tmpSignal[tableLength];
			for (int n = 0; n < tableLength; n++)
				tmpSignal[n] = 2.0 * spectrum[n] / tableLength;

			OouraFFT::rdft(tableLength, -1, tmpSignal, ip, w);

			for (int i = 0; i < tableLength; i++)
				tableSet[t][i] = tmpSignal[i];
			// Interpolation padding
			tableSet[t][tableLength] = tableSet[t][0];
			tableSet[t][tableLength + 1] = tableSet[t][1];
			tableSet[t][tableLength + 2] = tableSet[t][2];
			tableSet[t][tableLength + 3] = tableSet[t][3];
		}
	}

	void fillWithSaw303() {
		int N = tableLength;
		double k = 0.5;
		int N1 = clip(roundToInt(k * (N - 1)), 1, N - 1);
		int N2 = N - N1;
		double s1 = 1.0 / (N1 - 1);
		double s2 = 1.0 / N2;
		for (int n = 0; n < N1; n++)
			prototypeTable[n] = s1 * n;
		for (int n = N1; n < N; n++)
			prototypeTable[n] = -1.0 + s2 * (n - N1);
		generateMipMap();
	}

	void fillWithSquare303() {
		int N = tableLength;
		double k = 0.5;
		int N1 = clip(roundToInt(k * (N - 1)), 1, N - 1);
		int N2 = N - N1;
		double s1 = 1.0 / (N1 - 1);
		double s2 = 1.0 / N2;
		for (int n = 0; n < N1; n++)
			prototypeTable[n] = s1 * n;
		for (int n = N1; n < N; n++)
			prototypeTable[n] = -1.0 + s2 * (n - N1);

		// Apply tanh shaping with offset (switch polarity)
		for (int n = 0; n < N; n++)
			prototypeTable[n] = -std::tanh(tanhShaperFactor * prototypeTable[n] + tanhShaperOffset);

		// Circular shift for phase alignment with saw wave
		int nShift = roundToInt(N * squarePhaseShift / 360.0);
		circularShift(prototypeTable, N, nShift);

		generateMipMap();
	}

	void setWaveform(Waveform waveform) {
		if (waveform == SAW303)
			fillWithSaw303();
		else
			fillWithSquare303();
	}

	// Lookup with linear interpolation - matches Open303
	inline double getValueLinear(int integerPart, double fractionalPart, int tableIndex) const {
		if (tableIndex < 0) tableIndex = 0;
		else if (tableIndex >= numTables) tableIndex = numTables - 1;
		return (1.0 - fractionalPart) * tableSet[tableIndex][integerPart]
		     + fractionalPart * tableSet[tableIndex][integerPart + 1];
	}
};


// ============================================================================
// BlendOscillator303 - Ported from Open303 rosic_BlendOscillator
// Blends between SAW303 and SQUARE303 wavetables
// ============================================================================

struct BlendOscillator303 {
	MipMappedWaveTable303 sawTable;
	MipMappedWaveTable303 squareTable;

	static constexpr int tableLength = MipMappedWaveTable303::tableLength;
	double tableLengthDbl = (double)tableLength;

	double frequency = 440.0;
	double sampleRate = 44100.0;
	double sampleRateRec = 1.0 / 44100.0;
	double phaseIndex = 0.0;      // 0 to tableLength (2048)
	double increment = 0.0;       // tableLength * freq / sampleRate
	double startIndex = 0.0;
	double blend = 0.0;

	bool tablesInitialized = false;

	void initTables() {
		if (tablesInitialized) return;
		sawTable.setWaveform(MipMappedWaveTable303::SAW303);
		squareTable.setWaveform(MipMappedWaveTable303::SQUARE303);
		tablesInitialized = true;
	}

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) {
			sampleRate = newSampleRate;
			sampleRateRec = 1.0 / sampleRate;
			calculateIncrement();
		}
	}

	void setFrequency(double newFrequency) {
		if (newFrequency > 0.0 && newFrequency < 20000.0)
			frequency = newFrequency;
		calculateIncrement();
	}

	void setBlend(double newBlend) {
		blend = clamp((float)newBlend, 0.f, 1.f);
	}

	void calculateIncrement() {
		increment = tableLengthDbl * frequency * sampleRateRec;
	}

	void resetPhase() {
		phaseIndex = startIndex;
	}

	inline int getExponentOfDouble(double value) const {
		union { double d; uint64_t i; } u;
		u.d = value;
		int exponent = (int)(((u.i & 0x7FFFFFFFFFFFFFFFULL) >> 52)) - 1023;
		return exponent;
	}

	int getTableIndex() const {
		int tableNumber = getExponentOfDouble(increment);
		tableNumber += 2;  // Generate frequencies up to nyquist/4
		if (tableNumber < 0) tableNumber = 0;
		if (tableNumber >= MipMappedWaveTable303::numTables)
			tableNumber = MipMappedWaveTable303::numTables - 1;
		return tableNumber;
	}

	inline int floorInt(double x) const {
		return (int)std::floor(x);
	}

	float getSample() {
		if (!tablesInitialized) initTables();

		int tableNumber = getTableIndex();

		// Wraparound
		while (phaseIndex >= tableLengthDbl)
			phaseIndex -= tableLengthDbl;

		int intIndex = floorInt(phaseIndex);
		double frac = phaseIndex - (double)intIndex;

		double out1 = (1.0 - blend) * sawTable.getValueLinear(intIndex, frac, tableNumber);
		double out2 = blend * squareTable.getValueLinear(intIndex, frac, tableNumber);

		// Square wave scaling (matches Open303 line 160)
		out2 *= 0.5;

		phaseIndex += increment;
		return (float)(out1 + out2);
	}

	float saw() {
		if (!tablesInitialized) initTables();

		int tableNumber = getTableIndex();

		while (phaseIndex >= tableLengthDbl)
			phaseIndex -= tableLengthDbl;

		int intIndex = floorInt(phaseIndex);
		double frac = phaseIndex - (double)intIndex;

		double value = sawTable.getValueLinear(intIndex, frac, tableNumber);

		phaseIndex += increment;
		return (float)value;
	}

	float sqr() {
		if (!tablesInitialized) initTables();

		int tableNumber = getTableIndex();

		while (phaseIndex >= tableLengthDbl)
			phaseIndex -= tableLengthDbl;

		int intIndex = floorInt(phaseIndex);
		double frac = phaseIndex - (double)intIndex;

		double value = squareTable.getValueLinear(intIndex, frac, tableNumber);

		// Square wave scaling (matches Open303)
		value *= 0.5;

		phaseIndex += increment;
		return (float)value;
	}
};


// ============================================================================
// Circuit-Accurate Filter - Ported from Open303 by Robin Schmidt
// Based on the rosic::TeeBeeFilter (ROSIC library)
// https://github.com/maddanio/open303
//
// This is a circuit-accurate emulation based on analysis by mystran & kunn
// (KVR thread page 40) with:
// - Polynomial-approximated coefficients matching real 303 response
// - 1+3 pole configuration (characteristic of 303 diode ladder)
// - Highpass filter in feedback path reducing bass resonance
// - Nonlinear waveshaping between stages
// ============================================================================

struct OnePoleFilter303 {
	double x1 = 0.0, y1 = 0.0;
	double b0 = 1.0, b1 = 0.0, a1 = 0.0;
	double cutoffFreq = 150.0;
	double sampleRate = 44100.0;

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) sampleRate = newSampleRate;
		calcCoeffs();
	}

	void setCutoff(double newCutoff) {
		if (newCutoff > 0.0 && newCutoff <= 20000.0)
			cutoffFreq = newCutoff;
		else
			cutoffFreq = 20000.0;
		calcCoeffs();
	}

	void calcCoeffs() {
		double x = std::exp(-2.0 * M_PI * cutoffFreq / sampleRate);
		b0 =  0.5 * (1.0 + x);
		b1 = -0.5 * (1.0 + x);
		a1 = x;
	}

	double getSample(double in) {
		y1 = b0 * in + b1 * x1 + a1 * y1 + 1e-20;
		x1 = in;
		return y1;
	}

	void reset() {
		x1 = 0.0;
		y1 = 0.0;
	}
};

struct TeeBeeFilter303 {
	float cutoff = 1000.f;
	float resonance = 0.f;
	float lowpass = 0.f;
	float highpass = 0.f;

	double b0 = 0.0, a1 = 0.0;
	double y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
	double k = 0.0;
	double g = 1.0;
	double resonanceRaw = 0.0;
	double resonanceSkewed = 0.0;
	double sampleRate = 44100.0;
	double twoPiOverSampleRate = 2.0 * M_PI / 44100.0;

	OnePoleFilter303 feedbackHighpass;

	TeeBeeFilter303() {
		feedbackHighpass.setCutoff(150.0);
		calculateCoefficients();
	}

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) sampleRate = newSampleRate;
		twoPiOverSampleRate = 2.0 * M_PI / sampleRate;
		feedbackHighpass.setSampleRate(newSampleRate);
		calculateCoefficients();
	}

	void setCutoff(float freq) {
		double newCutoff = (double)freq;
		if (newCutoff < 200.0) newCutoff = 200.0;
		else if (newCutoff > 20000.0) newCutoff = 20000.0;
		cutoff = (float)newCutoff;
		calculateCoefficients();
	}

	void setResonance(float res) {
		resonanceRaw = clamp((float)res / 10.f, 0.f, 1.f);
		resonanceSkewed = (1.0 - std::exp(-3.0 * resonanceRaw)) / (1.0 - std::exp(-3.0));
		calculateCoefficients();
	}

	void calculateCoefficients() {
		double wc = twoPiOverSampleRate * (double)cutoff;
		double r = resonanceSkewed;

		const double ONE_OVER_SQRT2 = 0.70710678118654752440084436210485;
		double fx = wc * ONE_OVER_SQRT2 / (2.0 * M_PI);

		b0 = (0.00045522346 + 6.1922189 * fx) / (1.0 + 12.358354 * fx + 4.4156345 * (fx * fx));
		k = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997) - 5837.7917) - 476.47308) + 614.95611) + 213.87126) + 16.998792;

		g = k * 0.058823529411764705882352941176471;
		g = (g - 1.0) * r + 1.0;
		g = g * (1.0 + r);
		k = k * r;
	}

	inline double shape(double x) {
		const double SQRT2 = 1.4142135623730950488016887242097;
		const double r6 = 1.0 / 6.0;

		if (x > SQRT2) x = SQRT2;
		else if (x < -SQRT2) x = -SQRT2;
		return x - r6 * x * x * x;
	}

	void reset() {
		y1 = y2 = y3 = y4 = 0.0;
		feedbackHighpass.reset();
		lowpass = 0.f;
		highpass = 0.f;
	}

	void process(float input, float deltaTime) {
		setResonance(resonance);

		double in = (double)input;
		double y0 = in - feedbackHighpass.getSample(k * shape(y4));
		highpass = (float)feedbackHighpass.y1;

		y1 += 2.0 * b0 * (y0 - y1 + y2);
		y2 += b0 * (y1 - 2.0 * y2 + y3);
		y3 += b0 * (y2 - 2.0 * y3 + y4);
		y4 += b0 * (y3 - 2.0 * y4);

		lowpass = (float)(2.0 * g * y4);
	}
};


// ============================================================================
// Analog Envelope
// ============================================================================

struct AnalogEnvelope303 {
	enum Stage { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
	Stage stage = IDLE;

	double attackTime = 3.0;
	double decayTime = 200.0;
	double sustainLevel = 0.0;
	double releaseTime = 3.0;

	double sampleRate = 44100.0;
	double output = 0.0;
	double attackCoeff = 0.0;
	double decayCoeff = 0.0;
	double releaseCoeff = 0.0;
	double peakLevel = 1.0;
	double targetLevel = 0.0;
	bool noteOn_ = false;

	static constexpr double tauScale = 1.0;

	void setSampleRate(double newSampleRate) {
		sampleRate = newSampleRate;
		calculateCoefficients();
	}

	void setAttack(double ms) {
		attackTime = std::max(0.1, ms);
		calculateCoefficients();
	}

	void setDecay(double ms) {
		decayTime = std::max(1.0, ms);
		calculateCoefficients();
	}

	void setSustain(double level) {
		sustainLevel = clamp((float)level, 0.f, 1.f);
	}

	void setRelease(double ms) {
		releaseTime = std::max(0.1, ms);
		calculateCoefficients();
	}

	void calculateCoefficients() {
		double sr = sampleRate;
		attackCoeff = std::exp(-1000.0 / (attackTime * tauScale * sr));
		decayCoeff = std::exp(-1000.0 / (decayTime * tauScale * sr));
		releaseCoeff = std::exp(-1000.0 / (releaseTime * tauScale * sr));
	}

	void noteOn(bool startFromCurrentLevel = false) {
		noteOn_ = true;
		stage = ATTACK;
		targetLevel = peakLevel;
		if (!startFromCurrentLevel) {
			output = 0.0;
		}
	}

	void noteOff() {
		noteOn_ = false;
		stage = RELEASE;
		targetLevel = 0.0;
	}

	void reset() {
		stage = IDLE;
		output = 0.0;
		noteOn_ = false;
	}

	bool isNoteOn() const { return noteOn_; }

	double process() {
		switch (stage) {
			case IDLE:
				output = 0.0;
				break;

			case ATTACK:
				output = output + (1.0 - attackCoeff) * (peakLevel - output);
				if (output >= peakLevel * 0.99) {
					output = peakLevel;
					stage = DECAY;
					targetLevel = sustainLevel;
				}
				break;

			case DECAY:
				output = output + (1.0 - decayCoeff) * (sustainLevel - output);
				if (noteOn_) {
					if (output <= sustainLevel * 1.01 + 0.001) {
						stage = SUSTAIN;
					}
				}
				break;

			case SUSTAIN:
				output = sustainLevel;
				break;

			case RELEASE:
				output = output * releaseCoeff;
				if (output < 0.0001) {
					output = 0.0;
					stage = IDLE;
				}
				break;
		}

		return output;
	}

	float getSample() {
		return (float)process();
	}
};


// ============================================================================
// Leaky Integrator
// ============================================================================

struct LeakyIntegrator303 {
	double coeff = 0.0;
	double y1 = 0.0;
	double sampleRate = 44100.0;
	double tau = 60.0;

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) {
			sampleRate = newSampleRate;
			calculateCoefficient();
		}
	}

	void setTimeConstant(double newTau) {
		if (newTau >= 0.0 && newTau != tau) {
			tau = newTau;
			calculateCoefficient();
		}
	}

	void setState(double newState) {
		y1 = newState;
	}

	void calculateCoefficient() {
		if (tau > 0.0)
			coeff = std::exp(-1.0 / (sampleRate * 0.001 * tau));
		else
			coeff = 0.0;
	}

	void reset() {
		y1 = 0.0;
	}

	double process(double in) {
		return y1 = in + coeff * (y1 - in);
	}
};


// ============================================================================
// Simple One-Pole Highpass Filter
// ============================================================================

struct SimpleHighpass303 {
	double x1 = 0.0, y1 = 0.0;
	double b0 = 1.0, b1 = 0.0, a1 = 0.0;
	double cutoffFreq = 44.0;
	double sampleRate = 44100.0;

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) {
			sampleRate = newSampleRate;
			calculateCoefficients();
		}
	}

	void setCutoff(double newCutoff) {
		cutoffFreq = clamp((float)newCutoff, 1.f, 20000.f);
		calculateCoefficients();
	}

	void calculateCoefficients() {
		double x = std::exp(-2.0 * M_PI * cutoffFreq / sampleRate);
		b0 =  0.5 * (1.0 + x);
		b1 = -0.5 * (1.0 + x);
		a1 = x;
	}

	void reset() {
		x1 = 0.0;
		y1 = 0.0;
	}

	double process(double in) {
		y1 = b0 * in + b1 * x1 + a1 * y1;
		x1 = in;
		return y1;
	}
};


// ============================================================================
// ENV MOD Scaler
// ============================================================================

struct EnvModScaler303 {
	static constexpr double c0   = 3.138152786059267e+002;
	static constexpr double c1   = 2.394411986817546e+003;
	static constexpr double oF   = 0.048292930943553;
	static constexpr double oC   = 0.294391201442418;
	static constexpr double sLoF = 3.773996325111173;
	static constexpr double sLoC = 0.736965594166206;
	static constexpr double sHiF = 4.194548788411135;
	static constexpr double sHiC = 0.864344900642434;

	double envScaler = 1.0;
	double envOffset = 0.0;

	void calculate(double cutoff, double envMod) {
		double e = clamp((float)(envMod / 100.0), 0.f, 1.f);

		double c = 0.0;
		if (cutoff >= c0 && cutoff <= c1) {
			c = std::log(cutoff / c0) / std::log(c1 / c0);
		} else if (cutoff > c1) {
			c = 1.0;
		}
		c = clamp((float)c, 0.f, 1.f);

		double sLo = sLoF * e + sLoC;
		double sHi = sHiF * e + sHiC;

		envScaler = (1.0 - c) * sLo + c * sHi;
		envOffset = oF * c + oC;
	}

	double apply(double envValue) {
		return envScaler * (envValue - envOffset);
	}
};


// ============================================================================
// Elliptic Quarter-Band Anti-Aliasing Filter
// Ported from Open303 rosic_EllipticQuarterBandFilter
// 12th order elliptic filter for 2x oversampling
// ============================================================================

struct EllipticQuarterBandFilter {
	double w[12] = {0.0};

	void reset() {
		for (int i = 0; i < 12; i++) w[i] = 0.0;
	}

	inline double process(double in) {
		const double TINY = FLT_MIN;

		static constexpr double a01 = -9.1891604652189471;
		static constexpr double a02 = 40.177553696870497;
		static constexpr double a03 = -110.11636661771178;
		static constexpr double a04 = 210.18506612078195;
		static constexpr double a05 = -293.84744771903240;
		static constexpr double a06 = 308.16345558359234;
		static constexpr double a07 = -244.06786780384243;
		static constexpr double a08 = 144.81877911392738;
		static constexpr double a09 = -62.770692151724198;
		static constexpr double a10 = 18.867762095902137;
		static constexpr double a11 = -3.5327094230551848;
		static constexpr double a12 = 0.31183189275203149;

		static constexpr double b00 = 0.00013671732099945628;
		static constexpr double b01 = -0.00055538501265606384;
		static constexpr double b02 = 0.0013681887636296387;
		static constexpr double b03 = -0.0022158566490711852;
		static constexpr double b04 = 0.0028320091007278322;
		static constexpr double b05 = -0.0029776933151090413;
		static constexpr double b06 = 0.0030283628243514991;
		static constexpr double b07 = -0.0029776933151090413;
		static constexpr double b08 = 0.0028320091007278331;
		static constexpr double b09 = -0.0022158566490711861;
		static constexpr double b10 = 0.0013681887636296393;
		static constexpr double b11 = -0.00055538501265606384;
		static constexpr double b12 = 0.00013671732099945636;

		double tmp = (in + TINY)
			- ((a01*w[0] + a02*w[1]) + (a03*w[2] + a04*w[3]))
			- ((a05*w[4] + a06*w[5]) + (a07*w[6] + a08*w[7]))
			- ((a09*w[8] + a10*w[9]) + (a11*w[10] + a12*w[11]));

		double y = b00*tmp
			+ ((b01*w[0] + b02*w[1]) + (b03*w[2] + b04*w[3]))
			+ ((b05*w[4] + b06*w[5]) + (b07*w[6] + b08*w[7]))
			+ ((b09*w[8] + b10*w[9]) + (b11*w[10] + b12*w[11]));

		std::memmove(&w[1], &w[0], 11 * sizeof(double));
		w[0] = tmp;

		return y;
	}
};


// ============================================================================
// Decay Envelope
// ============================================================================

struct DecayEnvelope303 {
	double c = 1.0;
	double y = 0.0;
	double yInit = 1.0;
	double tau = 200.0;
	double sampleRate = 44100.0;
	bool normalizeSum = false;

	void setSampleRate(double newSampleRate) {
		if (newSampleRate > 0.0) {
			sampleRate = newSampleRate;
			calculateCoefficient();
		}
	}

	void setDecayTimeConstant(double newTau) {
		if (newTau > 0.001) {
			tau = newTau;
			calculateCoefficient();
		}
	}

	void setNormalizeSum(bool shouldNormalize) {
		normalizeSum = shouldNormalize;
		calculateCoefficient();
	}

	void calculateCoefficient() {
		c = std::exp(-1.0 / (0.001 * tau * sampleRate));
		if (normalizeSum)
			yInit = (1.0 - c) / c;
		else
			yInit = 1.0 / c;
	}

	void trigger() {
		y = yInit;
	}

	void reset() {
		y = 0.0;
	}

	bool endIsReached(double threshold = 0.0001) {
		return y < threshold;
	}

	double process() {
		y *= c;
		return y;
	}

	float getSample() {
		return (float)process();
	}
};


// ============================================================================
// SynthVoice
// ============================================================================

struct SynthVoice {
	BlendOscillator303 wavetableOsc;
	TeeBeeFilter303 filter;
	LeakyIntegrator303 pitchSlew;
	SimpleHighpass303 preFilterHPF;
	SimpleHighpass303 postFilterHPF;
	EnvModScaler303 envModScaler;
	DecayEnvelope303 accentDecayEnv;
	EllipticQuarterBandFilter antiAliasFilter;

	float env = 0.0f;
	float accentEnv = 0.0f;
	bool decaying = false;
	bool prevGate = false;

	double sampleRate = 44100.0;

	static constexpr float SLIDE_ATTACK_MS = 3.0f;
	static constexpr float SLIDE_DECAY_MIN_MS = 200.0f;
	static constexpr float SLIDE_DECAY_MAX_MS = 2500.0f;
	static constexpr float VCF_CUTOFF_MIN = 314.0f;
	static constexpr float VCF_CUTOFF_MAX = 2394.0f;
	static constexpr int OVERSAMPLE = 2;

	void setSampleRate(double sr) {
		if (sr <= 0.0) return;
		sampleRate = sr;
		wavetableOsc.setSampleRate(sr);
		wavetableOsc.initTables();
		filter.setSampleRate(sr * OVERSAMPLE);  // 2x oversampling
		pitchSlew.setSampleRate(sr);
		preFilterHPF.setSampleRate(sr * OVERSAMPLE);  // 2x oversampling
		postFilterHPF.setSampleRate(sr);
		accentDecayEnv.setSampleRate(sr);

		// Initialize fixed cutoffs
		preFilterHPF.setCutoff(44.486);
		postFilterHPF.setCutoff(24.167);
		accentDecayEnv.setDecayTimeConstant(200.0);
		accentDecayEnv.setNormalizeSum(false);
	}

	void reset() {
		filter.reset();
		antiAliasFilter.reset();
		pitchSlew.setTimeConstant(0.1f);
		env = 0.0f;
		accentEnv = 0.0f;
		decaying = false;
		prevGate = false;
	}

	float process(float pitch, bool gate, bool accent, bool slide,
	              float cutoff, float cutoffCv, float resonance, float envMod,
	              float decay, float accentAmt, float drive, float fineTune, float blend, float slideTime,
	              float vcfRangeShift, float decayRangeShift, float sustainLevel, float releaseAmt) {

		float sampleTime = 1.0f / sampleRate;

		if (slide) {
			float slideTimeMs = slideTime * 50.0f;
			pitchSlew.setTimeConstant(std::max(slideTimeMs, 10.0f));
		} else {
			pitchSlew.setTimeConstant(0.1f);
		}

		float pitchFine = 7.0f * dsp::quadraticBipolar(fineTune) / 12.0f;
		float totalPitch = pitch + pitchFine;

		float targetFreq = 261.626f * std::pow(2.f, totalPitch);
		float vcoFreq = (float)pitchSlew.process(targetFreq);

		wavetableOsc.setFrequency(vcoFreq);
		wavetableOsc.setBlend(blend);
		float oscOut = wavetableOsc.getSample();

		bool newNote = gate && !prevGate;
		prevGate = gate;

		float decayTimeMs = SLIDE_DECAY_MIN_MS * std::pow(SLIDE_DECAY_MAX_MS / SLIDE_DECAY_MIN_MS, decay);
		if (accent) {
			decayTimeMs = SLIDE_DECAY_MIN_MS;
		}

		float decayMultiplier = 4.0f * (1.0f - decayRangeShift * 0.5f);

		float attackTimeSec = SLIDE_ATTACK_MS / 1000.0f;
		float decayTimeSec = decayTimeMs / 1000.0f;
		float attackRate = 1.0f / (attackTimeSec * sampleRate);
		float decayRate = 1.0f / (decayTimeSec * sampleRate);

		if (newNote) {
			decaying = false;
			if (accent) {
				accentDecayEnv.trigger();
			}
		}

		if (gate) {
			if (!decaying) {
				env += attackRate;
				if (env >= 1.0f) {
					env = 1.0f;
					decaying = true;
				}
			} else {
				float target = sustainLevel;
				if (env > target + 0.001f) {
					env *= (1.0f - decayRate * decayMultiplier);
					if (env < target) env = target;
				} else {
					env = target;
				}
			}
		} else {
			float releaseMs = 21.0f + releaseAmt * 479.0f;
			float releaseRate = 1.0f / (releaseMs / 1000.0f * sampleRate);
			env *= (1.0f - releaseRate);
			if (env < 0.001f) env = 0.0f;
			decaying = false;
		}

		accentEnv = accentDecayEnv.getSample();

		float envLevel = env;
		if (accent) {
			envLevel *= (1.0f + accentAmt * 0.5f);
			envLevel = clamp(envLevel, 0.0f, 1.5f);
		}

		float vcaOut = oscOut * envLevel;

		float input = vcaOut / 5.0f;
		float gain = std::pow(1.f + drive, 5);
		input *= gain;
		input += 1e-6f * (2.f * random::uniform() - 1.f);

		filter.resonance = std::pow(resonance, 2) * 10.f;

		float t = clamp(vcfRangeShift + 1.0f, 0.0f, 1.0f);
		float vcfMin = 100.0f * std::pow(314.0f / 100.0f, t);
		float vcfMax = 760.0f * std::pow(2394.0f / 760.0f, t);
		float baseCutoffHz = vcfMin * std::exp(cutoff * std::log(vcfMax / vcfMin));
		baseCutoffHz *= std::pow(2.f, cutoffCv);
		baseCutoffHz = clamp(baseCutoffHz, 20.f, 20000.f);

		envModScaler.calculate(baseCutoffHz, envMod * 100.0f);
		float envModOct = (float)envModScaler.apply(env);
		float cutoffHz = baseCutoffHz * std::pow(2.f, envModOct);

		if (accent) {
			float accentModOct = accentAmt * resonance * 1.0f;
			cutoffHz *= std::pow(2.f, accentEnv * accentModOct);
		}

		cutoffHz = clamp(cutoffHz, 20.f, 20000.f);
		filter.setCutoff(cutoffHz);

		float oversampleTime = sampleTime / OVERSAMPLE;
		float filterOut = 0.f;
		for (int i = 0; i < OVERSAMPLE; i++) {
			float hpfInput = (float)preFilterHPF.process(input);
			filter.process(hpfInput, oversampleTime);
			filterOut = (float)antiAliasFilter.process(filter.lowpass);
		}
		filterOut = (float)postFilterHPF.process(filterOut);

		return 5.f * filterOut;
	}
};
