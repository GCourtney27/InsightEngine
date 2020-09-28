#include <Engine_pch.h>

#include "Scene_Component.h"

#include "Retina/Input/Input.h"
#include "Retina/Runtime/ACamera.h"

#include "imgui.h"
#include "ImGuizmo.h"

namespace Retina {

	namespace Runtime {

		
		SceneComponent::SceneComponent(AActor* pOwner)
			: ActorComponent("SceneComponent", pOwner)
		{
		}

		SceneComponent::~SceneComponent()
		{
		}

		bool SceneComponent::LoadFromJson(const rapidjson::Value& JsonComponent)
		{
			return false;
		}

		bool SceneComponent::WriteToJson(rapidjson::PrettyWriter<rapidjson::StringBuffer>& Writer)
		{
			return false;
		}

		void SceneComponent::SetEventCallback(const EventCallbackFn& callback)
		{
			m_TranslationData.EventCallback = callback;
		}

		void SceneComponent::OnEvent(Event& e)
		{
		}

		void SceneComponent::OnInit()
		{
		}

		void SceneComponent::OnPostInit()
		{
			TranslationEvent e;
			e.TranslationInfo.WorldMat = m_Transform.GetLocalMatrix();
			m_TranslationData.EventCallback(e);
		}

		void SceneComponent::OnDestroy()
		{
		}

		void SceneComponent::CalculateParent(const DirectX::XMMATRIX& Matrix)
		{
		}

		void SceneComponent::OnRender()
		{
		}

		void SceneComponent::OnUpdate(const float& DeltaTime)
		{
		}

		void SceneComponent::OnImGuiRender()
		{
			RenderSelectionGizmo();
			
			if (ImGui::CollapsingHeader(m_ComponentName, ImGuiTreeNodeFlags_DefaultOpen)) {

				// Show the actor's transform values
				ImGui::Text("Transform##SceneNode");
				ImGui::DragFloat3("Position##SceneNode", &m_Transform.GetPositionRef().x, 0.05f, -1000.0f, 1000.0f);
				ImGui::DragFloat3("Rotation##SceneNode", &m_Transform.GetRotationRef().x, 0.05f, -1000.0f, 1000.0f);
				ImGui::DragFloat3("Scale##SceneNode", &m_Transform.GetScaleRef().x, 0.05f, -1000.0f, 1000.0f);

				ImGui::Checkbox("IsStatic##SceneNode", &IsStatic);

				if (m_pParent)
				{
					m_Transform.SetWorldMatrix(XMMatrixMultiply(m_Transform.GetLocalMatrix(), m_pParent->GetTransformRef().GetWorldMatrix()));
				}
				else
				{
					m_Transform.SetWorldMatrix(m_Transform.GetLocalMatrix());
				}

				TranslationEvent e;
				e.TranslationInfo.WorldMat = m_Transform.GetWorldMatrix();
				m_TranslationData.EventCallback(e);
			}
		}

		void SceneComponent::RenderSceneHeirarchy()
		{
		}

		void SceneComponent::BeginPlay()
		{
			m_EditorTransform = m_Transform;
		}

		void SceneComponent::Tick(const float DeltaMs)
		{
		}

		void SceneComponent::OnAttach()
		{
		}

		void SceneComponent::OnDetach()
		{
		}

		static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
		void SceneComponent::RenderSelectionGizmo()
		{
			static ACamera& s_WorldCameraRef = ACamera::Get();

			XMFLOAT4X4 objectMat;
			XMFLOAT4X4 deltaMat;
			XMFLOAT4X4 viewMat;
			XMFLOAT4X4 projMat;
			XMStoreFloat4x4(&objectMat, m_Transform.GetLocalMatrix());
			XMStoreFloat4x4(&viewMat, s_WorldCameraRef.GetViewMatrix());
			XMStoreFloat4x4(&projMat, s_WorldCameraRef.GetProjectionMatrix());

			if (Input::IsKeyPressed('W')) {
				mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			}
			else if (Input::IsKeyPressed('E')) {
				mCurrentGizmoOperation = ImGuizmo::ROTATE;
			}
			else if (Input::IsKeyPressed('R')) {
				mCurrentGizmoOperation = ImGuizmo::SCALE;
			}

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			//TODO if(Raycast::LastRayCast::Succeeded) than run this line if false than skip it (disbles the guizmo)
			ImGuizmo::Manipulate(*viewMat.m, *projMat.m, mCurrentGizmoOperation, mCurrentGizmoMode, *objectMat.m, *deltaMat.m, NULL, NULL, NULL);

			if (ImGuizmo::IsOver())
			{
				float pos[3] = { 0.0f, 0.0f, 0.0f };
				float sca[3] = { 0.0f, 0.0f, 0.0f };
				float rot[3] = { 0.0f, 0.0f, 0.0f };

				switch (mCurrentGizmoOperation) {
				case ImGuizmo::TRANSLATE:
				{
					ImGuizmo::DecomposeMatrixToComponents(*deltaMat.m, pos, rot, sca);
					m_Transform.Translate(pos[0], pos[1], pos[2]);
					break;
				}
				case ImGuizmo::SCALE:
				{
					ImGuizmo::DecomposeMatrixToComponents(*objectMat.m, pos, rot, sca);
					m_Transform.SetScale(sca[0], sca[1], sca[2]);
					break;
				}
				case ImGuizmo::ROTATE:
				{
					ImGuizmo::DecomposeMatrixToComponents(*deltaMat.m, pos, rot, sca);
					m_Transform.Rotate(rot[0], rot[1], rot[2]);
					break;
				}
				default: { break; }
				}
			}
		}

	} // end namspace Runtime
}// end namaspace Insight
