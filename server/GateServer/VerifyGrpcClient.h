#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"
#include "ConfigMgr.h"
using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyReq;
using message::GetVarifyRsp;
using message::VarifyService;

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:

	GetVarifyRsp GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyRsp reply;
		GetVarifyReq request;
		request.set_email(email);

		Status status = stub_->GetVarifyCode(&context, request, &reply);

		if (status.ok()) {
			
			return reply;
		}
		else {
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}
	}

private:
	VerifyGrpcClient() {
		ConfigMgr cfg;
		std::string host = cfg["VarifyServer"]["Host"];
		std::string port = cfg["VarifyServer"]["Port"];
		if (host.empty()) {
			host = "127.0.0.1";
		}
		if (port.empty()) {
			port = "50051";
		}
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);
	}

	std::unique_ptr<VarifyService::Stub> stub_;
};

