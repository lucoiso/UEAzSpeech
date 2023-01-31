//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license for the full license information.
//
// speechapi_cxx_intent_recognizer.h: Public API declarations for IntentRecognizer C++ class
//

#pragma once
#include <speechapi_cxx_common.h>
#include <speechapi_cxx_string_helpers.h>
#include <speechapi_c.h>
#include "speechapi_c_json.h"
#include <speechapi_cxx_conversational_language_understanding_model.h>
#include <speechapi_cxx_recognizer.h>
#include <speechapi_cxx_intent_recognition_result.h>
#include <speechapi_cxx_intent_recognition_eventargs.h>
#include <speechapi_cxx_intent_trigger.h>
#include <speechapi_cxx_pattern_matching_model.h>
#include <speechapi_cxx_properties.h>
#include <speechapi_cxx_speech_config.h>
#include <speechapi_cxx_audio_stream.h>

namespace Microsoft {
namespace CognitiveServices {
namespace Speech {
namespace Intent {

/// <summary>
/// In addition to performing speech-to-text recognition, the IntentRecognizer extracts structured information
/// about the intent of the speaker, which can be used to drive further actions using dedicated intent triggers
/// (see <see cref="IntentTrigger"/>).
/// </summary>
    class IntentRecognizer : public AsyncRecognizer<IntentRecognitionResult, IntentRecognitionEventArgs, IntentRecognitionCanceledEventArgs>
    {
    public:

        using BaseType = AsyncRecognizer<IntentRecognitionResult, IntentRecognitionEventArgs, IntentRecognitionCanceledEventArgs>;

        /// <summary>
        /// Creates an intent recognizer from a speech config and an audio config.
        /// Users should use this function to create a new instance of an intent recognizer.
        /// </summary>
        /// <param name="speechConfig">Speech configuration.</param>
        /// <param name="audioInput">Audio configuration.</param>
        /// <returns>Instance of intent recognizer.</returns>
        static std::shared_ptr<IntentRecognizer> FromConfig(std::shared_ptr<SpeechConfig> speechConfig, std::shared_ptr<Audio::AudioConfig> audioInput = nullptr)
        {
            SPXRECOHANDLE hreco;
            SPX_THROW_ON_FAIL(::recognizer_create_intent_recognizer_from_config(
                &hreco,
                HandleOrInvalid<SPXSPEECHCONFIGHANDLE, SpeechConfig>(speechConfig),
                HandleOrInvalid<SPXAUDIOCONFIGHANDLE, Audio::AudioConfig>(audioInput)));
            return std::make_shared<IntentRecognizer>(hreco);
        }

        /// <summary>
        /// Creates an intent recognizer from an embedded speech config and an audio config.
        /// Users should use this function to create a new instance of an intent recognizer.
        /// Added in version 1.19.0
        /// </summary>
        /// <param name="speechConfig">Embedded speech configuration.</param>
        /// <param name="audioInput">Audio configuration.</param>
        /// <returns>Instance of intent recognizer.</returns>
        static std::shared_ptr<IntentRecognizer> FromConfig(std::shared_ptr<EmbeddedSpeechConfig> speechConfig, std::shared_ptr<Audio::AudioConfig> audioInput = nullptr)
        {
            SPXRECOHANDLE hreco;
            SPX_THROW_ON_FAIL(::recognizer_create_intent_recognizer_from_config(
                &hreco,
                HandleOrInvalid<SPXSPEECHCONFIGHANDLE, EmbeddedSpeechConfig>(speechConfig),
                HandleOrInvalid<SPXAUDIOCONFIGHANDLE, Audio::AudioConfig>(audioInput)));
            return std::make_shared<IntentRecognizer>(hreco);
        }

        /// <summary>
        /// Internal constructor. Creates a new instance using the provided handle.
        /// </summary>
        /// <param name="hreco">Recognizer handle.</param>
        explicit IntentRecognizer(SPXRECOHANDLE hreco) : BaseType(hreco), Properties(m_properties)
        {
            SPX_DBG_TRACE_SCOPE(__FUNCTION__, __FUNCTION__);
        }

