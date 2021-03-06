#include "run_brainfvck.h"
#include "brainfvck_interpreter.h"

namespace
{
    constexpr size_t max_instruction_count = 100000;

    std::string unescape_string(std::string msg)
    {
        size_t index = msg.find("&#91");
        while (index != std::string::npos)
        {
            msg.replace(index, 4, "[");
            index = msg.find("&#91");
        }
        index = msg.find("&#93");
        while (index != std::string::npos)
        {
            msg.replace(index, 4, "]");
            index = msg.find("&#93");
        }
        return msg;
    }
}

bool RunBrainfvck::on_group_msg(const int64_t group, const int64_t user, const std::string& msg)
{
    static const boost::regex regex{ fmt::format(R"({}runbf([\s\S]*))", utils::at_self_regex) };
    boost::smatch match;
    if (!regex_match(msg, match, regex)) return false;
    try
    {
        const std::string program = unescape_string(match[1]);
        const std::string result = brainfvck::interpret(program, max_instruction_count);
        utils::reply_group_member(group, user, u8"执行结果：" + result);
    }
    catch (const brainfvck::InterpretError& e)
    {
        utils::reply_group_member(group, user, fmt::format(u8"执行中发生错误：{}", e.what()));
    }
    catch (const cq::exception::ApiError& e)
    {
        if (e.code == -3)
            utils::reply_group_member(group, user, u8"执行中发生错误：运行结果过长或为非法UTF-8字符串");
        else
            throw;
    }
    return true;
}

bool RunBrainfvck::on_private_msg(const int64_t user, const std::string& msg)
{
    static const boost::regex regex{ R"(runbf([\s\S]*))" };
    boost::smatch match;
    if (!regex_match(msg, match, regex)) return false;
    try
    {
        const std::string program = unescape_string(match[1]);
        const std::string result = brainfvck::interpret(program, max_instruction_count);
        cqc::api::send_private_msg(user, u8"执行结果：" + result);
    }
    catch (const brainfvck::InterpretError& e)
    {
        cqc::api::send_private_msg(user, fmt::format(u8"执行中发生错误：{}", e.what()));
    }
    catch (const cq::exception::ApiError& e)
    {
        if (e.code == -3)
            cqc::api::send_private_msg(user, u8"执行中发生错误：运行结果过长或为非法UTF-8字符串");
        else
            throw;
    }
    return true;
}
