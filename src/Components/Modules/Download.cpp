#include <STDInclude.hpp>
#include "GSC/Script.hpp"

namespace Components
{
	mg_mgr Download::Mgr;
	Download::ClientDownload Download::CLDownload;

	std::thread Download::ServerThread;
	bool Download::Terminate;
	bool Download::ServerRunning;

#pragma region Client

	void Download::InitiateMapDownload(const std::string& map, bool needPassword)
	{
		Download::InitiateClientDownload(map, needPassword, true);
	}

	void Download::InitiateClientDownload(const std::string& mod, bool needPassword, bool map)
	{
		if (Download::CLDownload.running) return;

		Scheduler::Once([]
		{
			Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(0));
			Dvar::Var("ui_dl_progress").set("(0/0) %");
			Dvar::Var("ui_dl_transRate").set("0.0 MB/s");
		}, Scheduler::Pipeline::MAIN);

		Command::Execute("openmenu mod_download_popmenu", false);

		if (needPassword)
		{
			std::string pass = Dvar::Var("password").get<std::string>();
			if (pass.empty())
			{
				// shouldn't ever happen but this is safe
				Party::ConnectError("A password is required to connect to this server!");
				return;
			}

			Download::CLDownload.hashedPassword = Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(pass), "");
		}

		Download::CLDownload.running = true;
		Download::CLDownload.isMap = map;
		Download::CLDownload.mod = mod;
		Download::CLDownload.terminateThread = false;
		Download::CLDownload.totalBytes = 0;
		Download::CLDownload.lastTimeStamp = 0;
		Download::CLDownload.downBytes = 0;
		Download::CLDownload.timeStampBytes = 0;
		Download::CLDownload.isPrivate = needPassword;
		Download::CLDownload.target = Party::Target();
		Download::CLDownload.thread = std::thread(Download::ModDownloader, &Download::CLDownload);
	}

	bool Download::ParseModList(ClientDownload* download, const std::string& list)
	{
		if (!download) return false;
		download->files.clear();

		std::string error;
		nlohmann::json listData = nlohmann::json::parse(list);

		if (!error.empty() || !listData.is_array())
		{
			Logger::Print("Error: {}\n", error);
			return false;
		}

		download->totalBytes = 0;
		nlohmann::json::array_t listDataArray = listData;

		for (auto& file : listDataArray)
		{
			if (!file.is_object()) return false;

			auto hash = file["hash"];
			auto name = file["name"];
			auto size = file["size"];

			if (!hash.is_string() || !name.is_string() || !size.is_number()) return false;

			Download::ClientDownload::File fileEntry;
			fileEntry.name = name.get<std::string>();
			fileEntry.hash = hash.get<std::string>();
			fileEntry.size = size.get<size_t>();

			if (!fileEntry.name.empty())
			{
				download->files.push_back(fileEntry);
				download->totalBytes += fileEntry.size;
			}
		}

		return true;
	}

	void Download::DownloadHandler(mg_connection *nc, int ev, void* ev_data)
	{
		http_message* hm = reinterpret_cast<http_message*>(ev_data);
		Download::FileDownload* fDownload = reinterpret_cast<Download::FileDownload*>(nc->mgr->user_data);

		if (ev == MG_EV_CONNECT)
		{
			if (hm->message.p)
			{
				fDownload->downloading = true;
				return;
			}
		}

		if (ev == MG_EV_CLOSE)
		{
			fDownload->downloading = false;
			return;
		}

		if (ev == MG_EV_RECV)
		{
			size_t bytes = static_cast<size_t>(*reinterpret_cast<int*>(ev_data));
			Download::DownloadProgress(fDownload, bytes);
		}

		if (ev == MG_EV_HTTP_REPLY)
		{
			nc->flags |= MG_F_CLOSE_IMMEDIATELY;
			fDownload->buffer = std::string(hm->body.p, hm->body.len);
			fDownload->downloading = false;
			return;
		}
	}

	bool Download::DownloadFile(ClientDownload* download, unsigned int index)
	{
		if (!download || download->files.size() <= index) return false;

		auto file = download->files[index];

		std::string path = download->mod + "/" + file.name;
		if (download->isMap)
		{
			path = "usermaps/" + path;
		}

		if (Utils::IO::FileExists(path))
		{
			std::string data = Utils::IO::ReadFile(path);

			if (data.size() == file.size && Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(data), "") == file.hash)
			{
				download->totalBytes += file.size;
				return true;
			}
		}

		auto host = "http://" + download->target.getString();
		auto fastHost = Dvar::Var("sv_wwwBaseUrl").get<std::string>();
		if (Utils::String::StartsWith(fastHost, "https://"))
		{
			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("HTTPS not supported for downloading!");
			}, Scheduler::Pipeline::CLIENT);

			return false;
		}

		if (!Utils::String::StartsWith(fastHost, "http://"))
		{
			fastHost = "http://" + fastHost;
		}

		std::string url;

		// file directory for fasthost looks like this
		// /-usermaps
		//  /-mp_test
		//    -mp_test.ff
		//    -mp_test.iwd
		//   /-mp_whatever
		//	  /-mp_whatever.ff
		// /-mods
		//  /-mod1
		//	  -mod1.iwd
		//    -mod.ff
		//  /-mod2
		//     ...
		if (Dvar::Var("sv_wwwDownload").get<bool>())
		{
			if (!Utils::String::EndsWith(fastHost, "/")) fastHost.append("/");
			url = fastHost + path;
		}
		else
		{
			url = host + "/file/" + (download->isMap ? "map/" : "") + file.name
				+ (download->isPrivate ? ("?password=" + download->hashedPassword) : "");
		}

		Logger::Print("Downloading from url {}\n", url);

		Download::FileDownload fDownload;
		fDownload.file = file;
		fDownload.index = index;
		fDownload.download = download;
		fDownload.downloading = true;
		fDownload.receivedBytes = 0;

		Utils::String::Replace(url, " ", "%20");

		// Just a speedtest ;)
		//download->totalBytes = 1048576000;
		//url = "http://speed.hetzner.de/1GB.bin";

		download->valid = true;
		/*ZeroMemory(&download->mgr, sizeof download->mgr);
		mg_mgr_init(&download->mgr, &fDownload);
		mg_connect_http(&download->mgr, Download::DownloadHandler, url.data(), nullptr, nullptr);

		while (fDownload.downloading && !fDownload.download->terminateThread)
		{
			mg_mgr_poll(&download->mgr, 100);
		}

		mg_mgr_free(&download->mgr);*/

		fDownload.downloading = true;

		Utils::WebIO webIO;
		webIO.setProgressCallback([&fDownload, &webIO](size_t bytes, size_t)
		{
			if(!fDownload.downloading || fDownload.download->terminateThread)
			{
				webIO.cancelDownload();
				return;
			}

			Download::DownloadProgress(&fDownload, bytes - fDownload.receivedBytes);
		});

		bool result = false;
		fDownload.buffer = webIO.get(url, &result);
		if (!result) fDownload.buffer.clear();

		fDownload.downloading = false;

		download->valid = false;

		if (fDownload.buffer.size() != file.size || Utils::Cryptography::SHA256::Compute(fDownload.buffer, true) != file.hash)
		{
			return false;
		}

		if (download->isMap) Utils::IO::CreateDir("usermaps/" + download->mod);
		Utils::IO::WriteFile(path, fDownload.buffer);

		return true;
	}

	void Download::ModDownloader(ClientDownload* download)
	{
		if (!download) download = &Download::CLDownload;

		std::string host = "http://" + download->target.getString();

		std::string listUrl = host + (download->isMap ? "/map" : "/list") + (download->isPrivate ? ("?password=" + download->hashedPassword) : "");

		std::string list = Utils::WebIO("IW4x", listUrl).setTimeout(5000)->get();
		if (list.empty())
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to download the modlist!");
			}, Scheduler::Pipeline::CLIENT);

			return;
		}

		if (download->terminateThread) return;

		if (!Download::ParseModList(download, list))
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to parse the modlist!");
			}, Scheduler::Pipeline::CLIENT);

			return;
		}

		if (download->terminateThread) return;

		static std::string mod;
		mod = download->mod;

		for (unsigned int i = 0; i < download->files.size(); ++i)
		{
			if (download->terminateThread) return;

			if (!Download::DownloadFile(download, i))
			{
				if (download->terminateThread) return;

				mod = Utils::String::VA("Failed to download file: %s!", download->files[i].name.data());
				download->thread.detach();
				download->clear();

				Scheduler::Once([]
				{
					Dvar::Var("partyend_reason").set(mod);
					mod.clear();

					Command::Execute("closemenu mod_download_popmenu");
					Command::Execute("openmenu menu_xboxlive_partyended");
				}, Scheduler::Pipeline::CLIENT);

				return;
			}
		}

		if (download->terminateThread) return;

		download->thread.detach();
		download->clear();

		if (download->isMap)
		{
			Scheduler::Once([]
			{
				Command::Execute("reconnect", false);
			}, Scheduler::Pipeline::CLIENT);
		}
		else
		{
			// Run this on the main thread
			Scheduler::Once([]
			{
				Game::Dvar_SetString(*Game::fs_gameDirVar, mod.data());
				const_cast<Game::dvar_t*>(*Game::fs_gameDirVar)->modified = true;

				mod.clear();

				Command::Execute("closemenu mod_download_popmenu", false);

				if (Dvar::Var("cl_modVidRestart").get<bool>())
				{
					Command::Execute("vid_restart", false);
				}

				Command::Execute("reconnect", false);
			}, Scheduler::Pipeline::MAIN);
		}
	}

