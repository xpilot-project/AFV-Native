/* audio/VHFFilterSource.cpp
 *
 * This file is part of AFV-Native.
 *
 * Copyright (c) 2020 Christopher Collins
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#include "afv-native/audio/VHFFilterSource.h"
#include <SimpleComp.h>

using namespace afv_native::audio;

VHFFilterSource::VHFFilterSource():
    compressor(new chunkware_simple::SimpleComp())
{
    compressor->setSampleRate(sampleRateHz);
    compressor->setAttack(5.0);
    compressor->setRelease(10.0);
    compressor->setThresh(16);
    compressor->setRatio(6);
    compressor->initRuntime();
    compressorPostGain = pow(10.0f, (-5.5/20.0));
    
    setupPresets();
}

VHFFilterSource::~VHFFilterSource()
{
    delete compressor;
};

void VHFFilterSource::setupPresets()
{
    mFilters.push_back(BiQuadFilter::highPassFilter(44100, 310, 0.25));
    mFilters.push_back(BiQuadFilter::peakingEQ(44100, 450, 0.75, 17.0));
    mFilters.push_back(BiQuadFilter::peakingEQ(44100, 1450, 1.0, 25.0));
    mFilters.push_back(BiQuadFilter::peakingEQ(44100, 2000, 1.0, 25.0));
    mFilters.push_back(BiQuadFilter::lowPassFilter(44100, 2500, 0.25));
}

/** transformFrame lets use apply this filter to a normal buffer, without following the sink/source flow.
 *
 * It always performs a copy of the data from In to Out at the very least.
 */
void VHFFilterSource::transformFrame(SampleType *bufferOut, SampleType const bufferIn[]) {
    double sl, sr;
    for (unsigned i = 0; i < frameSizeSamples; i++) {
        sl = bufferIn[i];
        sr = sl;
        compressor->process(sl, sr);
        for (int band = 0; band < mFilters.size(); band++)
        {
            sl = mFilters[band].TransformOne(sl);
        }
        sl *= static_cast<float>(compressorPostGain);
        bufferOut[i] = sl;
    }
}
