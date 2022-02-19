#include "main.h"

bool menu_key = false;

auto get_process_wnd(uint32_t pid) -> HWND
{
	std::pair<HWND, uint32_t> params = { 0, pid };

	BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
		auto pParams = (std::pair<HWND, uint32_t>*)(lParam);
		uint32_t processId = 0;

		if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processId)) && processId == pParams->second) {
			SetLastError((uint32_t)-1);
			pParams->first = hwnd;
			return FALSE;
		}

		return TRUE;

		}, (LPARAM)&params);

	if (!bResult && GetLastError() == -1 && params.first)
		return params.first;

	return NULL;
}
auto DirectXInit(HWND hWnd) -> HRESULT
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = modules::Width;
	p_Params.BackBufferHeight = modules::Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig font_config;
	font_config.OversampleH = 1;
	font_config.OversampleV = 1;
	font_config.PixelSnapH = true;

	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};

	modules::espFont = io.Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\Verdana.ttf"), 13.f);
	modules::SkeetFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(icon_compressed_data, icon_compressed_size, 20.f, &font_config, ranges);
	

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	ImGuiStyle& Style = ImGui::GetStyle();
	auto Color = Style.Colors;

	Style.WindowBorderSize = 0;

	Style.ChildRounding = 0;
	Style.FrameRounding = 0;
	Style.ScrollbarRounding = 0;
	Style.GrabRounding = 0;
	Style.PopupRounding = 0;
	Style.WindowRounding = 0;
	Style.WindowTitleAlign.x = 0.50f;
	Style.WindowPadding.x = 20.0f;

	Color[ImGuiCol_WindowBg] = ImColor(18, 18, 18, 255);

	Color[ImGuiCol_FrameBg] = ImColor(31, 31, 31, 255);
	Color[ImGuiCol_FrameBgActive] = ImColor(41, 41, 41, 255);
	Color[ImGuiCol_FrameBgHovered] = ImColor(41, 41, 41, 255);

	Color[ImGuiCol_Button] = ImColor(29, 29, 29, 255);
	Color[ImGuiCol_ButtonActive] = ImColor(32, 32, 32, 255);
	Color[ImGuiCol_ButtonHovered] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
	Color[ImGuiCol_Separator] = ImColor(36, 36, 36, 255);

	Color[ImGuiCol_ResizeGrip] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripActive] = ImColor(30, 30, 30, 255);
	Color[ImGuiCol_ResizeGripHovered] = ImColor(30, 30, 30, 255);

	Color[ImGuiCol_ChildBg] = ImColor(26, 26, 26, 255);

	Color[ImGuiCol_ScrollbarBg] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrab] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);
	Color[ImGuiCol_ScrollbarGrabActive] = ImColor(24, 24, 24, 255);

	Color[ImGuiCol_Header] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderActive] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_HeaderHovered] = ImColor(39, 39, 39, 255);
	Color[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);

	p_Object->Release();
	return S_OK;
}
auto SetupWindow() -> void
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEXA wcex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		("X"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyWnd = CreateWindowExA(NULL, E("X"), E("X"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	//SetWindowDisplayAffinity(MyWnd, 1);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);

	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);
}

float closestDistance = FLT_MAX;
DWORD_PTR closestPawn = NULL;
bool targetlocked = false;
bool isaimbotting;

double GetCrossDistance(double x1, double y1, double z1, double x2, double y2, double z2) {
	return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}
void SetMouseAbsPosition(DWORD x, DWORD y)
{
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;
	input.mi.dx = x;
	input.mi.dy = y;
	SendInput(1, &input, sizeof(input));
}
static auto move_to(float x, float y) -> void {
	float center_x = (ImGui::GetIO().DisplaySize.x / 2);
	float center_y = (ImGui::GetIO().DisplaySize.y / 2);

	float smooth = aimbot::aimspeed;
	float target_x = 0.f;
	float target_y = 0.f;

	if (x != 0.f)
	{
		if (x > center_x)
		{
			target_x = -(center_x - x);
			target_x /= smooth;
			if (target_x + center_x > center_x * 2.f) target_x = 0.f;
		}

		if (x < center_x)
		{
			target_x = x - center_x;
			target_x /= smooth;
			if (target_x + center_x < 0.f) target_x = 0.f;
		}
	}

	if (y != 0.f)
	{
		if (y > center_y)
		{
			target_y = -(center_y - y);
			target_y /= smooth;
			if (target_y + center_y > center_y * 2.f) target_y = 0.f;
		}

		if (y < center_y)
		{
			target_y = y - center_y;
			target_y /= smooth;
			if (target_y + center_y < 0.f) target_y = 0.f;
		}
	}

	SetMouseAbsPosition(static_cast<DWORD>(target_x), static_cast<DWORD>(target_y));
}

auto fn_cache(LPVOID lpParam) -> ULONG WINAPI {

	while (true) {

		std::vector<FNlEntity> tmpList;

		/*if (!offests::offset_uworld)
			offests::offset_uworld = find_signature(E("\x48\x8B\x05\x00\x00\x00\x00\x4D\x8B\xC2"), E("xxx????xxx")) - modules::BaseAddress;

		printf("offset_uworld : 0x%llX\n", offests::offset_uworld);*/

		offests::uworld = read<uint64_t>(modules::BaseAddress + 0x9df4ce0);
		//printf("Uworld : 0x%llX\n", offests::uworld);

		offests::game_instance = read<uint64_t>(offests::uworld + 0x188);
		//printf("GameInstance : 0x%llX\n", offests::game_instance);

		offests::local_players_array = read<uint64_t>(read<uint64_t>(offests::game_instance + 0x38));
		//printf("LocalPlayers : 0x%llX\n", LocalPlayers);

		offests::player_controller = read<uint64_t>(offests::local_players_array + 0x30);
		//printf("PlayerController : 0x%llX\n", PlayerController);

		offests::Pawn = read<uint64_t>(offests::player_controller + 0x2A0);
		if (!offests::Pawn)continue;
		//printf("LocalPawn : 0x%llX\n", LocalPawn);

		offests::rootcomponent = read<uint64_t>(offests::Pawn + 0x130);
		if (!offests::rootcomponent)continue;
		//printf("LocalRootcomp : 0x%llX\n", LocalRootcomp);

		offests::relativelocation = read<Vector3>(offests::rootcomponent + 0x11C);
		if (!IsVec3Valid(offests::relativelocation))continue;

		uintptr_t GameState = read<uintptr_t>(offests::uworld + 0x128);
		if (!GameState)continue;
		//printf("GameState : 0x%llX\n", GameState);

		uintptr_t PlayerArray = read<uintptr_t>(GameState + 0x238);
		if (!PlayerArray)continue;
		//printf("PlayerArray : 0x%llX\n", PlayerArray);

		int Count = read<int>(GameState + (0x238 + sizeof(uint64_t)));

		//printf("%d\n", Count);

		for (int i = 0; i < Count; i++)
		{
			uint64_t CurrentPlayerState = read<uintptr_t>(PlayerArray + (i * sizeof(uint64_t)));
			if (!CurrentPlayerState) continue;
			//printf("CurrentPlayerState : 0x%llX\n", CurrentPlayerState);

			uintptr_t CurrentPawn = read<uintptr_t>(CurrentPlayerState + 0x280);
			if (!CurrentPawn) continue;

			uint64_t mesh = read<uint64_t>(CurrentPawn + 0x280);
			if (!mesh)continue;
			//printf("mesh : 0x%llX\n", mesh);

			uint32_t curactorid = read<uint32_t>(CurrentPawn + 0x18);
			if (!curactorid)continue;
			//printf("curactorid : %d\n", curactorid);

			uint64_t PlayerState = read<uint64_t>(CurrentPawn + 0x240);
			if (!PlayerState)continue;

			uint64_t LocalPlayerState = read<uint64_t>(offests::Pawn + 0x240);
			if (!LocalPlayerState)continue;

			if (mesh != 0x00 && PlayerState != 0x00 && LocalPlayerState != 0x00) {
				FNlEntity fnlEntity{ };
				fnlEntity.Actor = CurrentPawn;
				fnlEntity.mesh = mesh;
				fnlEntity.PlayerState = PlayerState;
				fnlEntity.LocalPlayerState = LocalPlayerState;
				tmpList.push_back(fnlEntity);
			}
		}
		entityList.clear();
		entityList = tmpList;
		//tmpList.clear();
		//Sleep(150);
	}
}
void fn_esp() {

	float distance;

	auto entityListCopy = entityList;

	RGBA skeleton = { colors::skelcol[0] * 255, colors::skelcol[1] * 255, colors::skelcol[2] * 255, 255 };
	RGBA box = { colors::boxcolor[0] * 255, colors::boxcolor[1] * 255, colors::boxcolor[2] * 255, 255 };

	for (unsigned long i = 0; i < entityListCopy.size(); i++)
	{
		auto entity = entityListCopy[i];

		if (offests::Pawn == entity.Actor) continue;

		uint64_t rootcomponent = read<uint64_t>(entity.Actor + 0x130);
		if (!rootcomponent)continue;

		Vector3 relativelocation = read<Vector3>(rootcomponent + 0x11C);
		if (!IsVec3Valid(relativelocation))continue;

		Vector3 Relative_Location = ProjectWorldToScreen(relativelocation);
		if (!IsVec3Valid(Relative_Location))continue;

		uint64_t mesh = entity.mesh;
		if (!mesh)continue;

		Vector3 Foot = GetBoneWithRotation(mesh, 0);
		if (!IsVec3Valid(Foot))continue;

		uint64_t LocalPlayerState = entity.LocalPlayerState;

		uint32_t LocalTeamId = read<uint32_t>(LocalPlayerState + 0xEE0);
		if (!LocalTeamId)continue;

		uint64_t PlayerState = entity.PlayerState;
		if (!PlayerState)continue;

		uint32_t TeamId = read<uint32_t>(PlayerState + 0xEE0);
		if (!TeamId)continue;

		if (mesh != 0x00 && LocalTeamId != TeamId && (int)Foot.x != 0 && (int)Foot.z != 0)
		{
			distance = Calc3D_Dist(offests::relativelocation, relativelocation) / 100.f;

			Vector3 vHeadBone = GetBoneWithRotation(mesh, 68);
			if (!IsVec3Valid(vHeadBone))continue;
			Vector3 vHeadBoneOut = ProjectWorldToScreen(vHeadBone);
			if (!IsVec3Valid(vHeadBoneOut))continue;

			Vector3 vHip = GetBoneWithRotation(mesh, 2);
			if (!IsVec3Valid(vHip))continue;
			Vector3 vHipOut = ProjectWorldToScreen(vHip);
			if (!IsVec3Valid(vHipOut))continue;

			Vector3 vNeck = GetBoneWithRotation(mesh, 67);
			if (!IsVec3Valid(vNeck))continue;
			Vector3 vNeckOut = ProjectWorldToScreen(vNeck);
			if (!IsVec3Valid(vNeckOut))continue;

			Vector3 vUpperArmLeft = GetBoneWithRotation(mesh, 9);
			if (!IsVec3Valid(vUpperArmLeft))continue;
			Vector3 vUpperArmLeftOut = ProjectWorldToScreen(vUpperArmLeft);
			if (!IsVec3Valid(vUpperArmLeftOut))continue;

			Vector3 vUpperArmRight = GetBoneWithRotation(mesh, 38);
			if (!IsVec3Valid(vUpperArmRight))continue;
			Vector3 vUpperArmRightOut = ProjectWorldToScreen(vUpperArmRight);
			if (!IsVec3Valid(vUpperArmRightOut))continue;

			Vector3 vLeftHand = GetBoneWithRotation(mesh, 10);
			if (!IsVec3Valid(vLeftHand))continue;
			Vector3 vLeftHandOut = ProjectWorldToScreen(vLeftHand);
			if (!IsVec3Valid(vLeftHandOut))continue;

			Vector3 vRightHand = GetBoneWithRotation(mesh, 39);
			if (!IsVec3Valid(vRightHand))continue;
			Vector3 vRightHandOut = ProjectWorldToScreen(vRightHand);
			if (!IsVec3Valid(vRightHandOut))continue;

			Vector3 vLeftHand1 = GetBoneWithRotation(mesh, 11);
			if (!IsVec3Valid(vLeftHand1))continue;
			Vector3 vLeftHandOut1 = ProjectWorldToScreen(vLeftHand1);
			if (!IsVec3Valid(vLeftHandOut1))continue;

			Vector3 vRightHand1 = GetBoneWithRotation(mesh, 40);
			if (!IsVec3Valid(vRightHand1))continue;
			Vector3 vRightHandOut1 = ProjectWorldToScreen(vRightHand1);
			if (!IsVec3Valid(vRightHandOut1))continue;

			Vector3 vRightThigh = GetBoneWithRotation(mesh, 76);
			if (!IsVec3Valid(vRightThigh))continue;
			Vector3 vRightThighOut = ProjectWorldToScreen(vRightThigh);
			if (!IsVec3Valid(vRightThighOut))continue;

			Vector3 vLeftThigh = GetBoneWithRotation(mesh, 69);
			if (!IsVec3Valid(vLeftThigh))continue;
			Vector3 vLeftThighOut = ProjectWorldToScreen(vLeftThigh);
			if (!IsVec3Valid(vLeftThighOut))continue;

			Vector3 vRightCalf = GetBoneWithRotation(mesh, 77);
			if (!IsVec3Valid(vRightCalf))continue;
			Vector3 vRightCalfOut = ProjectWorldToScreen(vRightCalf);
			if (!IsVec3Valid(vRightCalfOut))continue;

			Vector3 vLeftCalf = GetBoneWithRotation(mesh, 70);
			if (!IsVec3Valid(vLeftCalf))continue;
			Vector3 vLeftCalfOut = ProjectWorldToScreen(vLeftCalf);
			if (!IsVec3Valid(vLeftCalfOut))continue;

			Vector3 vLeftFoot = GetBoneWithRotation(mesh, 73);
			if (!IsVec3Valid(vLeftFoot))continue;
			Vector3 vLeftFootOut = ProjectWorldToScreen(vLeftFoot);
			if (!IsVec3Valid(vLeftFootOut))continue;

			Vector3 vRightFoot = GetBoneWithRotation(mesh, 80);
			if (!IsVec3Valid(vRightFoot))continue;
			Vector3 vRightFootOut = ProjectWorldToScreen(vRightFoot);
			if (!IsVec3Valid(vRightFootOut))continue;

			Vector3 Pelvis = GetBoneWithRotation(mesh, 2);
			if (!IsVec3Valid(Pelvis))continue;
			Vector3 PelvisOut = ProjectWorldToScreen(Pelvis);
			if (!IsVec3Valid(PelvisOut))continue;

			if (distance <= visuals::MaxSkeletonDrawDistance && IsInScreen(vNeckOut))
			{
				if (visuals::skel) {
					DrawLine(vNeckOut.x, vNeckOut.y, vHeadBoneOut.x, vHeadBoneOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(PelvisOut.x, PelvisOut.y, vNeckOut.x, vNeckOut.y, &skeleton, thick::SkeletonThick);

					DrawLine(vUpperArmLeftOut.x, vUpperArmLeftOut.y, vNeckOut.x, vNeckOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(vUpperArmRightOut.x, vUpperArmRightOut.y, vNeckOut.x, vNeckOut.y, &skeleton, thick::SkeletonThick);

					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vUpperArmLeftOut.x, vUpperArmLeftOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vUpperArmRightOut.x, vUpperArmRightOut.y, &skeleton, thick::SkeletonThick);

					DrawLine(vLeftHandOut.x, vLeftHandOut.y, vLeftHandOut1.x, vLeftHandOut1.y, &skeleton, thick::SkeletonThick);
					DrawLine(vRightHandOut.x, vRightHandOut.y, vRightHandOut1.x, vRightHandOut1.y, &skeleton, thick::SkeletonThick);

					DrawLine(vLeftThighOut.x, vLeftThighOut.y, vHipOut.x, vHipOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(vRightThighOut.x, vRightThighOut.y, vHipOut.x, vHipOut.y, &skeleton, thick::SkeletonThick);

					DrawLine(vLeftCalfOut.x, vLeftCalfOut.y, vLeftThighOut.x, vLeftThighOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(vRightCalfOut.x, vRightCalfOut.y, vRightThighOut.x, vRightThighOut.y, &skeleton, thick::SkeletonThick);

					DrawLine(vLeftFootOut.x, vLeftFootOut.y, vLeftCalfOut.x, vLeftCalfOut.y, &skeleton, thick::SkeletonThick);
					DrawLine(vRightFootOut.x, vRightFootOut.y, vRightCalfOut.x, vRightCalfOut.y, &skeleton, thick::SkeletonThick);

				}
			}

			if (distance <= visuals::MaxDistance && IsInScreen(vNeckOut))
			{
				Vector3 player_position = GetBoneWithRotation(mesh, 0);
				Vector3 player_screen = ProjectWorldToScreen(player_position);

				Vector3 BoxHead = GetBoneWithRotation(mesh, 68);
				Vector3 head_screen = ProjectWorldToScreen(Vector3(BoxHead.x, BoxHead.y + 0.3, BoxHead.z));

				Vector3 normal_head = ProjectWorldToScreen(BoxHead);

				float BoxHeight = player_screen.y - head_screen.y;
				float BoxWidth = BoxHeight / 1.8f;

				if (visuals::box && IsInScreen(vNeckOut))
				{
					if (visuals::boxMode == 0 || visuals::boxMode == 1)
					{
						if (visuals::outline)
						{
							DrawNormalBox(player_screen.x - BoxWidth / 2 + 1, head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawNormalBox(player_screen.x - BoxWidth / 2 - 1, head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawNormalBox(player_screen.x - BoxWidth / 2, head_screen.y + 1, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawNormalBox(player_screen.x - BoxWidth / 2, head_screen.y - 1, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
						}
						DrawNormalBox(player_screen.x - (BoxWidth / 2), head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &box);
					}
					if (visuals::boxMode == 2 || visuals::boxMode == 3)
					{
						if (visuals::outline)
						{
							DrawCornerBox(player_screen.x - BoxWidth / 2 + 1, head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawCornerBox(player_screen.x - BoxWidth / 2 - 1, head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawCornerBox(player_screen.x - BoxWidth / 2, head_screen.y + 1, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
							DrawCornerBox(player_screen.x - BoxWidth / 2, head_screen.y - 1, BoxWidth, BoxHeight, thick::box_thick, &Col.black);
						}
						DrawCornerBox(player_screen.x - (BoxWidth / 2), head_screen.y, BoxWidth, BoxHeight, thick::box_thick, &box);
					}

					if (visuals::boxMode == 1 || visuals::boxMode == 3)  thick::box_thick = 1,
						DrawFilledRect(player_screen.x - (BoxWidth / 2), head_screen.y, BoxWidth, BoxHeight, &Col.filled);

					if (visuals::boxMode == 4)
						Box3D(relativelocation, Vector3(80.f, 80.f, 180.f), &box, thick::SkeletonThick);
				}

				if (visuals::lines && IsInScreen(vNeckOut))
				{
					if (visuals::lineMode == 0)
						DrawLine(modules::Width / 2, modules::Height, player_screen.x, player_screen.y, &box, 1.5f);
					if (visuals::lineMode == 1)
						DrawLine(modules::Width / 2, 0.f, player_screen.x, player_screen.y, &box, 1.5f);
					if (visuals::lineMode == 2)
						DrawLine(modules::Width / 2, modules::Height / 2, player_screen.x, player_screen.y, &box, 1.5f);

				}

				if (visuals::show_head && IsInScreen(vNeckOut))
					DrawCircle(normal_head.x, normal_head.y, BoxHeight / 25, &box, 15);

				if (visuals::name && IsInScreen(vNeckOut))
				{
					char buf[256];
					sprintf(buf, E("[ %dm ]"), (int)distance);
					DrawString(14, vHeadBoneOut.x, vHeadBoneOut.y - 10, &box, true, true, buf);
				}

				if (aimbot::aimbot && IsInScreen(vNeckOut)) {
					auto dx = vHeadBoneOut.x - (modules::Width / 2);
					auto dy = vHeadBoneOut.y - (modules::Height / 2);
					auto dz = vHeadBoneOut.z - (modules::Depth / 2);
					auto dist = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
					if (dist < aimbot::aimfov && dist < closestDistance && targetlocked == false) {
						closestDistance = dist;
						closestPawn = entity.Actor;
					}
				}
			}
		}
	}
	if (closestPawn != 0) {
		if (aimbot::aimbot && closestPawn && GetAsyncKeyState(hotkeys::aimkey))
		{
			targetlocked = true;
			//isRecentlyRendered = true;

			uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x280);
			auto rootHead = GetBoneWithRotation(currentactormesh, select_hitbox());
			Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);

			if (aimbot::prediction) {

				Vector3 vellocity = read<Vector3>(currentactormesh + 0x140);
				float Distance = Calc3D_Dist(offests::relativelocation, rootHeadOut) / 100.f;
				float Time = Distance / (3000 / 100.f);
				rootHeadOut = rootHeadOut + vellocity * Time;
			}

			if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
				if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, modules::Width / 2, modules::Height / 2, modules::Depth / 2) <= aimbot::aimfov)) {
					//DrawLine(modules::Width / 2, modules::Height / 2, rootHeadOut.x, rootHeadOut.y, &Col.white, thick::SkeletonThick);
					move_to(rootHeadOut.x, rootHeadOut.y);
				}
			}
		}
		else
		{
			isaimbotting = false;
			targetlocked = false;
			//isRecentlyRendered = false;

			uint64_t currentactormesh = read<uint64_t>(closestPawn + 0x280);
			auto rootHead = GetBoneWithRotation(currentactormesh, select_hitbox());
			Vector3 rootHeadOut = ProjectWorldToScreen(rootHead);
			if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
				if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, modules::Width / 2, modules::Height / 2, modules::Depth / 2) <= aimbot::aimfov)) {
					DrawLine(modules::Width / 2, modules::Height / 2, rootHeadOut.x, rootHeadOut.y, &Col.white, thick::SkeletonThick);
				}
			}
			closestDistance = FLT_MAX;
			closestPawn = NULL;
		}
	}
}

Vector3 RotatePoint(Vector3 pointToRotate, Vector3 centerPoint, int posX, int posY, int sizeX, int sizeY, float angle, bool* viewCheck, bool angleInRadians = false);
Vector3 RotatePoint(Vector3 EntityPos, Vector3 LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, bool* viewCheck, bool angleInRadians)
{
	float r_1, r_2;
	float x_1, y_1;

	r_1 = -(EntityPos.y - LocalPlayerPos.y);
	r_2 = EntityPos.x - LocalPlayerPos.x;

	float Yaw = angle - 90.0f;

	float yawToRadian = Yaw * (float)(M_PI / 180.0F);
	x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
	y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

	*viewCheck = y_1 < 0;

	int sizX = sizeX / 2;
	int sizY = sizeY / 2;

	x_1 += sizX;
	y_1 += sizY;

	if (x_1 < 5)
		x_1 = 5;

	if (x_1 > sizeX - 5)
		x_1 = sizeX - 5;

	if (y_1 < 5)
		y_1 = 5;

	if (y_1 > sizeY - 5)
		y_1 = sizeY - 5;


	x_1 += posX;
	y_1 += posY;


	return Vector3(x_1, y_1, 0);
}
void DrawRadar()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 oldPadding = style.WindowPadding;
	float oldAlpha = style.Colors[ImGuiCol_WindowBg].w;
	style.WindowPadding = ImVec2(0, 0);
	style.Colors[ImGuiCol_WindowBg].w = (float)255 / 255.0f;
	ImGui::Begin(E(" RADAR "), &modules::radar, ImVec2(200, 200), 0.4F, ImGuiWindowFlags_NoCollapse);
	{
		ImVec2 siz = ImGui::GetWindowSize();
		ImVec2 pos = ImGui::GetWindowPos();


		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
		windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), 0xFF000000, 1.5f);
		windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), 0xFF000000, 1.5f);

		auto entityListCopy = entityList;

		for (auto entity : entityListCopy)
		{
			uint64_t rootcomponent = read<uint64_t>(entity.Actor + 0x130);
			if (!rootcomponent)continue;

			Vector3 Relativelocation = read<Vector3>(rootcomponent + 0x11C);
			if (!IsVec3Valid(Relativelocation))continue;

			bool viewCheck = false;
			Vector3 EntityPos = RotatePoint(Relativelocation, offests::relativelocation, pos.x, pos.y, siz.x, siz.y, 2, &viewCheck);

			int s = 4;
			switch (modules::radarstyle)  
			{
			case 0:
			{
				windowDrawList->AddRect(ImVec2(EntityPos.x - s, EntityPos.y - s),
					ImVec2(EntityPos.x + s, EntityPos.y + s),
					0xFFFFFFFF);
				break;
			}
			case 1:
			{
				windowDrawList->AddRectFilled(ImVec2(EntityPos.x - s, EntityPos.y - s),
					ImVec2(EntityPos.x + s, EntityPos.y + s),
					0xFFFFFFFF);
				break;
			}
			case 2:
			{
				windowDrawList->AddCircle(ImVec2(EntityPos.x, EntityPos.y), s, 0xFFFFFFFF);
				break;
			}
			case 3:
			{
				windowDrawList->AddCircleFilled(ImVec2(EntityPos.x, EntityPos.y), s, 0xFFFFFFFF);
				break;
			}
			default:
				break;
			}
		}
	}
	ImGui::End();
	style.WindowPadding = oldPadding;
	style.Colors[ImGuiCol_WindowBg].w = oldAlpha;
}
void background()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Once);

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.40f / 1.f * 2.f));
	static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove;
	ImGui::Begin(E("##background"), nullptr, flags);
	ImGui::End();
	ImGui::PopStyleColor();
}
void menu() {

	static int menutab = 0;
	static auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;

	ImGuiStyle* style = &ImGui::GetStyle();

	ImGui::Begin((E("LEAK")), NULL, flags);
	{
		static float flRainbow;
		float flSpeed = 0.0003f;
		ImVec2 curPos = ImGui::GetCursorPos();
		ImVec2 curWindowPos = ImGui::GetWindowPos();
		curPos.x += curWindowPos.x;
		curPos.y += curWindowPos.y;
		RGBA FPS = Col.rainbow();
		DrawRectRainbow(curPos.x - 10, curPos.y - 5, ImGui::GetWindowSize().x + modules::Width, curPos.y + -10, flSpeed, &FPS, flRainbow);

		ImGui::SetWindowSize(ImVec2(300, 800), ImGuiCond_Once);
		{
			ImGui::PushItemWidth(150.f);

			ImGui::Checkbox((E("Aimbot")), &aimbot::aimbot);
			ImGui::Checkbox((E("Prediction")), &aimbot::prediction);
			ImGui::Checkbox((E("Smooth")), &aimbot::smooth);
			ImGui::Checkbox((E("Show FOV")), &aimbot::fovcircle);

			ImGui::Text((E("FOV: ")));
			ImGui::SliderFloat(E("##fovkrai"), &aimbot::aimfov, 1.f, 700.f, E("%.2f"));

			ImGui::Text((E("Smooth: ")));
			ImGui::SliderFloat(E("##smoothkrai"), &aimbot::aimspeed, 1.0f, 25.0f, E("%.2f"));

			ImGui::Text((E("Aim Key: ")));
			HotkeyButton(hotkeys::aimkey, ChangeKey, keystatus);

			ImGui::Text((E("Hitbox: ")));
			ImGui::Combo((E("##BONES")), &aimbot::hitbox, Hitbox, IM_ARRAYSIZE(Hitbox));

			ImGui::Checkbox((E("Show Box")), &visuals::box);
			ImGui::SameLine(150.f);
			ImGui::Checkbox((E("Show Skeleton")), &visuals::skel);
			ImGui::Checkbox((E("Distance")), &visuals::name);
			ImGui::SameLine(150.f);
			ImGui::Checkbox((E("Show Head")), &visuals::show_head);
			ImGui::Checkbox((E("Show Lines")), &visuals::lines);
			ImGui::SameLine(150.f);
			ImGui::Checkbox((E("Outline")), &visuals::outline);
			ImGui::Checkbox((E("Radar")), &modules::radar);

			ImGui::Text((E("Box Style"))); ImGui::SameLine(100.f);
			ImGui::Combo((E("##BOXSTYLES")), &visuals::boxMode, boxStyle, IM_ARRAYSIZE(boxStyle));

			ImGui::Text(E("Lines Style")); 	ImGui::SameLine(100.f);
			ImGui::Combo((E("##LINESSTYLE")), &visuals::lineMode, linesMode, IM_ARRAYSIZE(linesMode));

			ImGui::Text((E("Box Thick: ")));
			ImGui::SliderInt((E("##BoxThick")), &thick::box_thick, 1.f, 3.f);

			ImGui::Text((E("Skeleton Thick: ")));
			ImGui::SliderInt((E("##SkeletonThick")), &thick::SkeletonThick, 1.f, 3.f);

			ImGui::Text((E("Radar Style:")));
			ImGui::Combo((E("##radar")), &modules::radarstyle, RadarStyle, IM_ARRAYSIZE(RadarStyle));

			ImGui::Text((E("FOV Color: ")));
			ImGui::SameLine(80.f);
			bool open_popup = ImGui::ColorButton(E("##fovcolor"), *(ImVec4*)&colors::fovcol, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions);
			if (open_popup)
			{
				ImGui::OpenPopup(E("fovcolor"));
			}
			if (ImGui::BeginPopup(E("fovcolor")))
			{
				ImGui::ColorPicker4(E("##fovcolor"), (float*)&colors::fovcol, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
				ImGui::EndPopup();
			}

			ImGui::Text((E("Skeleton Color: ")));
			ImGui::SameLine(110.f);
			bool open_popup4 = ImGui::ColorButton(E("##skeletoncolor"), *(ImVec4*)&colors::skelcol, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions);
			if (open_popup4)
			{
				ImGui::OpenPopup(E("skeletoncolor"));
			}
			if (ImGui::BeginPopup(E("skeletoncolor")))
			{
				ImGui::ColorPicker4(E("##skeletoncolor"), (float*)&colors::skelcol, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
				ImGui::EndPopup();
			}

			ImGui::Text((E("BOX Color: ")));
			ImGui::SameLine(80.f);
			bool open_popup5 = ImGui::ColorButton(E("##boxcolor"), *(ImVec4*)&colors::boxcolor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions);
			if (open_popup5)
			{
				ImGui::OpenPopup(E("boxcolor"));
			}
			if (ImGui::BeginPopup(E("boxcolor")))
			{
				ImGui::ColorPicker4(E("##boxcolor"), (float*)&colors::boxcolor, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
				ImGui::EndPopup();
			}

			ImGui::Text((E("ESP Distance: ")));
			ImGui::SliderFloat(E("##espdistance"), &visuals::MaxDistance, 500.f, 1000.f, E("%.2f"));

			ImGui::Text((E("Skeleton Distance: ")));
			ImGui::SliderFloat(E("##skeletondistance"), &visuals::MaxSkeletonDrawDistance, 50.f, 300.f, E("%.2f"));

			ImGui::Text((E("Config: "))); ImGui::SameLine(80.f); ImGui::Combo((E("##settsas")), &setting::settsMode, settsName, IM_ARRAYSIZE(settsName));


			if (ImGui::Button(((E("Save"))), ImVec2(100, 20)))
			{
				if (setting::settsMode == 0)
					Save_Settings((E("C:\\FN-Default.ini")));
			}

			ImGui::SameLine(150.f);
			if (ImGui::Button(((E("Load"))), ImVec2(100, 20)))
			{
				if (setting::settsMode == 0)
					Load_Settings((E("C:\\FN-Default.ini")));
			}

		}
	}

	ImGui::End();
}
void shortcurts()
{
	if (GetAsyncKeyState(VK_INSERT))
	{
		if (menu_key == false)
		{
			menu_key = true;
		}
		else if (menu_key == true)
		{
			menu_key = false;
		}
		Sleep(200);
	}
}
auto render() -> void
{

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	fn_esp();

	RGBA FOVCIRCLE = { colors::fovcol[0] * 255, colors::fovcol[1] * 255, colors::fovcol[2] * 255, 255 };

	if (aimbot::fovcircle) {
		DrawCircle(modules::Width / 2, modules::Height / 2, float(aimbot::aimfov), &FOVCIRCLE, 100);
	}

	if (modules::radar) {
		DrawRadar();
	}

	shortcurts();
	if (menu_key)
	{
		background();
		menu();
		ImGui::GetIO().MouseDrawCursor = 1;
	}
	else {
		ImGui::GetIO().MouseDrawCursor = 0;
	}

	ImGui::EndFrame();
	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}
	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}
auto MainLoop() -> WPARAM
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();
		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{

			old_rc = rc;

			modules::Width = rc.right;
			modules::Height = rc.bottom;

			p_Params.BackBufferWidth = modules::Width;
			p_Params.BackBufferHeight = modules::Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, modules::Width, modules::Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanuoD3D();
	DestroyWindow(MyWnd);

	return Message.wParam;
}
auto WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)->LRESULT CALLBACK
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		CleanuoD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}
auto CleanuoD3D() -> void
{
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}
	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}
