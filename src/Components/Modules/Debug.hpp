#pragma once

namespace Components
{
	class Debug : public Component
	{
	public:
		Debug();

	private:
		static const Game::dvar_t* DebugOverlay;
		static const Game::dvar_t* BugName;

		// Game dvars
		static Game::dvar_t** PlayerDebugHealth;

		static const char* PMFlagsValues[];
		static const char* POFlagsValues[];
		static const char* PLFlagsValues[];
		static const char* PEFlagsValues[];

		static const char strButtons[];
		static const char strTemplate[];

		static constexpr auto MY_SCALE2 = 0.5f;
		static constexpr auto MY_SCALE_2 = 0.201f;

		static constexpr auto MY_X = -25.0f;
		static constexpr auto MY_Y = 20.0f;

		static const float colorWhite[];

		static std::string BuildPMFlagsString(const Game::playerState_s* ps);
		static std::string BuildPOFlagsString(const Game::playerState_s* ps);
		static std::string BuildPLFlagsString(const Game::playerState_s* ps);
		static std::string BuildPEFlagsString(const Game::playerState_s* ps);

		static void CG_Debug_DrawPSFlags(int localClientNum);
		static void CG_DrawDebugPlayerHealth(int localClientNum);
		static void CG_Debug_DrawFontTest(int localClientNum);

		static void CG_DrawDebugOverlays_Hk(int localClientNum);

		static void Com_Assert_f();
		static void Com_Bug_f(Command::Params* params);

		static void CL_InitDebugDvars();
	};
}
