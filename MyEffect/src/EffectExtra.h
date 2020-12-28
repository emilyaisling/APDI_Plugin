//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//
//
//  EffectExtra.h
//  Additional Plugin Code
//
//  This file is a workspace for developing new DSP objects or functions to use in your plugin.
//
class MyFilter
{
public:
    void initialise(float coeff)
    {
        a = coeff;
        b = 1 - a;
    }
    
    float process(float input)
    {
        return previous = (a * input) + (b * previous);
    }
    
private:
    float a, b;
    float previous = 0;
};

class MyOscillator
{
public:
    MyOscillator()
    {
        fPhasePos = 0;
    }
    
    float generate(float fRate, float fDepth)
    {
        float fMod = 0;
        float fPhaseInc = (fTwoPI * fRate) / getSampleRate();
        fPhasePos += fPhaseInc;
        if (fPhasePos > fTwoPI)
        {
            fPhasePos -= fTwoPI;
        }
        
        if (fPhasePos < M_PI)
        {
            fMod = -1 + 2/M_PI * fPhasePos;
        }
        else if (fPhasePos >= M_PI)
        {
            fMod = 3 - 2/M_PI * fPhasePos;
        }
        return fMod * fDepth + fDepth;
    }
    
private:
    float fPhasePos;
    const float fTwoPI = 2 * M_PI;
};

class MyDelay
{
public:
    MyDelay()
    {
        iBufferSize = 2 * 192000;
        
        pfCircularBuffer = new float[iBufferSize];
        for (int i = 0; i < iBufferSize; i++)
        {
            pfCircularBuffer[i] = 0;
        }
        
        iBufferWritePos = 0;
        
        delayTimeFilter.initialise(0.0005);
    }
    ~MyDelay()
    {
        delete [] pfCircularBuffer;
    }
    
    int TapPos(float fDelTime)
    {
        float fBufferReadPos = iBufferWritePos - (delayTimeFilter.process(fDelTime) * getSampleRate());
        
        if (fBufferReadPos < 0)
        {
            fBufferReadPos += iBufferSize;
        }
        
        return fBufferReadPos;
    }
    
    float InterpolatedRead(float fBufferReadPos)
    {
        int iPos1, iPos2;
        float fPdiff, fVdiff, fResult;
        
        iPos1 = (int)fBufferReadPos;
        iPos2 = iPos1 + 1;
        if(iPos2 == iBufferSize)
            iPos2 = 0;

        fPdiff = fBufferReadPos - iPos1;
        fVdiff = pfCircularBuffer[iPos2] - pfCircularBuffer[iPos1];
        fResult = pfCircularBuffer[iPos1] + (fVdiff * fPdiff);

        return fResult;
    }
    
    float processSimple(float fIn, float fDelTime, float fDelGain)
    {
        pfCircularBuffer[iBufferWritePos] = fIn;
        iBufferWritePos++;
        if (iBufferWritePos > iBufferSize - 1)
        {
            iBufferWritePos = 0;
        }
        
        return InterpolatedRead(TapPos(fDelTime)) * fDelGain;
    }
    
    float processFeedback(float fFBDelTime, float fFBGain)
    {
        int iBufferReadPos;
        
        iBufferWritePos++;
        if(iBufferWritePos == iBufferSize)
            iBufferWritePos = 0;
        
        iBufferReadPos = iBufferWritePos - (delayTimeFilter.process(fFBDelTime) * getSampleRate());
        if(iBufferReadPos < 0)
            iBufferReadPos += iBufferSize;
        
        return pfCircularBuffer[iBufferReadPos] * fFBGain;
    }
    
    void feedback(float output)
    {
        pfCircularBuffer[iBufferWritePos] = output;
    }
    
private:
    float *pfCircularBuffer;
    int iBufferSize, iBufferWritePos;
    MyFilter delayTimeFilter;
};

class MyVoice
{
public:
    MyVoice()
    {
        fVoiceGain = 0;
        fDelTime = 0;
        fDelSig = 0;
    }

    void voiceInit(float fRate, float fDepth, int i)
    {
        fRate += ((i + 1) * 0.01);
        fDepth += ((i + 1) * 0.01);
        fDelTime = voiceOsc.generate(fRate, fDepth);
    }

    float process()
    {
        fDelSig = voiceDelay.processFeedback(fDelTime, 1);
        return fDelSig;
    }

    void voiceFB(float fOut)
    {
        voiceDelay.feedback(fOut);
    }

private:
    float fVoiceGain, fDelTime, fDelSig;
    MyOscillator voiceOsc;
    MyDelay voiceDelay;
};

