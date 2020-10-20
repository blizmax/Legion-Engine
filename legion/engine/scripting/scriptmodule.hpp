

#include <core/engine/module.hpp>

#include "systems/mono_vm.hpp"


namespace legion::scripting {

    class ScriptModule : public Module
    {

        void setup() override
        {
            addProcessChain("Scripting");
            reportSystem<MonoVMSystem>();
        }

        priority_type priority() override { return 10; }

    };
}