#pragma endregion

#pragma region Server

	bool Download::IsClient(mg_connection *nc)
	{
		return (Download::GetClient(nc) != nullptr);
	}

	Game::client_t* Download::GetClient(mg_connection *nc)
	{
		Network::Address address(nc->sa.sa);

		for (int i = 0; i < *Game::svs_clientCount; ++i)
		{
			Game::client_t* client = &Game::svs_clients[i];

			if (client->header.state >= Game::CS_CONNECTED)
			{
				if (address.getIP().full == Network::Address(client->header.netchan.remoteAddress).getIP().full)
				{
					return client;
				}
			}
		}

		return nullptr;
	}

	void Download::DownloadProgress(FileDownload* fDownload, size_t bytes)
	{
		fDownload->receivedBytes += bytes;
		fDownload->download->downBytes += bytes;
		fDownload->download->timeStampBytes += bytes;

		static volatile bool framePushed = false;

		if (!framePushed)
		{
			double progress = 0;
			if (fDownload->download->totalBytes)
			{
				progress = (100.0 / fDownload->download->totalBytes) * fDownload->download->downBytes;
			}

			static unsigned int dlIndex, dlSize, dlProgress;
			dlIndex = fDownload->index + 1;
			dlSize = fDownload->download->files.size();
			dlProgress = static_cast<unsigned int>(progress);

			framePushed = true;
			Scheduler::Once([]
			{
				framePushed = false;
				Dvar::Var("ui_dl_progress").set(Utils::String::VA("(%d/%d) %d%%", dlIndex, dlSize, dlProgress));
			}, Scheduler::Pipeline::CLIENT);
		}

		int delta = Game::Sys_Milliseconds() - fDownload->download->lastTimeStamp;
		if (delta > 300)
		{
			bool doFormat = fDownload->download->lastTimeStamp != 0;
			fDownload->download->lastTimeStamp = Game::Sys_Milliseconds();

			auto dataLeft = fDownload->download->totalBytes - fDownload->download->downBytes;

			int timeLeft = 0;
			if (fDownload->download->timeStampBytes)
			{
				double timeLeftD = ((1.0 * dataLeft) / fDownload->download->timeStampBytes) * delta;
				timeLeft = static_cast<int>(timeLeftD);
			}

			if (doFormat)
			{
				static size_t dlTsBytes;
				static int dlDelta, dlTimeLeft;
				dlTimeLeft = timeLeft;
				dlDelta = delta;
				dlTsBytes = fDownload->download->timeStampBytes;

				Scheduler::Once([]
				{
					Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(dlTimeLeft));
					Dvar::Var("ui_dl_transRate").set(Utils::String::FormatBandwidth(dlTsBytes, dlDelta));
				}, Scheduler::Pipeline::MAIN);
			}

			fDownload->download->timeStampBytes = 0;
		}
	}

	bool Download::VerifyPassword(mg_connection *nc, http_message* message)
	{
		std::string g_password = Dvar::Var("g_password").get<std::string>();
		if (g_password.empty()) return true;

		// sha256 hashes are 64 chars long but we're gonna be safe here
		char buffer[128] = { 0 };
		int passLen = mg_get_http_var(&message->query_string, "password", buffer, sizeof buffer);

		if (passLen <= 0 || std::string(buffer, passLen) != Utils::Cryptography::SHA256::Compute(g_password, true))
		{
			mg_printf(nc, ("HTTP/1.1 403 Forbidden\r\n"s +
				"Content-Type: text/html\r\n"s +
				"Connection: close\r\n"s +
				"\r\n"s +
				((passLen == 0) ? "Password Required"s : "Invalid Password"s)).c_str());

			nc->flags |= MG_F_SEND_AND_CLOSE;
			return false;
		}

		return true;
	}

	void Download::Forbid(mg_connection *nc)
	{
		mg_printf(nc, "HTTP/1.1 403 Forbidden\r\n"
			"Content-Type: text/html\r\n"
			"Connection: close\r\n"
			"\r\n"
			"403 - Forbidden");

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::ServerlistHandler(mg_connection* nc, int ev, void* /*ev_data*/)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		std::vector<nlohmann::json> servers;

		// Build server list
		for (auto& node : Node::GetNodes())
		{
			if (node.isValid())
			{
				servers.push_back(nlohmann::json{ node.to_json()});
			}
		}

		mg_printf(nc,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Connection: close\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"\r\n"
			"%s", nlohmann::json(servers).dump().data());

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::MapHandler(mg_connection *nc, int ev, void* ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		if (!Download::VerifyPassword(nc, reinterpret_cast<http_message*>(ev_data))) return;

		static std::string mapnamePre;
		static nlohmann::json jsonList;

		std::string mapname = (Party::IsInUserMapLobby() ? Dvar::Var("ui_mapname").get<std::string>() : Maps::GetUserMap()->getName());
		if (!Maps::GetUserMap()->isValid() && !Party::IsInUserMapLobby())
		{
			mapnamePre.clear();
			jsonList = std::vector<nlohmann::json>();
		}
		else if (!mapname.empty() && mapname != mapnamePre)
		{
			std::vector<nlohmann::json> fileList;

			mapnamePre = mapname;

			std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\usermaps\\" + mapname;

			for (int i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
			{
				std::string filename = path + "\\" + mapname + Maps::UserMapFiles[i];
				if (Utils::IO::FileExists(filename))
				{
					std::map<std::string, nlohmann::json> file;
					std::string fileBuffer = Utils::IO::ReadFile(filename);

					file["name"] = mapname + Maps::UserMapFiles[i];
					file["size"] = static_cast<int>(fileBuffer.size());
					file["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

					fileList.push_back(file);
				}
			}

			jsonList = fileList;
		}

		mg_printf(nc,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s", jsonList.dump().data());

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::ListHandler(mg_connection* nc, int ev, void* ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		if (!Download::VerifyPassword(nc, reinterpret_cast<http_message*>(ev_data))) return;

// 		if (!Download::IsClient(nc))
// 		{
// 			Download::Forbid(nc);
// 		}
// 		else
		{
			static std::string fsGamePre;
			static nlohmann::json jsonList;

			const std::string fsGame = (*Game::fs_gameDirVar)->current.string;

			if (!fsGame.empty() && fsGame != fsGamePre)
			{
				std::vector<nlohmann::json> fileList;

				fsGamePre = fsGame;

				std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + fsGame;
				auto list = FileSystem::GetSysFileList(path, "iwd", false);

				list.push_back("mod.ff");

				for (auto i = list.begin(); i != list.end(); ++i)
				{
					std::string filename = path + "\\" + *i;
					if (strstr(i->data(), "_svr_") == nullptr && Utils::IO::FileExists(filename))
					{
						std::map<std::string, nlohmann::json> file;
						std::string fileBuffer = Utils::IO::ReadFile(filename);

						file["name"] = *i;
						file["size"] = static_cast<int>(fileBuffer.size());
						file["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

						fileList.push_back(file);
					}
				}

				jsonList = fileList;
			}

			mg_printf(nc,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"
				"Connection: close\r\n"
				"\r\n"
				"%s", jsonList.dump().data());

			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}

	void Download::FileHandler(mg_connection *nc, int ev, void *ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		http_message* message = reinterpret_cast<http_message*>(ev_data);

		//if (!Download::VerifyPassword(nc, message)) return;

// 		if (!Download::IsClient(nc))
// 		{
// 			Download::Forbid(nc);
// 		}
// 		else
		{
			std::string url(message->uri.p, message->uri.len);
			Utils::String::Replace(url, "\\", "/");

			if (url.size() >= 6)
			{
				url = url.substr(6);
			}

			Utils::String::Replace(url, "%20", " ");

			bool isMap = false;
			if (Utils::String::StartsWith(url, "map/"))
			{
				isMap = true;
				url = url.substr(4);

				std::string mapname = (Party::IsInUserMapLobby() ? Dvar::Var("ui_mapname").get<std::string>() : Maps::GetUserMap()->getName());

				bool isValidFile = false;
				for (int i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
				{
					if (url == (mapname + Maps::UserMapFiles[i]))
					{
						isValidFile = true;
						break;
					}
				}

				if ((!Maps::GetUserMap()->isValid() && !Party::IsInUserMapLobby()) || !isValidFile)
				{
					Download::Forbid(nc);
					return;
				}

				url = Utils::String::VA("usermaps\\%s\\%s", mapname.data(), url.data());
			}
			else
			{
				if (url.find_first_of("/") != std::string::npos || (!Utils::String::EndsWith(url, ".iwd") && url != "mod.ff") || strstr(url.data(), "_svr_") != nullptr)
				{
					Download::Forbid(nc);
					return;
				}
			}

			std::string file;
			const std::string fsGame = (*Game::fs_gameDirVar)->current.string;
			std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + (isMap ? "" : fsGame + "\\") + url;

			if ((!isMap && fsGame.empty()) || !Utils::IO::ReadFile(path, &file))
			{
				mg_printf(nc,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"\r\n"
					"404 - Not Found %s", path.data());
			}
			else
			{
				mg_printf(nc,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n", file.size());

				mg_send(nc, file.data(), static_cast<int>(file.size()));
			}

			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}

	void Download::InfoHandler(mg_connection* nc, int ev, void* /*ev_data*/)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		//if (!Download::VerifyPassword(nc, reinterpret_cast<http_message*>(ev_data))) return;

		Utils::InfoString status = ServerInfo::GetInfo();
		Utils::InfoString host = ServerInfo::GetHostInfo();

		std::map<std::string, nlohmann::json> info;
		info["status"] = status.to_json();
		info["host"] = host.to_json();

		std::vector<nlohmann::json> players;

		// Build player list
		for (int i = 0; i < atoi(status.get("sv_maxclients").data()); ++i) // Maybe choose 18 here?
		{
			std::map<std::string, nlohmann::json> playerInfo;
			playerInfo["score"] = 0;
			playerInfo["ping"] = 0;
			playerInfo["name"] = "";

			if ((*Game::com_sv_running)->current.enabled)
			{
				if (Game::svs_clients[i].header.state < Game::CS_CONNECTED) continue;

				playerInfo["score"] = Game::SV_GameClientNum_Score(i);
				playerInfo["ping"] = Game::svs_clients[i].ping;
				playerInfo["name"] = Game::svs_clients[i].name;
			}
			else
			{
				// Score and ping are irrelevant
				const char* namePtr = Game::PartyHost_GetMemberName(Game::g_lobbyData, i);
				if (!namePtr || !namePtr[0]) continue;

				playerInfo["name"] = namePtr;
			}

			players.emplace_back(playerInfo);
		}

		info["players"] = players;

		mg_printf(nc,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Connection: close\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"\r\n"
			"%s", nlohmann::json(info).dump().data());

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::EventHandler(mg_connection *nc, int ev, void *ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		http_message* message = reinterpret_cast<http_message*>(ev_data);

// 		if (message->uri.p, message->uri.len == "/"s)
// 		{
// 			mg_printf(nc,
// 				"HTTP/1.1 200 OK\r\n"
// 				"Content-Type: text/html\r\n"
// 				"Connection: close\r\n"
// 				"\r\n"
// 				"Hi fella!<br>You are%s connected to this server!", (Download::IsClient(nc) ? " " : " not"));
//
//				Game::client_t* client = Download::GetClient(nc);
//
// 			if (client)
// 			{
// 				mg_printf(nc, "<br>Hello %s!", client->name);
// 			}
// 		}
// 		else
		{
			//std::string path = (Dvar::Var("fs_basepath").get<std::string>() + "\\" BASEGAME "\\html");
			//mg_serve_http_opts opts = { 0 };
			//opts.document_root = path.data();
			//mg_serve_http(nc, message, opts);

			FileSystem::File file;
			std::string url = "html" + std::string(message->uri.p, message->uri.len);

			if (Utils::String::EndsWith(url, "/"))
			{
				url.append("index.html");
				file = FileSystem::File(url);
			}
			else
			{
				file = FileSystem::File(url);

				if (!file.exists())
				{
					url.append("/index.html");
					file = FileSystem::File(url);
				}
			}

			std::string mimeType = Utils::GetMimeType(url);

			if (file.exists())
			{
				std::string buffer = file.getBuffer();

				mg_printf(nc,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: %s\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n", mimeType.data(), buffer.size());

				mg_send(nc, buffer.data(), static_cast<int>(buffer.size()));
			}
			else
			{
				mg_printf(nc,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"\r\n"
					"404 - Not Found");
			}
		}

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

#pragma endregion

	Download::Download()
	{
		if (Dedicated::IsEnabled() /*|| Dvar::Var("mod_force_download_server").get<bool>()*/)
		{
			ZeroMemory(&Download::Mgr, sizeof Download::Mgr);
			mg_mgr_init(&Download::Mgr, nullptr);

			Network::OnStart([]()
			{
				mg_connection* nc = mg_bind(&Download::Mgr, Utils::String::VA("%hu", Network::GetPort()), Download::EventHandler);

				if (nc)
				{
					// Handle special requests
					mg_register_http_endpoint(nc, "/info", Download::InfoHandler);
					mg_register_http_endpoint(nc, "/list", Download::ListHandler);
					mg_register_http_endpoint(nc, "/map", Download::MapHandler);
					mg_register_http_endpoint(nc, "/file/", Download::FileHandler);
					mg_register_http_endpoint(nc, "/serverlist", Download::ServerlistHandler);

					mg_set_protocol_http_websocket(nc);
				}
				else
				{
					Logger::Print("Failed to bind TCP socket, moddownload won't work!\n");
				}
			});

			Download::ServerRunning = true;
			Download::Terminate = false;
			Download::ServerThread = std::thread([]
			{
				while (!Download::Terminate)
				{
					mg_mgr_poll(&Download::Mgr, 100);
				}
			});
		}
		else
		{
			Scheduler::Once([]
			{
				Dvar::Register<const char*>("ui_dl_timeLeft", "", Game::DVAR_NONE, "");
				Dvar::Register<const char*>("ui_dl_progress", "", Game::DVAR_NONE, "");
				Dvar::Register<const char*>("ui_dl_transRate", "", Game::DVAR_NONE, "");
			}, Scheduler::Pipeline::MAIN);

			UIScript::Add("mod_download_cancel", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
			{
				Download::CLDownload.clear();
			});
		}

		Scheduler::Once([]
		{
			Dvar::Register<bool>("sv_wwwDownload", false, Game::DVAR_ARCHIVE, "Set to true to enable downloading maps/mods from an external server.");
			Dvar::Register<const char*>("sv_wwwBaseUrl", "", Game::DVAR_ARCHIVE, "Set to the base url for the external map download.");

			// Force users to enable this because we don't want to accidentally turn everyone's pc into a http server into all their files again
			// not saying we are but ya know... accidents happen
			// by having it saved we force the user to enable it in config_mp because it only checks the dvar on startup to see if we should init download or not
			Dvar::Register<bool>("mod_force_download_server", false, Game::DVAR_ARCHIVE, "Set to true to force the client to run the download server for mods (for mods in private matches).");
		}, Scheduler::Pipeline::MAIN);

		Script::AddFunction("HttpGet", Script::ShowDeprecationWarning);
		Script::AddFunction("HttpCancel", Script::ShowDeprecationWarning);
	}

	Download::~Download()
	{
		if (Download::ServerRunning)
		{
			mg_mgr_free(&Download::Mgr);
		}
	}

	void Download::preDestroy()
	{
		Download::Terminate = true;
		if (Download::ServerThread.joinable())
		{
			Download::ServerThread.join();
		}

		if (!Dedicated::IsEnabled())
		{
			Download::CLDownload.clear();
		}
	}
}