        /// <summary>
        /// destructor
        /// </summary>
        ~IntentRecognizer()
        {
            SPX_DBG_TRACE_SCOPE(__FUNCTION__, __FUNCTION__);
            TermRecognizer();
        }

        /// <summary>
        /// Starts intent recognition, and returns after a single utterance is recognized. The end of a
        /// single utterance is determined by listening for silence at the end or until a maximum of 15
        /// seconds of audio is processed.  The task returns the recognition text as result.
        /// Note: Since RecognizeOnceAsync() returns only a single utterance, it is suitable only for single
        /// shot recognition like command or query.
        /// For long-running multi-utterance recognition, use StartContinuousRecognitionAsync() instead..
        /// </summary>
        /// <returns>Future containing result value (a shared pointer to IntentRecognitionResult)
        /// of the asynchronous intent recognition.
        /// </returns>
        std::future<std::shared_ptr<IntentRecognitionResult>> RecognizeOnceAsync() override
        {
            return BaseType::RecognizeOnceAsyncInternal();
        }

        /// <summary>
        /// Starts intent recognition, and generates a result from the text passed in. This is useful for testing and other times when the speech input
        /// is not tied to the IntentRecognizer.
        /// Note: The Intent Service does not currently support this so it is only valid for offline pattern matching or exact matching intents.
        /// </summary>
        /// <param name="text">The text to be evaluated. </param>
        /// <returns>Future containing result value (a shared pointer to IntentRecognitionResult)
        /// of the asynchronous intent recognition.
        /// </returns>
        std::future<std::shared_ptr<IntentRecognitionResult>> RecognizeOnceAsync(SPXSTRING text)
        {
            auto keepAlive = this->shared_from_this();
            auto future = std::async(std::launch::async, [keepAlive, this, text]() -> std::shared_ptr<IntentRecognitionResult> {
                SPX_INIT_HR(hr);

                SPXRESULTHANDLE hresult = SPXHANDLE_INVALID;
                SPX_THROW_ON_FAIL(hr = intent_recognizer_recognize_text_once(m_hreco, Utils::ToUTF8(text).c_str(), &hresult));

                return std::make_shared<IntentRecognitionResult>(hresult);
                });
            return future;
        }

        /// <summary>
        /// Asynchronously initiates continuous intent recognition operation.
        /// </summary>
        /// <returns>An empty future.</returns>
        std::future<void> StartContinuousRecognitionAsync() override
        {
            return BaseType::StartContinuousRecognitionAsyncInternal();
        }

        /// <summary>
        /// Asynchronously terminates ongoing continuous intent recognition operation.
        /// </summary>
        /// <returns>An empty future.</returns>
        std::future<void> StopContinuousRecognitionAsync() override
        {
            return BaseType::StopContinuousRecognitionAsyncInternal();
        }

        /// <summary>
        /// Asynchronously initiates keyword recognition operation.
        /// </summary>
        /// <param name="model">Specifies the keyword model to be used.</param>
        /// <returns>An empty future.</returns>
        std::future<void> StartKeywordRecognitionAsync(std::shared_ptr<KeywordRecognitionModel> model) override
        {
            return BaseType::StartKeywordRecognitionAsyncInternal(model);
        }

        /// <summary>
        /// Asynchronously terminates keyword recognition operation.
        /// </summary>
        /// <returns>An empty future.</returns>
        std::future<void> StopKeywordRecognitionAsync() override
        {
            return BaseType::StopKeywordRecognitionAsyncInternal();
        }

        /// <summary>
        /// A collection of properties and their values defined for this <see cref="IntentRecognizer"/>.
        /// </summary>
        PropertyCollection& Properties;

