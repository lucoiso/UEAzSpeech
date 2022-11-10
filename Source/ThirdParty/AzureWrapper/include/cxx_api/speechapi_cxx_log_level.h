//
// Copyright (c) Microsoft. All rights reserved.
// See https://aka.ms/azai/vision/license202012 for the full license information.
//

#pragma once

namespace Microsoft {
namespace CognitiveServices {
namespace Speech {
namespace Diagnostics {
namespace Logging {

/// <summary>
/// Defines the different available log levels.
/// </summary>
/// <remarks>
/// This is used by different loggers to set the maximum level of detail they will output.
/// <see cref="MemoryLogger.SetLevel(Level)" />
/// <see cref="EventLogger.SetLevel(Level)" />
/// <see cref="FileLogger.SetLevel(Level)" />
/// </remarks>
enum class Level
{
    Error,
    Warning,
    Info,
    Verbose
};

/*! \cond INTERNAL */
namespace Details
{
    inline const char * LevelToString(Level level)
    {
        switch (level)
        {
        case Level::Error: return "error";
        case Level::Warning: return "warning";
        case Level::Info: return "info";
        default:
        case Level::Verbose: return "verbose";
        }
    }
}
/*! \endcond */

}}}}}