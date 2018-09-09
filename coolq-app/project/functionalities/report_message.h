#pragma once

#include "../processing/message_received.h"

class ReportMessage final : public MessageReceived
{
protected:
    Result process(const cq::Target& current_target, const std::string& message) override;
    Result process_creator(const std::string& message) override;
public:
    ReportMessage() = default;
    ~ReportMessage() override = default;
};
