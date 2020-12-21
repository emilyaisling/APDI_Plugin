//
//  EffectPlugin.cpp
//  MyEffect Plugin Source Code
//
//  Used to define the bodies of functions used by the plugin, as declared in EffectPlugin.h.
//

#include "EffectPlugin.h"

////////////////////////////////////////////////////////////////////////////
// EFFECT - represents the whole effect plugin
////////////////////////////////////////////////////////////////////////////

// Called to create the effect (used to add your effect to the host plugin)
extern "C" {
    CREATE_FUNCTION createEffect(float sampleRate) {
        ::stk::Stk::setSampleRate(sampleRate);
        
        //==========================================================================
        // CONTROLS - Use this array to completely specify your UI
        // - tells the system what parameters you want, and how they are controlled
        // - add or remove parameters by adding or removing entries from the list
        // - each control should have an expressive label / caption
        // - controls can be of different types: ROTARY, BUTTON, TOGGLE, SLIDER, or MENU (see definitions)
        // - for rotary and linear sliders, you can set the range of values (make sure the initial value is inside the range)
        // - for menus, replace the three numeric values with a single array of option strings: e.g. { "one", "two", "three" }
        // - by default, the controls are laid out in a grid, but you can also move and size them manually
        //   i.e. replace AUTO_SIZE with Bounds(50,50,100,100) to place a 100x100 control at (50,50)
        
        const Parameters CONTROLS = {
            //  name,       type,              min, max, initial, size
            {   "Rate",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Depth",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Feedback",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "LPF Cutoff",  Parameter::ROTARY, 0.0, 1.0, 0.0, AUTO_SIZE  },
            {   "Voices",  Parameter::MENU, {"1", "2"}, AUTO_SIZE  }
        };

        const Presets PRESETS = {
            { "Preset 1", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 2", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
            { "Preset 3", { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },
        };

        return (APDI::Effect*)new MyEffect(CONTROLS, PRESETS);
    }
}

// Constructor: called when the effect is first created / loaded
MyEffect::MyEffect(const Parameters& parameters, const Presets& presets)
: Effect(parameters, presets)
{
    // Initialise member variables, etc.
}

// Destructor: called when the effect is terminated / unloaded
MyEffect::~MyEffect()
{
    // Put your own additional clean up code here (e.g. free memory)
}

// EVENT HANDLERS: handle different user input (button presses, preset selection, drop menus)

void MyEffect::presetLoaded(int iPresetNum, const char *sPresetName)
{
    // A preset has been loaded, so you could perform setup, such as retrieving parameter values
    // using getParameter and use them to set state variables in the plugin
}

void MyEffect::optionChanged(int iOptionMenu, int iItem)
{
    // An option menu, with index iOptionMenu, has been changed to the entry, iItem
}

void MyEffect::buttonPressed(int iButton)
{
    // A button, with index iButton, has been pressed
}

// Applies audio processing to a buffer of audio
// (inputBuffer contains the input audio, and processed samples should be stored in outputBuffer)
void MyEffect::process(const float** inputBuffers, float** outputBuffers, int numSamples)
{
    float fIn0, fIn1, fOut0 = 0, fOut1 = 0, fMix = 0;
    const float *pfInBuffer0 = inputBuffers[0], *pfInBuffer1 = inputBuffers[1];
    float *pfOutBuffer0 = outputBuffers[0], *pfOutBuffer1 = outputBuffers[1];
    
    // Slider values
    float fRate = (parameters[0] * parameters[0] * parameters[0] * 0.09) + 0.01;
    float fDepth = (parameters[1] * parameters[1] * parameters[1] * 0.03)  + 0.02;
    float fFBGain = parameters[2] * 0.5;
    int iVoiceNum = parameters[3] + 1;
    
    // Delay values
    float fDelSig = 0;
    float fVoiceGain = 0.5;

    while(numSamples--)
    {
        // Get sample from input
        fIn0 = *pfInBuffer0++;
        fIn1 = *pfInBuffer1++;
        
        // Add your effect processing here
        fMix = (fIn0 + fIn1) * 0.5f;

        voice1.voiceInit(fRate, fDepth, 0);
        voice2.voiceInit(fRate, fDepth, 0);

        fDelSig += voice1.process() * fVoiceGain;
        fDelSig += voice2.process() * fVoiceGain;
        
        fOut0 = fMix + (fDelSig * fFBGain);
        
        voice1.voiceFB(fOut0);
        voice2.voiceFB(fOut0);
        
        // Copy result to output
        *pfOutBuffer0++ = fOut0;
        *pfOutBuffer1++ = fOut0;
    }
}
