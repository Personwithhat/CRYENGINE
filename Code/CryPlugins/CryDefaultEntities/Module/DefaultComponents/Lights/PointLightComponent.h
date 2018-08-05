#pragma once
#include "ILightComponent.h"

class CPlugin_CryDefaultEntities;

namespace Cry
{
	namespace DefaultComponents
	{
		class CPointLightComponent
			: public ILightComponent
#ifndef RELEASE
			, public IEntityComponentPreviewer
#endif
		{
		protected:
			friend CPlugin_CryDefaultEntities;
			static void Register(Schematyc::CEnvRegistrationScope& componentScope);

			// IEntityComponent
			virtual void Initialize() final;

			virtual void   ProcessEvent(const SEntityEvent& event) final;
			virtual uint64 GetEventMask() const final;

#ifndef RELEASE
			virtual IEntityComponentPreviewer* GetPreviewer() final { return this; }
#endif
			// ~IEntityComponent

#ifndef RELEASE
			// IEntityComponentPreviewer
			virtual void SerializeProperties(Serialization::IArchive& archive) final {}

			virtual void Render(const IEntity& entity, const IEntityComponent& component, SEntityPreviewContext &context) const final;
			// ~IEntityComponentPreviewer
#endif

		public:
			CPointLightComponent() {}
			virtual ~CPointLightComponent() {}

			virtual void SetOptics(const char* szOpticsFullName) override
			{
				m_optics.m_lensFlareName = szOpticsFullName;

				Initialize();
			}

			virtual void Enable(bool enable) override;

			static void ReflectType(Schematyc::CTypeDesc<CPointLightComponent>& desc)
			{
				desc.SetGUID("0A86908D-642F-4590-ACEF-484E8E39F31B"_cry_guid);
				desc.SetEditorCategory("Lights");
				desc.SetLabel("Point Light");
				desc.SetDescription("Emits light from its origin into all directions");
				desc.SetIcon("icons:ObjectTypes/light.ico");
				desc.SetComponentFlags({ IEntityComponent::EFlags::NoCreationOffset, IEntityComponent::EFlags::Transform, IEntityComponent::EFlags::Socket, IEntityComponent::EFlags::Attach, IEntityComponent::EFlags::ClientOnly });

				desc.AddMember(&CPointLightComponent::m_bActive, 'actv', "Active", "Active", "Determines whether the light is enabled", true);
				desc.AddMember(&CPointLightComponent::m_radius, 'radi', "Radius", "Radius", "Determines the range of the point light", 10.f);
				desc.AddMember(&CPointLightComponent::m_viewDistance, 'view', "ViewDistance", "View Distance", "Determines the distance in which this light will be active", 50);

				desc.AddMember(&CPointLightComponent::m_optics, 'opti', "Optics", "Optics", "Specific Optic Options", CPointLightComponent::SOptics());
				desc.AddMember(&CPointLightComponent::m_color, 'colo', "Color", "Color", "Color emission information", CPointLightComponent::SColor());
				desc.AddMember(&CPointLightComponent::m_shadows, 'shad', "Shadows", "Shadows", "Shadow casting settings", CPointLightComponent::SShadows());
				desc.AddMember(&CPointLightComponent::m_options, 'opt', "Options", "Options", "Specific Light Options", CPointLightComponent::SOptions());
				desc.AddMember(&CPointLightComponent::m_animations, 'anim', "Animations", "Animations", "Light style / animation properties", CPointLightComponent::SAnimations());
			}

			struct SOptions
			{
				inline bool operator==(const SOptions &rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }

				Schematyc::Range<0, 64000> m_attenuationBulbSize = SRenderLight().m_fAttenuationBulbSize;
				bool m_bIgnoreVisAreas = false;
				bool m_bVolumetricFogOnly = false;
				bool m_bAffectsVolumetricFog = true;
				bool m_bAffectsOnlyThisArea = true;
				bool m_bLinkToSkyColor = false;
				bool m_bAmbient = false;
				Schematyc::Range<0, 10000> m_fogRadialLobe = SRenderLight().m_fFogRadialLobe;

				ELightGIMode m_giMode = ELightGIMode::Disabled;
			};

			struct SColor
			{
				inline bool operator==(const SColor &rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }

				ColorF m_color = ColorF(1.f);
				Schematyc::Range<0, 10000, 0, 100> m_diffuseMultiplier = 1.f;
				Schematyc::Range<0, 10000> m_specularMultiplier = 1.f;
			};

			struct SShadows
			{
				inline bool operator==(const SShadows &rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }

				EMiniumSystemSpec m_castShadowSpec = EMiniumSystemSpec::Disabled;
			};

			struct SAnimations
			{
				inline bool operator==(const SAnimations &rhs) const { return 0 == memcmp(this, &rhs, sizeof(rhs)); }

				uint32 m_style = 0;
				float m_speed = 1.f;
			};

			virtual void Enable(bool bEnable);
			bool IsEnabled() const { return m_bActive; }

			virtual SOptions& GetOptions() { return m_options; }
			const SOptions& GetOptions() const { return m_options; }

			virtual SColor& GetColorParameters() { return m_color; }
			const SColor& GetColorParameters() const { return m_color; }

			virtual SShadows& GetShadowParameters() { return m_shadows; }
			const SShadows& GetShadowParameters() const { return m_shadows; }

			virtual SAnimations& GetAnimationParameters() { return m_animations; }
			const SAnimations& GetAnimationParameters() const { return m_animations; }

		protected:
			Schematyc::Range<0, 32768> m_radius = 10.f;
			Schematyc::Range<0, 100, 0, 100, int> m_viewDistance = 50;
		};
	}
}