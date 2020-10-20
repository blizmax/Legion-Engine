#pragma once
#include <core/core.hpp>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>

#if defined(ARGS_DEBUG) && !defined(SUPPRESS_MONO_DEBUG)
#define ARGS_DEBUG_MONO
#endif




namespace legion::scripting
{

    #ifdef ARGS_DEBUG_MONO
        constexpr bool install_softdebugger = true;
    #else
        constexpr bool install_softdebugger = false;
    #endif


    class MonoVMSystem final : public  System<MonoVMSystem>
    {
    public:
        void setup() override
        {
            m_scheduler->sendCommand(m_scheduler->getChainThreadId("Scripting"), [](void* p) {
                MonoVMSystem* self = reinterpret_cast<MonoVMSystem*>(p);

                mono_set_dirs("..\\..\\vm\\lib", "..\\..\\vm\\etc");


                if constexpr (install_softdebugger)
                {
                    mono_debug_init(MONO_DEBUG_FORMAT_MONO);
                    const char * arg = "--debugger-agent=transport=dt_socket,address=127.0.0.1:12123,server=n";

                    std::printf(arg);
                    mono_jit_parse_options(1, const_cast<char**>(&arg));
                }

                const char * filename = "test.dll";

                self->m_domain = mono_jit_init(__FILE__);

                if constexpr (install_softdebugger)
                {
                    mono_debug_domain_create(self->m_domain);
                }

                MonoAssembly* assembly = mono_domain_assembly_open(self->m_domain, "test.dll");

                mono_jit_exec(self->m_domain, assembly, 1, const_cast<char**>(&filename));

                }, this);
            bindToEvent<events::exit, &MonoVMSystem::onExit>();
        }


        void onExit(events::exit* evt)
        {
             m_scheduler->sendCommand(m_scheduler->getChainThreadId("Scripting"),[](void* p)
             {
                mono_jit_cleanup(static_cast<MonoVMSystem*>(p)->m_domain);
                 
             },this);
        }

    private:
        MonoDomain* m_domain = nullptr;
    };
}