        /// <summary>
        /// Adds a simple phrase that may be spoken by the user, indicating a specific user intent.
        /// This simple phrase can be a pattern including and enitity surrounded by braces. Such as "click the {checkboxName} checkbox".
        /// 
        /// </summary>
        /// <param name="simplePhrase">The phrase corresponding to the intent.</param>
        /// <remarks>Once recognized, the IntentRecognitionResult's IntentId property will match the simplePhrase specified here.
        /// If any entities are specified and matched, they will be available in the IntentResult->GetEntities() call.
        /// </remarks>
        void AddIntent(const SPXSTRING& simplePhrase)
        {
            auto trigger = IntentTrigger::From(simplePhrase);
            return AddIntent(trigger, simplePhrase);
        }

        /// <summary>
        /// Adds a simple phrase that may be spoken by the user, indicating a specific user intent.
        /// This simple phrase can be a pattern including and enitity surrounded by braces. Such as "click the {checkboxName} checkbox".
        /// </summary>
        /// <param name="simplePhrase">The phrase corresponding to the intent.</param>
        /// <param name="intentId">A custom id string to be returned in the IntentRecognitionResult's IntentId property.</param>
        /// <remarks>Once recognized, the result's intent id will match the id supplied here.
        /// If any entities are specified and matched, they will be available in the IntentResult->GetEntities() call.
        /// </remarks>
        void AddIntent(const SPXSTRING& simplePhrase, const SPXSTRING& intentId)
        {
            auto trigger = IntentTrigger::From(simplePhrase);
            return AddIntent(trigger, intentId);
        }

        /// <summary>
        /// Adds a single intent by name from the specified Language Understanding Model.
        /// For PatternMatchingModel and ConversationalLanguageUnderstandingModel types, this will clear
        /// any existing models before enabling it. For these types, the intentName is ignored.
        /// </summary>
        /// <param name="model">The language understanding model containing the intent.</param>
        /// <param name="intentName">The name of the single intent to be included from the language understanding model.</param>
        /// <remarks>Once recognized, the IntentRecognitionResult's IntentId property will contain the intentName specified here.</remarks>
        void AddIntent(std::shared_ptr<LanguageUnderstandingModel> model, const SPXSTRING& intentName)
        {
            switch (model->GetModelType())
            {
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::LanguageUnderstandingModel:
                {
                    auto trigger = IntentTrigger::From(model, intentName);
                    AddIntent(trigger, intentName);
                    break;
                }
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::PatternMatchingModel:
                {
                    intent_recognizer_clear_language_models(m_hreco);
                    auto jsonBuild = BuildModelJson(model);
                    intent_recognizer_import_pattern_matching_model(m_hreco, jsonBuild.c_str());
                    break;
                }
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::ConversationalLanguageUnderstandingModel:
                {
                    intent_recognizer_clear_language_models(m_hreco);
                    auto cluModel = static_cast<const ConversationalLanguageUnderstandingModel*>(model.get());
                    intent_recognizer_add_conversational_language_understanding_model(
                        m_hreco,
                        cluModel->languageResourceKey.c_str(),
                        cluModel->endpoint.c_str(),
                        cluModel->projectName.c_str(),
                        cluModel->deploymentName.c_str());
                    break;
                }
                default:
                    break;
            }
        }

