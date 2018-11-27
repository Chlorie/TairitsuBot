#pragma once

#include <regex>

#include "../processing/message_received.h"

class SubjectiveRepeat final : public MessageReceived
{
private:
    std::regex me_regex{ u8"我" };
    std::regex you_regex{ u8"你" };
    std::regex polite_you_regex{ u8"您" };
protected:
    Result process(const cq::Target& current_target, const std::string& message) override;
    Result process_creator(const std::string& message) override;
public:
    SubjectiveRepeat() = default;
    ~SubjectiveRepeat() override = default;
};
