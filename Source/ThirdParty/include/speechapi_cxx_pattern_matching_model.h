//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license201809 for the full license information.
//
// speechapi_cxx_pattern_matching_model.h: Public API declarations for PatternMatchingModel C++ class
//

#pragma once
#include <speechapi_cxx_common.h>
#include <speechapi_cxx_string_helpers.h>
#include <speechapi_cxx_pattern_matching_intent.h>
#include <speechapi_c.h>
#include <spxdebug.h>

namespace Microsoft {
namespace CognitiveServices {
namespace Speech {
namespace Intent {

/// <summary>
/// Represents a pattern matching model used for intent recognition.
/// </summary>
class PatternMatchingModel : public LanguageUnderstandingModel
{
public:

    /// <summary>
    /// Creates a pattern matching model using the specified model ID.
    /// </summary>
    /// <param name="modelId">A string that represents a unique Id for this model.</param>
    /// <returns>A shared pointer to pattern matching model.</returns>
    static std::shared_ptr<PatternMatchingModel> FromModelId(const SPXSTRING& modelId)
    {
        return std::shared_ptr<PatternMatchingModel> {
            new PatternMatchingModel(modelId)
        };
    }

    /// <summary>
    /// Returns id for this model.
    /// </summary>
    /// <returns>A string representing the id of this model.</returns>
    SPXSTRING GetModelId() { return m_modelId; }

    std::vector<PatternMatchingIntent> Intents;
private:
    DISABLE_COPY_AND_MOVE(PatternMatchingModel);

    PatternMatchingModel(const SPXSTRING& modelId) : LanguageUnderstandingModel(LanguageUnderstandingModelType::PatternMatchingModel),m_modelId(modelId) {}

    SPXSTRING m_modelId;
};

} } } } // Microsoft::CognitiveServices::Speech::Intent
