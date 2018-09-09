// ReSharper disable CppInconsistentNaming
#pragma once

#include <vector>

#include "../../processing/message_received.h"
#include "leader_data.h"

class MeetingAt7th final : public MessageReceived
{
private:
    std::vector<LeaderData> leaders;
    void list_leaders() const;
    void clear_leaders();
    void remove_leader(int64_t user_id);
protected:
    Result process(const cq::Target& current_target, const std::string& message) override;
    Result process_creator(const std::string& message) override;
public:
    MeetingAt7th() = default;
    ~MeetingAt7th() override = default;
};
