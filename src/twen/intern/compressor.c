// (c) Copyright 2016, Sean Connelly (@voidqk), http://syntheti.cc
// MIT License
// Project Home: https://github.com/voidqk/sndfilter

#include "compressor.h"
#include <math.h>
#include <string.h>

// core algorithm extracted from Chromium source, DynamicsCompressorKernel.cpp, here:
//   https://git.io/v1uSK
//
// changed a few things though in an attempt to simplify the curves and algorithm, and also included
// a pregain so that samples can be scaled up then compressed

void sf_defaultcomp(sf_compressor_state_st *state, int rate){
	// sane defaults
	sf_advancecomp(state, rate,
		  0.000f, // pregain
		-24.000f, // threshold
		 30.000f, // knee
		 12.000f, // ratio
		  0.003f, // attack
		  0.250f, // release
		  0.006f, // predelay
		  0.090f, // releasezone1
		  0.160f, // releasezone2
		  0.420f, // releasezone3
		  0.980f, // releasezone4
		  0.000f, // postgain
		  1.000f  // wet
	);
}

void sf_simplecomp(sf_compressor_state_st *state, int rate, float pregain, float threshold,
	float knee, float ratio, float attack, float release){
	// sane defaults
	sf_advancecomp(state, rate, pregain, threshold, knee, ratio, attack, release,
		0.006f, // predelay
		0.090f, // releasezone1
		0.160f, // releasezone2
		0.420f, // releasezone3
		0.980f, // releasezone4
		0.000f, // postgain
		1.000f  // wet
	);
}

static inline float db2lin(float db){ // dB to linear
	return powf(10.0f, 0.05f * db);
}

static inline float lin2db(float lin){ // linear to dB
	return 20.0f * log10f(lin);
}

// for more information on the knee curve, check out the compressor-curve.html demo + source code
// included in this repo
static inline float kneecurve(float x, float k, float linearthreshold){
	return linearthreshold + (1.0f - expf(-k * (x - linearthreshold))) / k;
}

static inline float kneeslope(float x, float k, float linearthreshold){
	return k * x / ((k * linearthreshold + 1.0f) * expf(k * (x - linearthreshold)) - 1);
}

static inline float compcurve(float x, float k, float slope, float linearthreshold,
	float linearthresholdknee, float threshold, float knee, float kneedboffset){
	if (x < linearthreshold)
		return x;
	if (knee <= 0.0f) // no knee in curve
		return db2lin(threshold + slope * (lin2db(x) - threshold));
	if (x < linearthresholdknee)
		return kneecurve(x, k, linearthreshold);
	return db2lin(kneedboffset + slope * (lin2db(x) - threshold - knee));
}