        /// <summary>
        /// Adds a single intent by name from the specified Language Understanding Model.
        /// For PatternMatchingModel and ConversationalLanguageUnderstandingModel types, this will clear
        /// any existing models before enabling it. For these types, the intentName and intentId are ignored.
        /// </summary>
        /// <param name="model">The language understanding model containing the intent.</param>
        /// <param name="intentName">The name of the single intent to be included from the language understanding model.</param>
        /// <param name="intentId">A custom id string to be returned in the IntentRecognitionResult's IntentId property.</param>
        void AddIntent(std::shared_ptr<LanguageUnderstandingModel> model, const SPXSTRING& intentName, const SPXSTRING& intentId)
        {
            switch (model->GetModelType())
            {
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::LanguageUnderstandingModel:
                {
                    auto trigger = IntentTrigger::From(model, intentName);
                    AddIntent(trigger, intentId);
                    break;
                }
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::PatternMatchingModel:
                {
                    intent_recognizer_clear_language_models(m_hreco);
                    auto jsonBuild = BuildModelJson(model);
                    intent_recognizer_import_pattern_matching_model(m_hreco, jsonBuild.c_str());
                    break;
                }
                case LanguageUnderstandingModel::LanguageUnderstandingModelType::ConversationalLanguageUnderstandingModel:
                {
                    intent_recognizer_clear_language_models(m_hreco);
                    auto cluModel = static_cast<const ConversationalLanguageUnderstandingModel*>(model.get());
                    intent_recognizer_add_conversational_language_understanding_model(
                        m_hreco,
                        cluModel->languageResourceKey.c_str(),
                        cluModel->endpoint.c_str(),
                        cluModel->projectName.c_str(),
                        cluModel->deploymentName.c_str());
                    break;
                }
                default:
                    break;
            }
        }

    /// <summary>
    /// Adds all intents from the specified Language Understanding Model.
    /// For PatternMatchingModel and ConversationalLanguageUnderstandingModel types, this will clear
    /// any existing models before enabling it.
    /// </summary>
    /// <param name="model">The language understanding model containing the intents.</param>
    /// <remarks>Once recognized, the IntentRecognitionResult's IntentId property will contain the name of the intent recognized.</remarks>
    void AddAllIntents(std::shared_ptr<LanguageUnderstandingModel> model)
    {
        switch (model->GetModelType())
        {
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::LanguageUnderstandingModel:
            {
                auto trigger = IntentTrigger::From(model);
                AddIntent(trigger, SPXSTRING_EMPTY);
                break;
            }
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::PatternMatchingModel:
            {
                intent_recognizer_clear_language_models(m_hreco);
                auto jsonBuild = BuildModelJson(model);
                intent_recognizer_import_pattern_matching_model(m_hreco, jsonBuild.c_str());
                break;
            }
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::ConversationalLanguageUnderstandingModel:
            {
                intent_recognizer_clear_language_models(m_hreco);
                auto cluModel = static_cast<const ConversationalLanguageUnderstandingModel*>(model.get());
                intent_recognizer_add_conversational_language_understanding_model(
                    m_hreco,
                    cluModel->languageResourceKey.c_str(),
                    cluModel->endpoint.c_str(),
                    cluModel->projectName.c_str(),
                    cluModel->deploymentName.c_str());
                break;
            }
            default:
                break;
            }
    }

    /// <summary>
    /// Adds all intents from the specified Language Understanding Model.
    /// For PatternMatchingModel and ConversationalLanguageUnderstandingModel types, this will clear
    /// any existing models before enabling it.
    /// </summary>
    /// <param name="model">The language understanding model containing the intents.</param>
    /// <param name="intentId">A custom string id to be returned in the IntentRecognitionResult's IntentId property.</param>
    void AddAllIntents(std::shared_ptr<LanguageUnderstandingModel> model, const SPXSTRING& intentId)
    {
        switch (model->GetModelType())
        {
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::LanguageUnderstandingModel:
            {
                auto trigger = IntentTrigger::From(model);
                AddIntent(trigger, intentId);
                break;
            }
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::PatternMatchingModel:
            {
                intent_recognizer_clear_language_models(m_hreco);
                auto jsonBuild = BuildModelJson(model);
                intent_recognizer_import_pattern_matching_model(m_hreco, jsonBuild.c_str());
                break;
            }
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::ConversationalLanguageUnderstandingModel:
            {
                intent_recognizer_clear_language_models(m_hreco);
                auto cluModel = static_cast<const ConversationalLanguageUnderstandingModel*>(model.get());
                intent_recognizer_add_conversational_language_understanding_model(
                    m_hreco,
                    cluModel->languageResourceKey.c_str(),
                    cluModel->endpoint.c_str(),
                    cluModel->projectName.c_str(),
                    cluModel->deploymentName.c_str());
                break;
            }
            default:
                break;
        }
    }

