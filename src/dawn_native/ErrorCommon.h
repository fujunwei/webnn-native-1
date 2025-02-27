// Copyright 2018 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DAWNNATIVE_ERRORCOMMON_H_
#define DAWNNATIVE_ERRORCOMMON_H_

#include "absl/strings/str_format.h"
#include "common/Result.h"
#include "dawn_native/ErrorData.h"

#include <string>

namespace dawn_native {

    enum class InternalErrorType : uint32_t;

    // MaybeError and ResultOrError are meant to be used as return value for function that are not
    // expected to, but might fail. The handling of error is potentially much slower than successes.
    using MaybeError = Result<void, ErrorData>;

    template <typename T>
    using ResultOrError = Result<T, ErrorData>;

    // Returning a success is done like so:
    //   return {}; // for Error
    //   return SomethingOfTypeT; // for ResultOrError<T>
    //
    // Returning an error is done via:
    //   return DAWN_MAKE_ERROR(errorType, "My error message");
    //
    // but shorthand version for specific error types are preferred:
    //   return DAWN_VALIDATION_ERROR("My error message");
    //
    // There are different types of errors that should be used for different purpose:
    //
    //   - Validation: these are errors that show the user did something bad, which causes the
    //     whole call to be a no-op. It's most commonly found in the frontend but there can be some
    //     backend specific validation in non-conformant backends too.
    //
    //   - Out of memory: creation of a Buffer or Texture failed because there isn't enough memory.
    //     This is similar to validation errors in that the call becomes a no-op and returns an
    //     error object, but is reported separated from validation to the user.
    //
    //   - Device loss: the backend driver reported that the GPU has been lost, which means all
    //     previous commands magically disappeared and the only thing left to do is clean up.
    //     Note: Device loss should be used rarely and in most case you want to use Internal
    //     instead.
    //
    //   - Internal: something happened that the backend didn't expect, and it doesn't know
    //     how to recover from that situation. This causes the device to be lost, but is separate
    //     from device loss, because the GPU execution is still happening so we need to clean up
    //     more gracefully.
    //
    //   - Unimplemented: same as Internal except it puts "unimplemented" in the error message for
    //     more clarity.

#define DAWN_MAKE_ERROR(TYPE, MESSAGE) \
    ::dawn_native::ErrorData::Create(TYPE, MESSAGE, __FILE__, __func__, __LINE__)

#define DAWN_VALIDATION_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::Validation, MESSAGE)

// TODO(dawn:563): Rename to DAWN_VALIDATION_ERROR once all message format strings have been
// converted to constexpr.
#define DAWN_FORMAT_VALIDATION_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__))

#define DAWN_INVALID_IF(EXPR, ...)                                                           \
    if (DAWN_UNLIKELY(EXPR)) {                                                               \
        return DAWN_MAKE_ERROR(InternalErrorType::Validation, absl::StrFormat(__VA_ARGS__)); \
    }                                                                                        \
    for (;;)                                                                                 \
    break

// DAWN_DEVICE_LOST_ERROR means that there was a real unrecoverable native device lost error.
// We can't even do a graceful shutdown because the Device is gone.
#define DAWN_DEVICE_LOST_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::DeviceLost, MESSAGE)

// DAWN_INTERNAL_ERROR means Dawn hit an unexpected error in the backend and should try to
// gracefully shut down.
#define DAWN_INTERNAL_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::Internal, MESSAGE)

#define DAWN_FORMAT_INTERNAL_ERROR(...) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, absl::StrFormat(__VA_ARGS__))

#define DAWN_UNIMPLEMENTED_ERROR(MESSAGE) \
    DAWN_MAKE_ERROR(InternalErrorType::Internal, std::string("Unimplemented: ") + MESSAGE)

// DAWN_OUT_OF_MEMORY_ERROR means we ran out of memory. It may be used as a signal internally in
// Dawn to free up unused resources. Or, it may bubble up to the application to signal an allocation
// was too large or they should free some existing resources.
#define DAWN_OUT_OF_MEMORY_ERROR(MESSAGE) DAWN_MAKE_ERROR(InternalErrorType::OutOfMemory, MESSAGE)

#define DAWN_CONCAT1(x, y) x##y
#define DAWN_CONCAT2(x, y) DAWN_CONCAT1(x, y)
#define DAWN_LOCAL_VAR DAWN_CONCAT2(_localVar, __LINE__)