// this is the main initialization function
// it does a bunch of pre-calculation so that the inner loop of signal processing is fast
void sf_advancecomp(sf_compressor_state_st *state, int rate, float pregain, float threshold,
	float knee, float ratio, float attack, float release, float predelay, float releasezone1,
	float releasezone2, float releasezone3, float releasezone4, float postgain, float wet){

	// setup the predelay buffer
	int delaybufsize = rate * predelay;
	if (delaybufsize < 1)
		delaybufsize = 1;
	else if (delaybufsize > SF_COMPRESSOR_MAXDELAY)
		delaybufsize = SF_COMPRESSOR_MAXDELAY;
        memset(state->delaybuf, 0, sizeof(sf_sample_st) * delaybufsize);

	// useful values
	float linearpregain = db2lin(pregain);
	float linearthreshold = db2lin(threshold);
	float slope = 1.0f / ratio;
	float attacksamples = rate * attack;
	float attacksamplesinv = 1.0f / attacksamples;
	float releasesamples = rate * release;
	float satrelease = 0.0025f; // seconds
	float satreleasesamplesinv = 1.0f / ((float)rate * satrelease);
	float dry = 1.0f - wet;

	// metering values (not used in core algorithm, but used to output a meter if desired)
	float metergain = 1.0f; // gets overwritten immediately because gain will always be negative
	float meterfalloff = 0.325f; // seconds
	float meterrelease = 1.0f - expf(-1.0f / ((float)rate * meterfalloff));

	// calculate knee curve parameters
	float k = 5.0f; // initial guess
	float kneedboffset = 0.0f;
	float linearthresholdknee = 0.0f;
	if (knee > 0.0f){ // if a knee exists, search for a good k value
		float xknee = db2lin(threshold + knee);
		float mink = 0.1f;
		float maxk = 10000.0f;
		// search by comparing the knee slope at the current k guess, to the ideal slope
		for (int i = 0; i < 15; i++){
			if (kneeslope(xknee, k, linearthreshold) < slope)
				maxk = k;
			else
				mink = k;
			k = sqrtf(mink * maxk);
		}
		kneedboffset = lin2db(kneecurve(xknee, k, linearthreshold));
		linearthresholdknee = db2lin(threshold + knee);
	}

	// calculate a master gain based on what sounds good
	float fulllevel = compcurve(1.0f, k, slope, linearthreshold, linearthresholdknee,
		threshold, knee, kneedboffset);
	float mastergain = db2lin(postgain) * powf(1.0f / fulllevel, 0.6f);

	// calculate the adaptive release curve parameters
	// solve a,b,c,d in `y = a*x^3 + b*x^2 + c*x + d`
	// interescting points (0, y1), (1, y2), (2, y3), (3, y4)
	float y1 = releasesamples * releasezone1;
	float y2 = releasesamples * releasezone2;
	float y3 = releasesamples * releasezone3;
	float y4 = releasesamples * releasezone4;
	float a = (-y1 + 3.0f * y2 - 3.0f * y3 + y4) / 6.0f;
	float b = y1 - 2.5f * y2 + 2.0f * y3 - 0.5f * y4;
	float c = (-11.0f * y1 + 18.0f * y2 - 9.0f * y3 + 2.0f * y4) / 6.0f;
	float d = y1;

	// save everything
	state->metergain            = 1.0f; // large value overwritten immediately since it's always < 0
	state->meterrelease         = meterrelease;
	state->threshold            = threshold;
	state->knee                 = knee;
	state->wet                  = wet;
	state->linearpregain        = linearpregain;
	state->linearthreshold      = linearthreshold;
	state->slope                = slope;
	state->attacksamplesinv     = attacksamplesinv;
	state->satreleasesamplesinv = satreleasesamplesinv;
	state->dry                  = dry;
	state->k                    = k;
	state->kneedboffset         = kneedboffset;
	state->linearthresholdknee  = linearthresholdknee;
	state->mastergain           = mastergain;
	state->a                    = a;
	state->b                    = b;
	state->c                    = c;
	state->d                    = d;
	state->detectoravg          = 0.0f;
	state->compgain             = 1.0f;
	state->maxcompdiffdb        = -1.0f;
	state->delaybufsize         = delaybufsize;
	state->delaywritepos        = 0;
	state->delayreadpos         = delaybufsize > 1 ? 1 : 0;
}

// for more information on the adaptive release curve, check out adaptive-release-curve.html demo +
// source code included in this repo
static inline float adaptivereleasecurve(float x, float a, float b, float c, float d){
	// a*x^3 + b*x^2 + c*x + d
	float x2 = x * x;
	return a * x2 * x + b * x2 + c * x + d;
}

static inline float clampf(float v, float min, float max){
	return v < min ? min : (v > max ? max : v);
}

static inline float absf(float v){
	return v < 0.0f ? -v : v;
}

static inline float fixf(float v, float def){
	// fix NaN and infinity values that sneak in... not sure why this is needed, but it is
	if (isnan(v) || isinf(v))
		return def;
	return v;
}