    /// <summary>
    /// Adds the IntentTrigger specified.
    /// </summary>
    /// <param name="trigger">The IntentTrigger corresponding to the intent.</param>
    /// <param name="intentId">A custom string id to be returned in the IntentRecognitionResult's IntentId property.</param>
    void AddIntent(std::shared_ptr<IntentTrigger> trigger, const SPXSTRING& intentId)
    {
        SPX_THROW_ON_FAIL(intent_recognizer_add_intent(m_hreco, Utils::ToUTF8(intentId).c_str(), (SPXTRIGGERHANDLE)(*trigger.get())));
    }

    /// <summary>
    /// Sets the authorization token that will be used for connecting to the service.
    /// Note: The caller needs to ensure that the authorization token is valid. Before the authorization token
    /// expires, the caller needs to refresh it by calling this setter with a new valid token.
    /// Otherwise, the recognizer will encounter errors during recognition.
    /// </summary>
    /// <param name="token">A string that represents the authorization token.</param>
    void SetAuthorizationToken(const SPXSTRING& token)
    {
        Properties.SetProperty(PropertyId::SpeechServiceAuthorization_Token, token);
    }

    /// <summary>
    /// Gets the authorization token.
    /// </summary>
    /// <returns>Authorization token</returns>
    SPXSTRING GetAuthorizationToken()
    {
        return Properties.GetProperty(PropertyId::SpeechServiceAuthorization_Token, SPXSTRING());
    }

    /// <summary>
    /// Takes a collection of language understanding models, makes a copy of them, and applies them to the recognizer. This application
    /// happens at different times depending on the language understanding model type.
    /// Simple Language Models will become active almost immediately whereas
    /// language understanding models utilizing LUIS will become active on the next Speech turn.
    /// This replaces any previously applied models.
    /// </summary>
    /// <param name="collection">A vector of shared pointers to LanguageUnderstandingModels.</param>
    /// <returns>True if the application of the models takes effect immediately. Otherwise false.</returns>
    bool ApplyLanguageModels(std::vector<std::shared_ptr<LanguageUnderstandingModel>> collection)
    {
        bool result = true;
        SPXTRIGGERHANDLE htrigger = SPXHANDLE_INVALID;

        // Clear existing language models.
        SPX_THROW_ON_FAIL(intent_recognizer_clear_language_models(m_hreco));

        // Add the new ones.
        for (auto model : collection)
        {
            switch (model->GetModelType())
            {
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::LanguageUnderstandingModel:
                SPX_THROW_ON_FAIL(intent_trigger_create_from_language_understanding_model(&htrigger, static_cast<SPXLUMODELHANDLE>(*model), nullptr));
                intent_recognizer_add_intent(m_hreco, nullptr, htrigger);
                result = false;
                break;
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::PatternMatchingModel:
            {
                auto jsonBuild = BuildModelJson(model);
                intent_recognizer_import_pattern_matching_model(m_hreco, jsonBuild.c_str());
                break;
            }
            case LanguageUnderstandingModel::LanguageUnderstandingModelType::ConversationalLanguageUnderstandingModel:
            {
                auto cluModel = static_cast<const ConversationalLanguageUnderstandingModel*>(model.get());
                intent_recognizer_add_conversational_language_understanding_model(
                    m_hreco,
                    cluModel->languageResourceKey.c_str(),
                    cluModel->endpoint.c_str(),
                    cluModel->projectName.c_str(),
                    cluModel->deploymentName.c_str());
                break;
            }
            default:
                break;
            }
            
        }
        return result;
    }

private:

