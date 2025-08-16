#ifndef COMPONENTS_MULTICAST_DELEGATE_COMPONENT_H
#define COMPONENTS_MULTICAST_DELEGATE_COMPONENT_H

#include "pch.h"
#include "components/abstract.h"
#include "components/delegate_component.h"

namespace live_object_explorer {

class MulticastDelegateComponent : public AbstractComponent {
   protected:
    std::string header;
    unrealsdk::unreal::TArray<unrealsdk::unreal::FScriptDelegate>* addr;
    void* last_data;

    unrealsdk::unreal::UFunction* signature;

    std::vector<DelegateComponent> components;
    bool was_force_closed;

   public:
    /**
     * @brief Creates a new component pointing at a multicast delegate property.
     *
     * @param name The component's name. May include hashes.
     * @param addr Pointer to the array.
     * @param signature The function's signature.
     */
    MulticastDelegateComponent(std::string&& name,
                               unrealsdk::unreal::TArray<unrealsdk::unreal::FScriptDelegate>* addr,
                               unrealsdk::unreal::UFunction* signature);

    ~MulticastDelegateComponent() override = default;
    void draw(const ObjectWindowSettings& settings,
              ForceExpandTree expand_children,
              bool show_all_children) override;
    [[nodiscard]] bool passes_filter(const ImGuiTextFilter& filter) override;
};

}  // namespace live_object_explorer

#endif /* COMPONENTS_MULTICAST_DELEGATE_COMPONENT_H */
