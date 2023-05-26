// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/json_value.h"
#include "json_rpc_server/json_rpc_server.h"
#include "serialization/json_input_value_serializer.h"
#include "serialization/json_output_stream_serializer.h"

namespace hugin_api
{
    class HuginApiJsonRpcServer : public cryptonote::JsonRpcServer
    {
    public:
        HuginApiJsonRpcServer(
            syst::Dispatcher &sys,
            syst::Event &stop_event,
            HuginApi &hugin_api,
            std::shared_ptr<logging::ILogger> logger_group,
            payment_service::ConfigurationManager &config);
        HuginApiJsonRpcServer(const HuginApiJsonRpcServer &) = delete;

    protected:
        virtual void processJsonRpcRequest(const common::JsonValue &req, common::JsonValue &resp) override;

    private:
        HuginApi &hugin_api;
        logging::LoggerRef logger;

        typedef std::function<void(const common::JsonValue &json_rpc_params, common::JsonValue &json_response)> HandlerFunction;

        template <typename RequestType, typename ResponseType, typename RequestHandler>

        HandlerFunction jsonHandler(RequestHandler handler)
        {
            return [handler, this](const common::JsonValue &json_rpc_params, common::JsonValue &json_response) mutable
            {
                RequestType request;
                ResponseType response;

                try
                {
                    cryptonote::JsonInputValueSerializer input_serializer(const_cast<common::JsonValue &>(json_rpc_params));
                    SerializeRequest(request, input_serializer);
                }
                catch (std::exception &)
                {
                    makeGenericErrorResponse(json_response, "Invalid Request", -32600);
                    return;
                }

                std::error_code ec = handler(request, response);
                if (ec)
                {
                    makeErrorResponse(ec, json_response);
                    return;
                }

                cryptonote::JsonOutputStreamSerializer output_serializer;
                SerializeResponse(response, output_serializer);
                json_response = output_serializer.getValue();
            };

            template <typename RequestType, typename ResponseType, typename RequestHandler>

            void SerializeRequest(RequestType & request, cryptonote::JsonInputValueSerializer & input_serializer)
            {
                request.Serialize(input_serializer);
            }

            // add more requests here as needed

            std::unordered_map<std::string, HandlerFunction> handlers;
        }
    }
} // namespace hugin_api