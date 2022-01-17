// Copyright 2021 The WebNN-native Authors
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

#include "webnn_wire/client/NamedOutputs.h"

#include "webnn_wire/WireCmd_autogen.h"
#include "webnn_wire/client/Client.h"

namespace webnn_wire { namespace client {

    void NamedOutputs::Set(char const* name, MLArrayBufferView const* resource) {
        NamedOutputsSetCmd cmd;
        cmd.namedOutputsId = this->id;
        cmd.name = name;
        cmd.buffer = static_cast<const uint8_t*>(resource->buffer);
        cmd.byteLength = resource->byteLength;
        cmd.byteOffset = resource->byteOffset;

        client->SerializeCommand(cmd);
    }

}}  // namespace webnn_wire::client