void sf_compressor_process(sf_compressor_state_st *state, int size, sf_sample_st *input,
	sf_sample_st *output){

	// pull out the state into local variables
	float metergain            = state->metergain;
	float meterrelease         = state->meterrelease;
	float threshold            = state->threshold;
	float knee                 = state->knee;
	float linearpregain        = state->linearpregain;
	float linearthreshold      = state->linearthreshold;
	float slope                = state->slope;
	float attacksamplesinv     = state->attacksamplesinv;
	float satreleasesamplesinv = state->satreleasesamplesinv;
	float wet                  = state->wet;
	float dry                  = state->dry;
	float k                    = state->k;
	float kneedboffset         = state->kneedboffset;
	float linearthresholdknee  = state->linearthresholdknee;
	float mastergain           = state->mastergain;
	float a                    = state->a;
	float b                    = state->b;
	float c                    = state->c;
	float d                    = state->d;
	float detectoravg          = state->detectoravg;
	float compgain             = state->compgain;
	float maxcompdiffdb        = state->maxcompdiffdb;
	int delaybufsize           = state->delaybufsize;
	int delaywritepos          = state->delaywritepos;
	int delayreadpos           = state->delayreadpos;
	sf_sample_st *delaybuf     = state->delaybuf;

	int samplesperchunk = SF_COMPRESSOR_SPU;
	int chunks = size / samplesperchunk;
	float ang90 = (float)M_PI * 0.5f;
	float ang90inv = 2.0f / (float)M_PI;
	int samplepos = 0;
	float spacingdb = SF_COMPRESSOR_SPACINGDB;

	for (int ch = 0; ch < chunks; ch++){
		detectoravg = fixf(detectoravg, 1.0f);
		float desiredgain = detectoravg;
		float scaleddesiredgain = asinf(desiredgain) * ang90inv;
		float compdiffdb = lin2db(compgain / scaleddesiredgain);

		// calculate envelope rate based on whether we're attacking or releasing
		float enveloperate;
		if (compdiffdb < 0.0f){ // compgain < scaleddesiredgain, so we're releasing
			compdiffdb = fixf(compdiffdb, -1.0f);
			maxcompdiffdb = -1; // reset for a future attack mode
			// apply the adaptive release curve
			// scale compdiffdb between 0-3
			float x = (clampf(compdiffdb, -12.0f, 0.0f) + 12.0f) * 0.25f;
			float releasesamples = adaptivereleasecurve(x, a, b, c, d);
			enveloperate = db2lin(spacingdb / releasesamples);
		}
		else{ // compresorgain > scaleddesiredgain, so we're attacking
			compdiffdb = fixf(compdiffdb, 1.0f);
			if (maxcompdiffdb == -1 || maxcompdiffdb < compdiffdb)
				maxcompdiffdb = compdiffdb;
			float attenuate = maxcompdiffdb;
			if (attenuate < 0.5f)
				attenuate = 0.5f;
			enveloperate = 1.0f - powf(0.25f / attenuate, attacksamplesinv);
		}

		// process the chunk
		for (int chi = 0; chi < samplesperchunk; chi++, samplepos++,
			delayreadpos = (delayreadpos + 1) % delaybufsize,
			delaywritepos = (delaywritepos + 1) % delaybufsize){

			float inputL = input[samplepos].L * linearpregain;
			float inputR = input[samplepos].R * linearpregain;
			delaybuf[delaywritepos] = (sf_sample_st){ .L = inputL, .R = inputR };

			inputL = absf(inputL);
			inputR = absf(inputR);
			float inputmax = inputL > inputR ? inputL : inputR;

			float attenuation;
			if (inputmax < 0.0001f)
				attenuation = 1.0f;
			else{
				float inputcomp = compcurve(inputmax, k, slope, linearthreshold,
					linearthresholdknee, threshold, knee, kneedboffset);
				attenuation = inputcomp / inputmax;
			}

			float rate;
			if (attenuation > detectoravg){ // if releasing
				float attenuationdb = -lin2db(attenuation);
				if (attenuationdb < 2.0f)
					attenuationdb = 2.0f;
				float dbpersample = attenuationdb * satreleasesamplesinv;
				rate = db2lin(dbpersample) - 1.0f;
			}
			else
				rate = 1.0f;

			detectoravg += (attenuation - detectoravg) * rate;
			if (detectoravg > 1.0f)
				detectoravg = 1.0f;
			detectoravg = fixf(detectoravg, 1.0f);

			if (enveloperate < 1) // attack, reduce gain
				compgain += (scaleddesiredgain - compgain) * enveloperate;
			else{ // release, increase gain
				compgain *= enveloperate;
				if (compgain > 1.0f)
					compgain = 1.0f;
			}

			// the final gain value!
			float premixgain = sinf(ang90 * compgain);
			float gain = dry + wet * mastergain * premixgain;

			// calculate metering (not used in core algo, but used to output a meter if desired)
			float premixgaindb = lin2db(premixgain);
			if (premixgaindb < metergain)
				metergain = premixgaindb; // spike immediately
			else
				metergain += (premixgaindb - metergain) * meterrelease; // fall slowly

			// apply the gain
			output[samplepos] = (sf_sample_st){
				.L = delaybuf[delayreadpos].L * gain,
				.R = delaybuf[delayreadpos].R * gain
			};
		}
	}

	state->metergain     = metergain;
	state->detectoravg   = detectoravg;
	state->compgain      = compgain;
	state->maxcompdiffdb = maxcompdiffdb;
	state->delaywritepos = delaywritepos;
	state->delayreadpos  = delayreadpos;
}
