// Copyright 2019 The Dawn Authors
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

#ifndef WEBNN_WIRE_WIRESERVER_H_
#define WEBNN_WIRE_WIRESERVER_H_

#include <memory>

#include "webnn_wire/Wire.h"

struct WebnnProcTable;

namespace webnn_wire {

    namespace server {
        class Server;
        class MemoryTransferService;
    }  // namespace server

    struct WEBNN_WIRE_EXPORT WireServerDescriptor {
        const WebnnProcTable* procs;
        CommandSerializer* serializer;
        server::MemoryTransferService* memoryTransferService = nullptr;
    };

    class WEBNN_WIRE_EXPORT WireServer : public CommandHandler {
      public:
        WireServer(const WireServerDescriptor& descriptor);
        ~WireServer() override;

        const volatile char* HandleCommands(const volatile char* commands,
                                            size_t size) override final;

        bool InjectContext(MLContext context, uint32_t id, uint32_t generation);
        bool InjectNamedInputs(MLNamedInputs namedInputs,
                               uint32_t id,
                               uint32_t generation,
                               uint32_t contextId,
                               uint32_t contextGeneration);
        bool InjectNamedOperands(MLNamedOperands namedOperands, uint32_t id, uint32_t generation);
        bool InjectNamedOutputs(MLNamedOutputs namedOutputs, uint32_t id, uint32_t generation);

      private:
        std::unique_ptr<server::Server> mImpl;
    };

    namespace server {
        class WEBNN_WIRE_EXPORT MemoryTransferService {
          public:
            MemoryTransferService();
            virtual ~MemoryTransferService();

            class ReadHandle;
            class WriteHandle;

            // Deserialize data to create Read/Write handles. These handles are for the client
            // to Read/Write data.
            virtual bool DeserializeReadHandle(const void* deserializePointer,
                                               size_t deserializeSize,
                                               ReadHandle** readHandle) = 0;
            virtual bool DeserializeWriteHandle(const void* deserializePointer,
                                                size_t deserializeSize,
                                                WriteHandle** writeHandle) = 0;

            class WEBNN_WIRE_EXPORT ReadHandle {
              public:
                ReadHandle();
                virtual ~ReadHandle();

                // Return the size of the command serialized if
                // SerializeDataUpdate is called with the same offset/size args
                virtual size_t SizeOfSerializeDataUpdate(size_t offset, size_t size) = 0;

                // Gets called when a MapReadCallback resolves.
                // Serialize the data update for the range (offset, offset + size) into
                // |serializePointer| to the client There could be nothing to be serialized (if
                // using shared memory)
                virtual void SerializeDataUpdate(const void* data,
                                                 size_t offset,
                                                 size_t size,
                                                 void* serializePointer) = 0;

              private:
                ReadHandle(const ReadHandle&) = delete;
                ReadHandle& operator=(const ReadHandle&) = delete;
            };

            class WEBNN_WIRE_EXPORT WriteHandle {
              public:
                WriteHandle();
                virtual ~WriteHandle();

                // Set the target for writes from the client. DeserializeFlush should copy data
                // into the target.
                void SetTarget(void* data);
                // Set Staging data length for OOB check
                void SetDataLength(size_t dataLength);

                // This function takes in the serialized result of
                // client::MemoryTransferService::WriteHandle::SerializeDataUpdate.
                // Needs to check potential offset/size OOB and overflow
                virtual bool DeserializeDataUpdate(const void* deserializePointer,
                                                   size_t deserializeSize,
                                                   size_t offset,
                                                   size_t size) = 0;

              protected:
                void* mTargetData = nullptr;
                size_t mDataLength = 0;

              private:
                WriteHandle(const WriteHandle&) = delete;
                WriteHandle& operator=(const WriteHandle&) = delete;
            };

          private:
            MemoryTransferService(const MemoryTransferService&) = delete;
            MemoryTransferService& operator=(const MemoryTransferService&) = delete;
        };
    }  // namespace server

}  // namespace webnn_wire

#endif  // WEBNN_WIRE_WIRESERVER_H_
