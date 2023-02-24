#include "VoiceChannel.h"
#include "../util/logging.h"

void VoiceChannel::stop_play() 
{
    // For a full ADSR pattern, OFF would be usually issued 
    // in the Sustain period, however, it can be issued 
    // at any time (eg. to abort Attack, skip the Decay and Sustain periods, and switch immediately to Release)
    DEBUG("STUB:stop_play_on_voice:" << this->voice_number);
}

void VoiceChannel::start_play() 
{
    // TODO: Starts the ADSR Envelope, and automatically initializes ADSR Volume to zero, 
    // TODO: and copies Voice Start Address to Voice Repeat Address
    DEBUG("STUB:start_play_on_voice:" << this->voice_number);
}