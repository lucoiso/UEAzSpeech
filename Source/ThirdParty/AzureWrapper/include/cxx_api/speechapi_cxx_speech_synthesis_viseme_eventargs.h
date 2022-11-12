//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license201809 for the full license information.
//

#pragma once
#include <speechapi_cxx_common.h>
#include <speechapi_cxx_string_helpers.h>
#include <speechapi_cxx_eventargs.h>
#include <speechapi_c_synthesizer.h>

namespace Microsoft {
namespace CognitiveServices {
namespace Speech {

/// <summary>
/// Class for speech synthesis viseme event arguments.
/// Added in version 1.16.0
/// </summary>
class SpeechSynthesisVisemeEventArgs : public EventArgs
{
private:

    SPXEVENTHANDLE m_hEvent;

public:

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="hevent">Event handle</param>
    explicit SpeechSynthesisVisemeEventArgs(SPXEVENTHANDLE hevent) :
        m_hEvent(hevent),
        Animation(m_animation)
    {
        SPX_DBG_TRACE_VERBOSE("%s (this=0x%p, handle=0x%p)", __FUNCTION__, (void*)this, (void*)m_hEvent);
        synthesizer_viseme_event_get_values(hevent, &m_audioOffset, &m_visemeId);
        AudioOffset = m_audioOffset;
        VisemeId = m_visemeId;

        m_animation = Utils::ToSPXString(Utils::CopyAndFreePropertyString(synthesizer_viseme_event_get_animation(hevent)));
    };

    /// <inheritdoc/>
    virtual ~SpeechSynthesisVisemeEventArgs()
    {
        SPX_DBG_TRACE_VERBOSE("%s (this=0x%p, handle=0x%p)", __FUNCTION__, (void*)this, (void*)m_hEvent);
        SPX_THROW_ON_FAIL(synthesizer_event_handle_release(m_hEvent));
    }

    /// <summary>
    /// Audio offset, in ticks (100 nanoseconds).
    /// </summary>
    uint64_t AudioOffset;

    /// <summary>
    /// Viseme ID.
    /// </summary>
    uint32_t VisemeId;

    /// <summary>
    /// Animation, could be svg or other format.
    /// </summary>
    const SPXSTRING& Animation;

private:

    DISABLE_DEFAULT_CTORS(SpeechSynthesisVisemeEventArgs);

    uint64_t m_audioOffset{ 0 };
    uint32_t m_visemeId { 0 };
    SPXSTRING m_animation;
};

} } } // Microsoft::CognitiveServices::Speech
