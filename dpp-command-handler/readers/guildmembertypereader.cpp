#include "guildmembertypereader.h"
#include "dpp-command-handler/extensions/cache.h"
#include "dpp-command-handler/utils/lexical_cast.h"
#include "dpp-command-handler/utils/mentions.h"
#include "dpp-command-handler/utils/strings.h"
#include <dpp/cache.h>
#include <dpp/dispatcher.h>

namespace dpp
{
    type_reader_result guild_member_in::read(cluster* cluster, const message_create_t* context, std::string_view input)
    {
        add_results_by_mention(context->msg.guild_id, input); // weight: 1.0
        add_results_by_id(context->msg.guild_id, input); // weight: 0.9
        add_results_by_global_name(context->msg.guild_id, input); // weight: 0.7-0.8
        add_results_by_username(context->msg.guild_id, input); // weight: 0.7-0.8
        add_results_by_nickname(context->msg.guild_id, input); // weight: 0.7-0.8

        if (has_result())
            return type_reader_result::from_success();
        return type_reader_result::from_error(command_error::object_not_found, "User not found.");
    }

    void guild_member_in::add_results_by_global_name(const snowflake guild_id, std::string_view input)
    {
        if (const guild* guild = find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (const user* user = member.get_user(); utility::iequals(user->global_name, input))
                    add_result(member, user->global_name == input ? 0.8f : 0.7f);
    }

    void guild_member_in::add_results_by_id(const snowflake guild_id, std::string_view input)
    {
        if (uint64_t id = utility::lexical_cast<uint64_t>(input, false))
            if (auto gm = find_guild_member_opt(guild_id, id))
                add_result(gm.value());
    }

    void guild_member_in::add_results_by_mention(const snowflake guild_id, std::string_view input)
    {
        if (uint64_t id = utility::parse_user_mention(input))
            if (auto gm = find_guild_member_opt(guild_id, id))
                add_result(gm.value());
    }

    void guild_member_in::add_results_by_nickname(const snowflake guild_id, std::string_view input)
    {
        if (const guild* guild = find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (std::string nickname = member.get_nickname(); utility::iequals(nickname, input))
                    add_result(member, nickname == input ? 0.8f : 0.7f);
    }

    void guild_member_in::add_results_by_username(const snowflake guild_id, std::string_view input)
    {
        if (const guild* guild = find_guild(guild_id))
            for (const auto& [_, member] : guild->members)
                if (const user* user = member.get_user(); utility::iequals(user->username, input))
                    add_result(member, user->username == input ? 0.8f : 0.7f);
    }
}
