#pragma once
#include "dpp-command-handler/results/commandresult.h"
#include "dpp-command-handler/utils/strings.h"
#include "modulebase.h"

namespace dpp
{
    template<typename Derived>
    class module : public module_base
    {
    public:
        explicit module(std::string_view name, std::string_view summary = "") : module_base(name, summary) {}

        TASK(command_result) exec(command_function* function, dpp::cluster* cluster,
            const message_create_t* context, const module_service* service, std::vector<std::string>&& args,
            const std::any& extra_data = {}) override
        {
            try
            {
                auto d = std::make_unique<Derived>();
                d->cluster = cluster;
                d->context = context;
                d->m_extra_data = extra_data;
                d->service = service;

                if (function->is_coroutine())
                    RETURN(AWAIT(function->invoke<TASK(command_result)>(d.get(), std::move(args), cluster, context)));
                else
                    RETURN(function->invoke<command_result>(d.get(), std::move(args), cluster, context));
            }
            catch (const bad_command_argument& e)
            {
                RETURN(command_result::from_error(e.error(), e.what()));
            }
            catch (const std::exception& e)
            {
                RETURN(command_result::from_error(e));
            }
        }
    protected:
        template<typename T>
        T extra_data() const
        {
            if (!m_extra_data.has_value())
            {
                throw std::logic_error("Attempted to get extra data for module " +
                                       utility::demangle(typeid(Derived).name()) +
                                       " when no such data exists");
            }

            return std::any_cast<T>(m_extra_data);
        }
    private:
        std::any m_extra_data;
    };
}