auto isTopwin() -> ULONG
{
	HWND hWnd = GetForegroundWindow();
	if (hWnd == GameWnd)
		return TopWindowGame;
	if (hWnd == MyWnd)
		return TopWindowMvoe;

	return 0;
}
auto SetWindowToTarget() -> void
{
	while (true)
	{
		GameWnd = get_process_wnd(modules::PID);
		if (GameWnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(GameWnd, &GameRect);
			modules::Width = GameRect.right - GameRect.left;
			modules::Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(GameWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				modules::Height -= 39;
			}
			modules::ScreenCenterX = modules::Width / 2;
			modules::ScreenCenterY = modules::Height / 2;
			MoveWindow(MyWnd, GameRect.left, GameRect.top, modules::Width, modules::Height, true);
		}
	}
}

auto main() -> ULONG
{
	unsigned long int Readed_Bytes_Amount;

	pidde = GetProcessId(L"FortniteClient-Win64-Shipping.exe");
	if (!pidde) {
		printf(E("Process Not Found\n"));
		Sleep(-1);
	}

	DrverInit = CreateFileW((L"\\\\.\\TDLD"), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	info_t Input_Output_Data;
	Input_Output_Data.pid = pidde;
	DeviceIoControl(DrverInit, ctl_base, &Input_Output_Data, sizeof Input_Output_Data, &Input_Output_Data, sizeof Input_Output_Data, &Readed_Bytes_Amount, nullptr);
	uint64_t sussyaddy = (unsigned long long int)Input_Output_Data.data;

	while (true)
	{
		SetupWindow();
		DirectXInit(MyWnd);

		modules::PID = pidde;
		modules::BaseAddress = sussyaddy;

		DWORD tid = 0;
		HANDLE tHandle = CreateThread(NULL, 0, fn_cache, 0, 0, &tid);
		CloseHandle(tHandle);

		MainLoop();
	}
}