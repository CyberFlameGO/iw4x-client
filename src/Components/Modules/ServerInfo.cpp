#include <STDInclude.hpp>

namespace Components
{
	ServerInfo::Container ServerInfo::PlayerContainer;

	Game::dvar_t** ServerInfo::CGScoreboardHeight;
	Game::dvar_t** ServerInfo::CGScoreboardWidth;

	unsigned int ServerInfo::GetPlayerCount()
	{
		return ServerInfo::PlayerContainer.playerList.size();
	}

	const char* ServerInfo::GetPlayerText(unsigned int index, int column)
	{
		if (index < ServerInfo::PlayerContainer.playerList.size())
		{
			switch (column)
			{
			case 0:
				return Utils::String::VA("%d", index);
			case 1:
				return ServerInfo::PlayerContainer.playerList[index].name.data();
			case 2:
				return Utils::String::VA("%d", ServerInfo::PlayerContainer.playerList[index].score);
			case 3:
				return Utils::String::VA("%d", ServerInfo::PlayerContainer.playerList[index].ping);
			default:
				break;
			}
		}

		return "";
	}

	void ServerInfo::SelectPlayer(unsigned int index)
	{
		ServerInfo::PlayerContainer.currentPlayer = index;
	}

	void ServerInfo::ServerStatus([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
	{
		ServerInfo::PlayerContainer.currentPlayer = 0;
		ServerInfo::PlayerContainer.playerList.clear();

		auto* serverInfo = ServerList::GetCurrentServer();

		if (info)
		{
			Dvar::Var("uiSi_ServerName").set(serverInfo->hostname);
			Dvar::Var("uiSi_MaxClients").set(serverInfo->clients);
			Dvar::Var("uiSi_Version").set(serverInfo->shortversion);
			Dvar::Var("uiSi_SecurityLevel").set(serverInfo->securityLevel);
			Dvar::Var("uiSi_isPrivate").set(serverInfo->password ? "@MENU_YES" : "@MENU_NO");
			Dvar::Var("uiSi_Hardcore").set(serverInfo->hardcore ? "@MENU_ENABLED" : "@MENU_DISABLED");
			Dvar::Var("uiSi_KillCam").set("@MENU_NO");
			Dvar::Var("uiSi_ffType").set("@MENU_DISABLED");
			Dvar::Var("uiSi_MapName").set(serverInfo->mapname);
			Dvar::Var("uiSi_MapNameLoc").set(Game::UI_LocalizeMapName(serverInfo->mapname.data()));
			Dvar::Var("uiSi_GameType").set(Game::UI_LocalizeGameType(serverInfo->gametype.data()));
			Dvar::Var("uiSi_ModName").set("");
			Dvar::Var("uiSi_aimAssist").set(serverInfo->aimassist ? "@MENU_YES" : "@MENU_NO");
			Dvar::Var("uiSi_voiceChat").set(serverInfo->voice ? "@MENU_YES" : "@MENU_NO");

			if (serverInfo->mod.size() > 5)
			{
				Dvar::Var("uiSi_ModName").set(serverInfo->mod.data() + 5);
			}

			ServerInfo::PlayerContainer.target = serverInfo->addr;
			Network::SendCommand(ServerInfo::PlayerContainer.target, "getstatus");
		}
	}

	void ServerInfo::DrawScoreboardInfo(int localClientNum)
	{
		Game::Font_s* font = Game::R_RegisterFont("fonts/bigfont", 0);
		const auto* cxt = Game::ScrPlace_GetActivePlacement(localClientNum);

		auto addressText = Network::Address(*Game::connectedHost).getString();

		if (addressText == "0.0.0.0:0" || addressText == "loopback")
			addressText = "Listen Server";

		// Get x positions
		auto y = (480.0f - (*ServerInfo::CGScoreboardHeight)->current.value) * 0.5f;
		y += (*ServerInfo::CGScoreboardHeight)->current.value + 6.0f;

		const auto x = 320.0f - (*ServerInfo::CGScoreboardWidth)->current.value * 0.5f;
		const auto x2 = 320.0f + (*ServerInfo::CGScoreboardWidth)->current.value * 0.5f;

		// Draw only when stream friendly ui is not enabled
		if (!Friends::UIStreamFriendly.get<bool>())
		{
			constexpr auto fontSize = 0.35f;
			Game::UI_DrawText(cxt, reinterpret_cast<const char*>(0x7ED3F8), 0x7FFFFFFF, font, x, y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
			Game::UI_DrawText(cxt, addressText.data(), 0x7FFFFFFF, font, x2 - Game::UI_TextWidth(addressText.data(), 0, font, fontSize), y, 0, 0, fontSize, reinterpret_cast<float*>(0x747F34), 3);
		}
	}

	__declspec(naked) void ServerInfo::DrawScoreboardStub()
	{
		__asm
		{
			pushad
			push eax
			call ServerInfo::DrawScoreboardInfo
			pop eax
			popad

			push 591B70h
			retn
		}
	}

	Utils::InfoString ServerInfo::GetHostInfo()
	{
		Utils::InfoString info;

		// TODO: Possibly add all Dvar starting with _
		info.set("admin", Dvar::Var("_Admin").get<const char*>());
		info.set("website", Dvar::Var("_Website").get<const char*>());
		info.set("email", Dvar::Var("_Email").get<const char*>());
		info.set("location", Dvar::Var("_Location").get<const char*>());

		return info;
	}

	Utils::InfoString ServerInfo::GetInfo()
	{
		auto maxClientCount = *Game::svs_clientCount;

		if (!maxClientCount)
		{
			maxClientCount = Dvar::Var("party_maxplayers").get<int>();
		}

		Utils::InfoString info(Game::Dvar_InfoString_Big(Game::DVAR_SERVERINFO));
		info.set("gamename", "IW4");
		info.set("sv_maxclients", Utils::String::VA("%i", maxClientCount));
		info.set("protocol", Utils::String::VA("%i", PROTOCOL));
		info.set("shortversion", SHORTVERSION);
		info.set("mapname", (*Game::sv_mapname)->current.string);
		info.set("isPrivate", (Dvar::Var("g_password").get<std::string>().empty() ? "0" : "1"));
		info.set("checksum", Utils::String::VA("%X", Utils::Cryptography::JenkinsOneAtATime::Compute(Utils::String::VA("%u", Game::Sys_Milliseconds()))));
		info.set("aimAssist", (Gamepad::sv_allowAimAssist.get<bool>() ? "1" : "0"));
		info.set("voiceChat", (Voice::SV_VoiceEnabled() ? "1" : "0"));

		// Ensure mapname is set
		if (info.get("mapname").empty())
		{
			info.set("mapname", Dvar::Var("ui_mapname").get<const char*>());
		}

		// Set matchtype
		// 0 - No match, connecting not possible
		// 1 - Party, use Steam_JoinLobby to connect
		// 2 - Match, use CL_ConnectFromParty to connect

		if (Dvar::Var("party_enable").get<bool>() && Dvar::Var("party_host").get<bool>()) // Party hosting
		{
			info.set("matchtype", "1");
		}
		else if ((*Game::com_sv_running)->current.enabled) // Match hosting
		{
			info.set("matchtype", "2");
		}
		else
		{
			info.set("matchtype", "0");
		}

		return info;
	}

	ServerInfo::ServerInfo()
	{
		ServerInfo::PlayerContainer.currentPlayer = 0;

		ServerInfo::CGScoreboardHeight = reinterpret_cast<Game::dvar_t**>(0x9FD070);
		ServerInfo::CGScoreboardWidth = reinterpret_cast<Game::dvar_t**>(0x9FD0AC);

		// Draw IP and hostname on the scoreboard
		Utils::Hook(0x4FC6EA, ServerInfo::DrawScoreboardStub, HOOK_CALL).install()->quick();

		// Ignore native getStatus implementation
		Utils::Hook::Nop(0x62654E, 6);

		// Add uiscript
		UIScript::Add("ServerStatus", ServerInfo::ServerStatus);

		// Add uifeeder
		UIFeeder::Add(13.0f, ServerInfo::GetPlayerCount, ServerInfo::GetPlayerText, ServerInfo::SelectPlayer);

		Network::OnClientPacket("getStatus", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			std::string playerList;

			Utils::InfoString info = ServerInfo::GetInfo();
			info.set("challenge", Utils::ParseChallenge(data));

			for (auto i = 0; i < atoi(info.get("sv_maxclients").data()); ++i) // Maybe choose 18 here?
			{
				auto score = 0;
				auto ping = 0;
				std::string name;

				if ((*Game::com_sv_running)->current.enabled)
				{
					if (Game::svs_clients[i].header.state < Game::CS_CONNECTED) continue;

					score = Game::SV_GameClientNum_Score(i);
					ping = Game::svs_clients[i].ping;
					name = Game::svs_clients[i].name;
				}
				else
				{
					// Score and ping are irrelevant
					const auto* namePtr = Game::PartyHost_GetMemberName(reinterpret_cast<Game::PartyData*>(0x1081C00), i);
					if (!namePtr || !namePtr[0]) continue;

					name = namePtr;
				}

				playerList.append(Utils::String::VA("%i %i \"%s\"\n", score, ping, name.data()));
			}

			Network::SendCommand(address, "statusResponse", "\\" + info.build() + "\n" + playerList + "\n");
		});

		Network::OnClientPacket("statusResponse", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			if (ServerInfo::PlayerContainer.target != address)
			{
				return;
			}

			const Utils::InfoString info(data.substr(0, data.find_first_of("\n")));

			Dvar::Var("uiSi_ServerName").set(info.get("sv_hostname"));
			Dvar::Var("uiSi_MaxClients").set(info.get("sv_maxclients"));
			Dvar::Var("uiSi_Version").set(info.get("shortversion"));
			Dvar::Var("uiSi_SecurityLevel").set(info.get("sv_securityLevel"));
			Dvar::Var("uiSi_isPrivate").set(info.get("isPrivate") == "0" ? "@MENU_NO" : "@MENU_YES");
			Dvar::Var("uiSi_Hardcore").set(info.get("g_hardcore") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");
			Dvar::Var("uiSi_KillCam").set(info.get("scr_game_allowkillcam") == "0" ? "@MENU_NO" : "@MENU_YES");
			Dvar::Var("uiSi_MapName").set(info.get("mapname"));
			Dvar::Var("uiSi_MapNameLoc").set(Game::UI_LocalizeMapName(info.get("mapname").data()));
			Dvar::Var("uiSi_GameType").set(Game::UI_LocalizeGameType(info.get("g_gametype").data()));
			Dvar::Var("uiSi_ModName").set("");
			Dvar::Var("uiSi_aimAssist").set(info.get("aimAssist") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");
			Dvar::Var("uiSi_voiceChat").set(info.get("voiceChat") == "0" ? "@MENU_DISABLED" : "@MENU_ENABLED");

			switch (atoi(info.get("scr_team_fftype").data()))
			{
			default:
				Dvar::Var("uiSi_ffType").set("@MENU_DISABLED");
				break;
			case 1:
				Dvar::Var("uiSi_ffType").set("@MENU_ENABLED");
				break;
			case 2:
				Dvar::Var("uiSi_ffType").set("@MPUI_RULES_REFLECT");
				break;
			case 3:
				Dvar::Var("uiSi_ffType").set("@MPUI_RULES_SHARED");
				break;
			}

			if (info.get("fs_game").size() > 5)
			{
				Dvar::Var("uiSi_ModName").set(info.get("fs_game").data() + 5);
			}

			auto lines = Utils::String::Split(data, '\n');

			if (lines.size() <= 1) return;

			for (std::size_t i = 1; i < lines.size(); ++i)
			{
				ServerInfo::Container::Player player;

				std::string currentData = lines[i];

				if (currentData.size() < 3) continue;

				// Insert score
				player.score = atoi(currentData.substr(0, currentData.find_first_of(" ")).data());

				// Remove score
				currentData = currentData.substr(currentData.find_first_of(" ") + 1);

				// Insert ping
				player.ping = atoi(currentData.substr(0, currentData.find_first_of(" ")).data());

				// Remove ping
				currentData = currentData.substr(currentData.find_first_of(" ") + 1);

				if (currentData[0] == '\"')
				{
					currentData = currentData.substr(1);
				}

				if (currentData.back() == '\"')
				{
					currentData.pop_back();
				}

				player.name = currentData;

				ServerInfo::PlayerContainer.playerList.push_back(player);
			}
		});
	}

	ServerInfo::~ServerInfo()
	{
		ServerInfo::PlayerContainer.playerList.clear();
	}
}
