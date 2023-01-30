//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/csspeech/license for the full license information.
//
// speechapi_c_pattern_matching_model.h: Public API declarations for PatternMatchingModel related C methods and typedefs
//

#pragma once
#include <speechapi_c_common.h>

SPXAPI_(bool) pattern_matching_model_handle_is_valid(SPXLUMODELHANDLE hlumodel);

SPXAPI pattern_matching_model_create_from_id(SPXLUMODELHANDLE* hlumodel, const char* id);

SPXAPI pattern_matching_model__handle_release(SPXLUMODELHANDLE hlumodel);