    // When Errors aren't handled explicitly, calls to functions returning errors should be
    // wrapped in an DAWN_TRY. It will return the error if any, otherwise keep executing
    // the current function.
#define DAWN_TRY(EXPR) DAWN_TRY_WITH_CLEANUP(EXPR, {})

#define DAWN_TRY_CONTEXT(EXPR, ...) \
    DAWN_TRY_WITH_CLEANUP(EXPR, { error->AppendContext(absl::StrFormat(__VA_ARGS__)); })

#define DAWN_TRY_WITH_CLEANUP(EXPR, BODY)                                                    \
    {                                                                                        \
        auto DAWN_LOCAL_VAR = EXPR;                                                          \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR.IsError())) {                                       \
            std::unique_ptr<::dawn_native::ErrorData> error = DAWN_LOCAL_VAR.AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */                  \
            error->AppendBacktrace(__FILE__, __func__, __LINE__);                            \
            return {std::move(error)};                                                       \
        }                                                                                    \
    }                                                                                        \
    for (;;)                                                                                 \
    break

    // DAWN_TRY_ASSIGN is the same as DAWN_TRY for ResultOrError and assigns the success value, if
    // any, to VAR.
#define DAWN_TRY_ASSIGN(VAR, EXPR) DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {})

    // Argument helpers are used to determine which macro implementations should be called when
    // overloading with different number of variables.
#define DAWN_ERROR_UNIMPLEMENTED_MACRO_(...) UNREACHABLE()
#define DAWN_ERROR_GET_5TH_ARG_HELPER_(_1, _2, _3, _4, NAME, ...) NAME
#define DAWN_ERROR_GET_5TH_ARG_(args) DAWN_ERROR_GET_5TH_ARG_HELPER_ args

    // DAWN_TRY_ASSIGN_WITH_CLEANUP is overloaded with 2 version so that users can override the
    // return value of the macro when necessary. This is particularly useful if the function
    // calling the macro may want to return void instead of the error, i.e. in a test where we may
    // just want to assert and fail if the assign cannot go through. In both the cleanup and return
    // clauses, users can use the `error` variable to access the pointer to the acquired error.
    //
    // Example usages:
    //     3 Argument Case:
    //          Result res;
    //          DAWN_TRY_ASSIGN_WITH_CLEANUP(
    //              res, GetResultOrErrorFunction(), { AddAdditionalErrorInformation(error.get()); }
    //          );
    //
    //     4 Argument Case:
    //          bool FunctionThatReturnsBool() {
    //              DAWN_TRY_ASSIGN_WITH_CLEANUP(
    //                  res, GetResultOrErrorFunction(),
    //                  { AddAdditionalErrorInformation(error.get()); },
    //                  false
    //              );
    //          }
#define DAWN_TRY_ASSIGN_WITH_CLEANUP(...)                                       \
    DAWN_ERROR_GET_5TH_ARG_((__VA_ARGS__, DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_, \
                             DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_,              \
                             DAWN_ERROR_UNIMPLEMENTED_MACRO_))                  \
    (__VA_ARGS__)

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_3_(VAR, EXPR, BODY) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, std::move(error))

#define DAWN_TRY_ASSIGN_WITH_CLEANUP_IMPL_4_(VAR, EXPR, BODY, RET)            \
    {                                                                         \
        auto DAWN_LOCAL_VAR = EXPR;                                           \
        if (DAWN_UNLIKELY(DAWN_LOCAL_VAR.IsError())) {                        \
            std::unique_ptr<ErrorData> error = DAWN_LOCAL_VAR.AcquireError(); \
            {BODY} /* comment to force the formatter to insert a newline */   \
            error->AppendBacktrace(__FILE__, __func__, __LINE__);             \
            return (RET);                                                     \
        }                                                                     \
        VAR = DAWN_LOCAL_VAR.AcquireSuccess();                                \
    }                                                                         \
    for (;;)                                                                  \
    break

}  // namespace dawn_native

#endif  // DAWNNATIVE_ERRORCOMMON_H_