    std::string BuildModelJson(std::shared_ptr<LanguageUnderstandingModel> model)
    {
        SPXHANDLE builder = SPXHANDLE_INVALID;
        auto root = ai_core_json_builder_create(&builder, "", 0);
        auto simpleModel = static_cast<const PatternMatchingModel*>(model.get());

        auto modelIdItem = ai_core_json_builder_item_add(builder, root, 0, "modelId");
        auto modelIdString = model->GetModelId();
        SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, modelIdItem, nullptr, 0, 34, modelIdString.c_str(), modelIdString.length(), false, 0, 0));
        
        auto intentsArrayRootItem = ai_core_json_builder_item_add(builder, root, 0, "intents");
        SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, intentsArrayRootItem, "[]", 2, 91, nullptr, 0, false, 0, 0));

        unsigned int index = 0;
        for (auto& intent : simpleModel->Intents)
        {
            auto intentsArrayItem = ai_core_json_builder_item_add(builder, intentsArrayRootItem, index, nullptr);
            auto intentItem = ai_core_json_builder_item_add(builder, intentsArrayItem, 0, "id");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, intentItem, nullptr, 0, 34, intent.Id.c_str(), intent.Id.length(), false, 0, 0));
            intentItem = ai_core_json_builder_item_add(builder, intentsArrayItem, 0, "priority");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, intentItem, nullptr, 0, 49, nullptr, 0, false, 0, 0));

            unsigned int phraseIndex = 0;
            auto phrasesArrayRootItem = ai_core_json_builder_item_add(builder, intentsArrayItem, 0, "phrases");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, phrasesArrayRootItem, "[]", 2, 91, nullptr, 0, false, 0, 0));
            for (auto& phrase : intent.Phrases)
            {
                auto phrasesArrayItem = ai_core_json_builder_item_add(builder, phrasesArrayRootItem, phraseIndex, nullptr);
                SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, phrasesArrayItem, nullptr, 0, 34, phrase.c_str(), phrase.length(), false, 0, 0));
                phraseIndex++;
            }
            index++;
        }

        index = 0;
        auto entitiesArrayRootItem = ai_core_json_builder_item_add(builder, root, 0, "entities");
        SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, entitiesArrayRootItem, "[]", 2, 91, nullptr, 0, false, 0, 0));
        for (auto& entity : simpleModel->Entities)
        {
            auto entitiesArrayItem = ai_core_json_builder_item_add(builder, entitiesArrayRootItem, index, nullptr);
            auto entitityItem = ai_core_json_builder_item_add(builder, entitiesArrayItem, 0, "id");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, entitityItem, nullptr, 0, 34, entity.Id.c_str(), entity.Id.length(), false, 0, 0));

            entitityItem = ai_core_json_builder_item_add(builder, entitiesArrayItem, 0, "type");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, entitityItem, nullptr, 0, 49, nullptr, 0, false, (unsigned int)entity.Type, 0));

            entitityItem = ai_core_json_builder_item_add(builder, entitiesArrayItem, 0, "mode");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, entitityItem, nullptr, 0, 49, nullptr, 0, false, (unsigned int)entity.Mode, 0));

            unsigned int phraseIndex = 0;
            auto phrasesArrayRootItem = ai_core_json_builder_item_add(builder, entitiesArrayItem, 0, "phrases");
            SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, phrasesArrayRootItem, "[]", 2, 91, nullptr, 0, false, 0, 0));
            for (auto& phrase : entity.Phrases)
            {
                auto phrasesArrayItem = ai_core_json_builder_item_add(builder, phrasesArrayRootItem, phraseIndex, nullptr);
                SPX_THROW_ON_FAIL(ai_core_json_builder_item_set(builder, phrasesArrayItem, nullptr, 0, 34, phrase.c_str(), phrase.length(), false, 0, 0));
                phraseIndex++;
            }
            index++;
        }
        auto jsonCharString = ai_core_json_value_as_json_copy(builder, root);
        ai_core_json_builder_handle_release(builder);
        return jsonCharString;
    }

    DISABLE_COPY_AND_MOVE(IntentRecognizer);

    friend class Microsoft::CognitiveServices::Speech::Session;
};


} } } } // Microsoft::CognitiveServices::Speech::Intent
