//
// MIT license
//
// Copyright (c) 2019 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#include "AlsaPlayer.h"
#include "wav.h"

#include <string>
#include <vector>

AlsaPlayer::AlsaPlayer():
    mPCMHandle(nullptr),
    mUpdatePeriod(0),
    mChannelsCount(0),
    mBitsPerSample(0),
    mSamplesPerSecond(0)
{
}

AlsaPlayer::~AlsaPlayer()
{
    Close();
}

/**
 *******************************************************************************
 * @fn Init
 * @brief Will init alsa
 *******************************************************************************
 */
PlayerError AlsaPlayer::Init(
    uint16_t channelsCount,
    uint16_t bitsPerSample,
    uint32_t samplesPerSecond,
    bool play,
    bool record
    )
{
    if(!play && !record)
    {
        std::cerr << "Error: player must has play or record flags set!" << std::endl;

        return PlayerError::InvalidParameters;
    }

    std::vector<std::string> devices;
    int pcmError(0);

    //Start with first card
    int cardNum = -1;

    for(;;)
    {
        snd_ctl_t *cardHandle(nullptr);

        if((pcmError = snd_card_next(&cardNum)) < 0)
        {
            std::cerr << "Can't get the next card number: %s" << snd_strerror(pcmError) << std::endl;

            break;
        }

        if(cardNum < 0)
        {
            break;
        }

        // Open this card's control interface. We specify only the card number -- not
        // any device nor sub-device too
        {
            std::string cardName("hw:");
            cardName += std::to_string(cardNum);

            if((pcmError = snd_ctl_open(&cardHandle, cardName.c_str(), 0)) < 0)
            {
                std::cerr << "Can't open card " << cardNum << ": " << snd_strerror(pcmError) << std::endl;

                continue;
            }

            devices.push_back(cardName);
        }

        {
            snd_ctl_card_info_t *cardInfo(nullptr);

            // We need to get a snd_ctl_card_info_t. Just alloc it on the stack
            snd_ctl_card_info_alloca(&cardInfo);

            // Tell ALSA to fill in our snd_ctl_card_info_t with info about this card
            if((pcmError = snd_ctl_card_info(cardHandle, cardInfo)) < 0)
            {
                std::cerr << "Can't get info for card " << cardNum << ": " << snd_strerror(pcmError) << std::endl;
            }
            else
            {
                std::cout << "Card " << cardNum << " = " << snd_ctl_card_info_get_name(cardInfo) << std::endl;
            }
        }

        // Close the card's control interface after we're done with it
        snd_ctl_close(cardHandle);
    }

    if(devices.empty())
    {
        std::cerr << "Error: No compatible sound card found." << std::endl;

        return PlayerError::PCMError;
    }

    snd_pcm_stream_t openMode(
        snd_pcm_stream_t(
            play && record
                ? SND_PCM_STREAM_PLAYBACK | SND_PCM_STREAM_CAPTURE
                : (play ? SND_PCM_STREAM_PLAYBACK : SND_PCM_STREAM_CAPTURE)
            )
        );

    /* Open the PCM device in playback mode */
	if(pcmError = snd_pcm_open(
        &mPCMHandle,
        "default",
        openMode,
        0//SND_PCM_ASYNC | SND_PCM_NONBLOCK
        ) < 0)
    {
        std::cerr << "Error: Can't open default PCM device. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }
    else
    {
        std::cout << "PCM device opened: " << devices[0] << std::endl;
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_t *params(nullptr);
	snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(mPCMHandle, params);

    /* Set parameters */
	if(pcmError = snd_pcm_hw_params_set_access(
        mPCMHandle,
        params,
        SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
		std::cerr << "Error: Can't set interleaved mode. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }

	if(pcmError = snd_pcm_hw_params_set_format(
        mPCMHandle,
        params,
        SND_PCM_FORMAT_S16_LE) < 0)
    {
		std::cerr << "Error: Can't set format. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }

	if(pcmError = snd_pcm_hw_params_set_channels(mPCMHandle, params, channelsCount) < 0)
    {
		std::cerr << "Error: Can't set channels number. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }

	if(pcmError = snd_pcm_hw_params_set_rate_near(mPCMHandle, params, &samplesPerSecond, 0) < 0)
    {
		std::cerr << "Error: Can't set rate. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }

	/* Write parameters */
	if(pcmError = snd_pcm_hw_params(mPCMHandle, params) < 0)
    {
		std::cerr << "Error: Can't set harware parameters. " << snd_strerror(pcmError) << std::endl;

        return PlayerError::PCMError;
    }

	{
        uint32_t channels(0);
	    snd_pcm_hw_params_get_channels(params, &channels);

        uint32_t rate(0);
        snd_pcm_hw_params_get_rate(params, &rate, 0);

        std::cout
            << "PCM name: " << snd_pcm_name(mPCMHandle) << std::endl
            << "PCM state: " << snd_pcm_state_name(snd_pcm_state(mPCMHandle)) << std::endl
            << "Channels count: " << channels << std::endl
            << "Rate: " << rate << std::endl
            ;
    }

    mChannelsCount = channelsCount;
    mBitsPerSample = bitsPerSample;
    mSamplesPerSecond = samplesPerSecond;

    mUpdatePeriod = 0;
    snd_pcm_hw_params_get_period_time(params, &mUpdatePeriod, NULL);

	return PlayerError::OK;
}

/**
 *******************************************************************************
 * @fn Release
 *******************************************************************************
 */
void AlsaPlayer::Close()
{
    if(mPCMHandle)
    {
        snd_pcm_drain(mPCMHandle);
        snd_pcm_close(mPCMHandle);

        mPCMHandle = nullptr;
    }

    // ALSA allocates some mem to load its config file when we call some of the
    // above functions. Now that we're done getting the info, let's tell ALSA
    // to unload the info and free up that mem
    snd_config_update_free_global();
}

/**
 *******************************************************************************
 * @fn Play
 * @brief Play output
 *******************************************************************************
 */
uint32_t AlsaPlayer::Play(uint8_t * outputBuffer, uint32_t size, bool mute)
{
    uint32_t uiFrames2Play(size / mChannelsCount / (mBitsPerSample / 8));

    snd_pcm_sframes_t pcmFramesResult = 0;

    //todo: mute in realtime?
    if(!mute)
    {

        if(-EPIPE == (pcmFramesResult = snd_pcm_writei(mPCMHandle, outputBuffer, uiFrames2Play)))
        {
            snd_pcm_prepare(mPCMHandle);

            return 0;
        }
        else if(pcmFramesResult < 0)
        {
            std::cerr << "Error: Can't write to PCM device. " << snd_strerror(pcmFramesResult) << std::endl;

            return 0;
        }
    }

    return pcmFramesResult * mChannelsCount * mBitsPerSample / 8;
}

/**
*******************************************************************************
* @fn Record
*******************************************************************************
*/
uint32_t AlsaPlayer::Record(uint8_t * outputBuffer, uint32_t size)
{
    uint32_t uiFrames2Play(size / mChannelsCount / (mBitsPerSample / 8));

    int pcmResult = 0;

    //Returns
    //a positive number of frames actually read otherwise a negative error code
    //Return values
    //-EBADFD	PCM is not in the right state (SND_PCM_STATE_PREPARED or SND_PCM_STATE_RUNNING)
    //-EPIPE	an overrun occurred
    //-ESTRPIPE	a suspend event occurred (stream is suspended and waiting for an application recovery)

    if(-EPIPE == (pcmResult = snd_pcm_readi(mPCMHandle, outputBuffer, uiFrames2Play)))
    {
        snd_pcm_prepare(mPCMHandle);

        return 0;
    }
    else if(pcmResult < 0)
    {
        std::cerr << "Error: Can't read from PCM device. " << snd_strerror(pcmResult) << std::endl;

        return 0;
    }

    return pcmResult * mChannelsCount * (mBitsPerSample / 8);
}