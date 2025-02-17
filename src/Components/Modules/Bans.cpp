#include <STDInclude.hpp>

namespace Components
{
	// Have only one instance of IW4x read/write the file
	std::unique_lock<Utils::NamedMutex> Bans::Lock()
	{
		static Utils::NamedMutex mutex{"iw4x-ban-list-lock"};
		std::unique_lock lock{mutex};
		return lock;
	}

	bool Bans::IsBanned(const banEntry& entry)
	{
		BanList list;
		LoadBans(&list);

		if (entry.first.bits)
		{
			for (const auto& idEntry : list.idList)
			{
				if (idEntry.bits == entry.first.bits)
				{
					return true;
				}
			}
		}

		if (entry.second.full)
		{
			for (const auto& ipEntry : list.ipList)
			{
				if (ipEntry.full == entry.second.full)
				{
					return true;
				}
			}
		}

		return false;
	}

	void Bans::InsertBan(const banEntry& entry)
	{
		BanList list;
		LoadBans(&list);

		if (entry.first.bits)
		{
			bool found = false;
			for (const auto& idEntry : list.idList)
			{
				if (idEntry.bits == entry.first.bits)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.idList.push_back(entry.first);
			}
		}

		if (entry.second.full)
		{
			bool found = false;
			for (const auto& ipEntry : list.ipList)
			{
				if (ipEntry.full == entry.second.full)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.ipList.push_back(entry.second);
			}
		}

		SaveBans(&list);
	}

	void Bans::SaveBans(const BanList* list)
	{
		assert(list != nullptr);

		const auto _ = Lock();

		std::vector<std::string> idVector;
		std::vector<std::string> ipVector;

		for (const auto& idEntry : list->idList)
		{
			idVector.emplace_back(Utils::String::VA("%llX", idEntry.bits));
		}

		for (const auto& ipEntry : list->ipList)
		{
			ipVector.emplace_back(Utils::String::VA("%u.%u.%u.%u",
				ipEntry.bytes[0] & 0xFF,
				ipEntry.bytes[1] & 0xFF,
				ipEntry.bytes[2] & 0xFF,
				ipEntry.bytes[3] & 0xFF));
		}

		const nlohmann::json bans = nlohmann::json
		{
			{ "ip", ipVector },
			{ "id", idVector },
		};

		FileSystem::FileWriter ("bans.json").write(bans.dump());
	}

	void Bans::LoadBans(BanList* list)
	{
		assert(list != nullptr);

		const auto _ = Lock();

		FileSystem::File bans("bans.json");

		if (!bans.exists())
		{
			Logger::Debug("bans.json does not exist");
			return;
		}

		std::string error;
		const auto banData = nlohmann::json::parse(bans.getBuffer());

		if (!banData.is_object())
		{
			Logger::Debug("bans.json contains invalid data");
			return;
		}

		const auto& idList = banData["id"];
		const auto& ipList = banData["ip"];

		if (idList.is_array())
		{
			nlohmann::json::array_t arr = idList;

			for (auto &idEntry : arr)
			{
				if (idEntry.is_string())
				{
					SteamID id;
					auto guid = idEntry.get<std::string>();
					id.bits = std::strtoull(guid.data(), nullptr, 16);

					list->idList.push_back(id);
				}
			}
		}

		if (ipList.is_array())
		{
			nlohmann::json::array_t arr = ipList;

			for (auto &ipEntry : arr)
			{
				if (ipEntry.is_string())
				{
					Network::Address addr(ipEntry.get<std::string>());

					list->ipList.push_back(addr.getIP());
				}
			}
		}
	}

	void Bans::BanClient(Game::client_t* cl, const std::string& reason)
	{
		SteamID guid;
		guid.bits = cl->steamID;

		InsertBan({guid, cl->header.netchan.remoteAddress.ip});

		Game::SV_DropClient(cl, reason.data(), true);
	}

	void Bans::UnbanClient(SteamID id)
	{
		BanList list;
		LoadBans(&list);

		const auto entry = std::find_if(list.idList.begin(), list.idList.end(), [&id](const SteamID& entry)
		{
			return id.bits == entry.bits;
		});

		if (entry != list.idList.end())
		{
			list.idList.erase(entry);
		}

		SaveBans(&list);
	}

	void Bans::UnbanClient(Game::netIP_t ip)
	{
		BanList list;
		LoadBans(&list);

		const auto entry = std::find_if(list.ipList.begin(), list.ipList.end(), [&ip](const Game::netIP_t& entry)
		{
			return ip.full == entry.full;
		});

		if (entry != list.ipList.end())
		{
			list.ipList.erase(entry);
		}

		SaveBans(&list);
	}

	Bans::Bans()
	{
		Command::Add("banClient", [](Command::Params* params)
		{
			if (!(*Game::com_sv_running)->current.enabled)
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 2)
			{
				Logger::Print("{} <client number> : permanently ban a client\n", params->get(0));
				return;
			}

			const auto* input = params->get(1);

			for (auto i = 0; input[i] != '\0'; ++i)
			{
				if (input[i] < '0' || input[i] > '9')
				{
					Logger::Print("Bad slot number: {}\n", input);
					return;
				}
			}

			const auto num = std::atoi(input);

			if (num < 0 || num >= *Game::svs_clientCount)
			{
				Logger::Print("Bad client slot: {}\n", num);
				return;
			}

			const auto* cl = &Game::svs_clients[num];
			if (cl->header.state == Game::CS_FREE)
			{
				Logger::Print("Client {} is not active\n", num);
				return;
			}

			const std::string reason = params->size() < 3 ? "EXE_ERR_BANNED_PERM" : params->join(2);
			Bans::BanClient(&Game::svs_clients[num], reason);
		});

		Command::Add("unbanClient", [](Command::Params* params)
		{
			if (!(*Game::com_sv_running)->current.enabled)
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 3)
			{
				Logger::Print("{} <type> <ip or guid>\n", params->get(0));
				return;
			}

			const auto* type = params->get(1);

			if (type == "ip"s)
			{
				Network::Address address(params->get(2));
				UnbanClient(address.getIP());

				Logger::Print("Unbanned IP {}\n", params->get(2));

			}
			else if (type == "guid"s)
			{
				SteamID id;
				id.bits = strtoull(params->get(2), nullptr, 16);

				UnbanClient(id);

				Logger::Print("Unbanned GUID {}\n", params->get(2));
			}
		});
	}
}
