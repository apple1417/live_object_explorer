#ifndef COMPONENTS_DELEGATE_COMPONENT_H
#define COMPONENTS_DELEGATE_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"

namespace live_object_explorer {

class DelegateComponent : public AbstractComponent {
   protected:
    unrealsdk::unreal::FScriptDelegate* addr;
    unrealsdk::unreal::UFunction* signature;

    unrealsdk::unreal::FName last_func_name{0, 0};
    unrealsdk::unreal::UObject* last_obj = nullptr;

    std::string hashless_name;
    std::string cached_func_name;
    std::string cached_obj_name;

   public:
    /**
     * @brief Creates a new component pointing at a delegate property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the property.
     * @param signature The function's signature.
     */
    DelegateComponent(std::string&& name,
                      unrealsdk::unreal::FScriptDelegate* addr,
                      unrealsdk::unreal::UFunction* signature);

    ~DelegateComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_DELEGATE_COMPONENT_H */
