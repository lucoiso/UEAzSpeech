//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license201809 for the full license information.
//
// speechapi_c_simple_language_model.h: Public API declarations for SimpleLanguageModel related C methods and typedefs
//

#pragma once
#include <speechapi_c_common.h>

SPXAPI_(bool) simple_language_understanding_model_handle_is_valid(SPXSLMODELHANDLE hslmodel);

SPXAPI simple_language_understanding_model_create_from_model_id(SPXSLMODELHANDLE* hslmodel, const char* modelId);

SPXAPI simple_language_understanding_model_handle_release(SPXSLMODELHANDLE hslmodel);
