/*
	Yelo: Open Sauce SDK
		Halo 1 (CE) Edition

	See license\OpenSauce\Halo1_CE for specific license information
*/
#include "Common/Precompile.hpp"
#include "Rasterizer/PostProcessing/Generic/c_effect_instance_generic.hpp"

#if !PLATFORM_IS_DEDI

#include <YeloLib/Halo1/shaders/effect_postprocess_generic_definitions.hpp>
#include "Rasterizer/PostProcessing/c_post_processing_main.hpp"

namespace Yelo
{
    namespace Rasterizer
    {
        namespace PostProcessing
        {
            namespace Generic
            {
                void c_effect_instance_generic::SetEffectInstanceDefinition(TagGroups::s_effect_postprocess_generic_effect_instance* definition)
                {
                    m_members_generic.definition = definition;
                    SetQuadDefinition(&m_members_generic.definition->quad_definition);
                }

                const char* c_effect_instance_generic::GetName()
                {
                    return m_members_generic.definition->name;
                }

                Enums::postprocess_render_stage c_effect_instance_generic::GetRenderStage()
                {
                    if (m_members_generic.definition)
                    {
                        return static_cast<Enums::postprocess_render_stage>(m_members_generic.definition->render_stage);
                    }
                    return Enums::_postprocess_render_stage_pre_blur;
                }

                void c_effect_instance_generic::Ctor()
                {
                    c_effect_instance::Ctor();

                    m_members_generic.definition = nullptr;
                }

                void c_effect_instance_generic::Dtor()
                {
                    c_effect_instance::Dtor();

                    m_members_generic.definition = nullptr;
                }

                void c_effect_instance_generic::SetupEffectInstance()
                {
                    c_effect_instance::SetupEffectInstance();

                    if (m_members_generic.definition)
                    {
                        m_members.m_flags.is_active = GetInitiallyActive();
                    }
                    else
                    {
                        m_members.m_flags.is_active = false;
                    }
                }

                bool c_effect_instance_generic::GetInitiallyActive()
                {
                    auto control = FindActivationControl(Enums::_effect_activation_state_initially_active);
                    if (control)
                    {
                        return GetActivationValue(*control);
                    }
                    return true;
                }

                bool c_effect_instance_generic::IsActive()
                {
                    if (!c_effect_instance::IsActive())
                    {
                        return false;
                    }
                    return EvaluateActivationControls();
                }

                TagGroups::s_effect_postprocess_effect_activation_control* c_effect_instance_generic::FindActivationControl(Enums::effect_activation_state state)
                {
                    // return a pointer to the requested activation control
                    for (auto i = 0; i < m_members_generic.definition->activation_controls.Count; i++)
                    {
                        auto& control = m_members_generic.definition->activation_controls[i];

                        if (control.state == state)
                        {
                            return &m_members_generic.definition->activation_controls[i];
                        }
                    }
                    return nullptr;
                }

                bool c_effect_instance_generic::EvaluateActivationControls()
                {
                    // if there are no activation controls, default to active
                    if (m_members_generic.definition->activation_controls.Count == 0)
                    {
                        return true;
                    }

                    // evaluate each control block
                    auto active = m_members.m_flags.is_active;
                    for (auto i = 0; i < m_members_generic.definition->activation_controls.Count; i++)
                    {
                        auto& control = m_members_generic.definition->activation_controls[i];

                        // ignore initially active blocks, they are only used when the instance is initially set up
                        if (control.state == Enums::_effect_activation_state_initially_active)
                        {
                            continue;
                        }

                        // all activation rules must be true before the effect is activated
                        if (m_members_generic.definition->activation_operation)
                        {
                            active |= GetActivationValue(control);
                        }
                        else
                        {
                            if (!GetActivationValue(control))
                            {
                                return false;
                            }
                            active = true;
                        }
                    }
                    return active;
                }

                bool c_effect_instance_generic::GetActivationValue(TagGroups::s_effect_postprocess_effect_activation_control& control)
                {
                    auto& globals = c_post_processing_main::Instance().Globals();

                    // evaluate an activation control
                    auto value = true;
                    switch (control.state)
                    {
                        case Enums::_effect_activation_state_is_in_cutscene:
                            value = globals.m_activation_variables.in_cinematic;
                            break;
                        case Enums::_effect_activation_state_player_is_zoomed:
                            value = globals.m_activation_variables.local_player_zoomed;
                            break;
                        case Enums::_effect_activation_state_player_using_a_vehicle:
                            value = globals.m_activation_variables.local_player_in_vehicle;
                            break;
                    }
                    // invert the result if necessary
                    return control.flags.invert_bit ? !value : value;
                }
            }
        }
    }
}
#endif
