#include <STDInclude.hpp>

namespace Components
{
	std::mutex Logger::MessageMutex;
	std::vector<std::string> Logger::MessageQueue;
	std::vector<Network::Address> Logger::LoggingAddresses[2];
	void(*Logger::PipeCallback)(const std::string&) = nullptr;

	bool Logger::IsConsoleReady()
	{
		return (IsWindow(Console::GetWindow()) != FALSE || (Dedicated::IsEnabled() && !Flags::HasFlag("console")));
	}

	void Logger::Print_Stub(const int channel, const char* message, ...)
	{
		char buf[4096] = {0};

		va_list va;
		va_start(va, message);
		_vsnprintf_s(buf, _TRUNCATE, message, va);
		va_end(va);

		Logger::MessagePrint(channel, {buf});
	}

	void Logger::MessagePrint(const int channel, const std::string& msg)
	{
		std::string out = msg;

		// Filter out coloured strings for stdout
		if (out[0] == '^' && out[1] != '\0')
		{
			out = out.substr(2);
		}

		if (Flags::HasFlag("stdout") || Loader::IsPerformingUnitTests())
		{
			printf("%s", out.data());
			fflush(stdout);
			return;
		}

		if (!Logger::IsConsoleReady())
		{
			OutputDebugStringA(out.data());
		}

		if (!Game::Sys_IsMainThread())
		{
			Logger::EnqueueMessage(msg);
		}
		else
		{
			Game::Com_PrintMessage(channel, msg.data(), 0);
		}
	}

	void Logger::DebugInternal(std::string_view fmt, std::format_args&& args, [[maybe_unused]] const std::source_location& loc)
	{
#ifdef LOGGER_TRACE
		const auto msg = std::vformat(fmt, args);
		const auto out = std::format("Debug:\n    {}\nFile:    {}\nLine:    {}\n", msg, loc.file_name(), loc.line());
#else
		const auto msg = std::vformat(fmt, args);
		const auto out = std::format("^2{}\n", msg);
#endif

		Logger::MessagePrint(Game::CON_CHANNEL_DONT_FILTER, out);
	}

	void Logger::PrintInternal(int channel, std::string_view fmt, std::format_args&& args)
	{
		const auto msg = std::vformat(fmt, args);

		Logger::MessagePrint(channel, msg);
	}

	void Logger::ErrorInternal(const Game::errorParm_t error, const std::string_view fmt, std::format_args&& args)
	{
#ifdef _DEBUG
		if (IsDebuggerPresent()) __debugbreak();
#endif

		const auto msg = std::vformat(fmt, args);
		Game::Com_Error(error, "%s", msg.data());
	}

	void Logger::PrintErrorInternal(int channel, std::string_view fmt, std::format_args&& args)
	{
		const auto msg = "^1Error: " + std::vformat(fmt, args);

		++(*Game::com_errorPrintsCount);
		Logger::MessagePrint(channel, msg);

		if (*Game::cls_uiStarted != 0 && (*Game::com_fixedConsolePosition == 0))
		{
			Game::CL_ConsoleFixPosition();
		}
	}

	void Logger::WarningInternal(int channel, std::string_view fmt, std::format_args&& args)
	{
		const auto msg = "^3" + std::vformat(fmt, args);

		Logger::MessagePrint(channel, msg);
	}

	void Logger::Flush()
	{
// 		if (!Game::Sys_IsMainThread())
// 		{
// 			while (!Logger::MessageQueue.empty())
// 			{
// 				std::this_thread::sleep_for(10ms);
// 			}
// 		}
// 		else
		{
			Logger::Frame();
		}
	}

	void Logger::Frame()
	{
		std::unique_lock _(Logger::MessageMutex);

		for (auto i = Logger::MessageQueue.begin(); i != Logger::MessageQueue.end();)
		{
			Game::Com_PrintMessage(Game::CON_CHANNEL_DONT_FILTER, i->data(), 0);

			if (!Logger::IsConsoleReady())
			{
				OutputDebugStringA(i->data());
			}

			i = Logger::MessageQueue.erase(i);
		}
	}

	void Logger::PipeOutput(void(*callback)(const std::string&))
	{
		Logger::PipeCallback = callback;
	}

	void Logger::PrintMessagePipe(const char* data)
	{
		if (Logger::PipeCallback)
		{
			Logger::PipeCallback(data);
		}
	}

	void Logger::NetworkLog(const char* data, bool gLog)
	{
		if (data == nullptr)
		{
			return;
		}

		for (const auto& addr : Logger::LoggingAddresses[gLog & 1])
		{
			Network::SendCommand(addr, "print", data);
		}
	}

	void Logger::G_LogPrintf_Hk(const char* fmt, ...)
	{
		char string[1024]{};
		char string2[1024]{};

		va_list ap;
		va_start(ap, fmt);
		vsnprintf_s(string2, _TRUNCATE, fmt, ap);
		va_end(ap);

		const auto time = Game::level->time / 1000;
		const auto len = _snprintf_s(string, _TRUNCATE, "%3i:%i%i %s", time / 60, time % 60 / 10, time % 60 % 10, string2);

		if (Game::level->logFile != nullptr)
		{
			Game::FS_Write(string, len, reinterpret_cast<int>(Game::level->logFile));
		}

		// Allow the network log to run even if logFile was not opened
		Logger::NetworkLog(string, true);
	}

	__declspec(naked) void Logger::PrintMessage_Stub()
	{
		__asm
		{
			mov eax, Logger::PipeCallback
			test eax, eax
			jz returnPrint

			pushad
			push [esp + 28h]
			call Logger::PrintMessagePipe
			add esp, 4h
			popad
			retn

		returnPrint:
			pushad
			push 0
			push [esp + 2Ch]
			call Logger::NetworkLog
			add esp, 8h
			popad

			push esi
			mov esi, [esp + 0Ch]

			push 4AA835h
			retn
		}
	}

	void Logger::EnqueueMessage(const std::string& message)
	{
		std::unique_lock _(Logger::MessageMutex);
		Logger::MessageQueue.push_back(message);
	}

	void Logger::RedirectOSPath(const char* file, char* folder)
	{
		if (Dvar::Var("g_log").get<std::string>() == file)
		{
			if (folder != "userraw"s)
			{
				if (Dvar::Var("iw4x_onelog").get<bool>())
				{
					strcpy_s(folder, 256, "userraw");
				}
			}
		}
	}

	__declspec(naked) void Logger::BuildOSPath_Stub()
	{
		__asm
		{
			pushad

			push [esp + 28h]
			push [esp + 30h]

			call Logger::RedirectOSPath

			add esp, 8h

			popad

			mov eax, [esp + 8h]
			push ebp
			push esi
			mov esi, [esp + 0Ch]

			push 64213Fh
			retn
		}
	}

	void Logger::AddServerCommands()
	{
		Command::AddSV("log_add", [](Command::Params* params)
		{
			if (params->size() < 2) return;

			Network::Address addr(params->get(1));

			if (std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr) == Logger::LoggingAddresses[0].end())
			{
				Logger::LoggingAddresses[0].push_back(addr);
			}
		});

		Command::AddSV("log_del", [](Command::Params* params)
		{
			if (params->size() < 2) return;

			const auto num = atoi(params->get(1));
			if (Utils::String::VA("%i", num) == std::string(params->get(1)) && static_cast<unsigned int>(num) < Logger::LoggingAddresses[0].size())
			{
				auto addr = Logger::LoggingAddresses[0].begin() + num;
				Logger::Print("Address {} removed\n", addr->getString());
				Logger::LoggingAddresses[0].erase(addr);
			}
			else
			{
				Network::Address addr(params->get(1));

				const auto i = std::find(Logger::LoggingAddresses[0].begin(), Logger::LoggingAddresses[0].end(), addr);
				if (i != Logger::LoggingAddresses[0].end())
				{
					Logger::LoggingAddresses[0].erase(i);
					Logger::Print("Address {} removed\n", addr.getString());
				}
				else
				{
					Logger::Print("Address {} not found!\n", addr.getString());
				}
			}
		});

		Command::AddSV("log_list", [](Command::Params*)
		{
			Logger::Print("# ID: Address\n");
			Logger::Print("-------------\n");

			for (unsigned int i = 0; i < Logger::LoggingAddresses[0].size(); ++i)
			{
				Logger::Print("#{:03d}: {}\n", i, Logger::LoggingAddresses[0][i].getString());
			}
		});

		Command::AddSV("g_log_add", [](Command::Params* params)
		{
			if (params->size() < 2) return;

			const Network::Address addr(params->get(1));

			if (std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr) == Logger::LoggingAddresses[1].end())
			{
				Logger::LoggingAddresses[1].push_back(addr);
			}
		});

		Command::AddSV("g_log_del", [](Command::Params* params)
		{
			if (params->size() < 2) return;

			const auto num = std::atoi(params->get(1));
			if (Utils::String::VA("%i", num) == std::string(params->get(1)) && static_cast<unsigned int>(num) < Logger::LoggingAddresses[1].size())
			{
				const auto addr = Logger::LoggingAddresses[1].begin() + num;
				Logger::Print("Address {} removed\n", addr->getString());
				Logger::LoggingAddresses[1].erase(addr);
			}
			else
			{
				const Network::Address addr(params->get(1));

				const auto i = std::find(Logger::LoggingAddresses[1].begin(), Logger::LoggingAddresses[1].end(), addr);
				if (i != Logger::LoggingAddresses[1].end())
				{
					Logger::LoggingAddresses[1].erase(i);
					Logger::Print("Address {} removed\n", addr.getString());
				}
				else
				{
					Logger::Print("Address {} not found!\n", addr.getString());
				}
			}
		});

		Command::AddSV("g_log_list", [](Command::Params*)
		{
			Logger::Print("# ID: Address\n");
			Logger::Print("-------------\n");

			for (std::size_t i = 0; i < Logger::LoggingAddresses[1].size(); ++i)
			{
				Logger::Print("#{:03d}: {}\n", i, Logger::LoggingAddresses[1][i].getString());
			}
		});
	}

	Logger::Logger()
	{
		Dvar::Register<bool>("iw4x_onelog", false, Game::DVAR_LATCH | Game::DVAR_ARCHIVE, "Only write the game log to the 'userraw' OS folder");
		Utils::Hook(0x642139, Logger::BuildOSPath_Stub, HOOK_JUMP).install()->quick();

		Logger::PipeOutput(nullptr);

		Scheduler::Loop(Logger::Frame, Scheduler::Pipeline::SERVER);

		Utils::Hook(Game::G_LogPrintf, Logger::G_LogPrintf_Hk, HOOK_JUMP).install()->quick();
		Utils::Hook(Game::Com_PrintMessage, Logger::PrintMessage_Stub, HOOK_JUMP).install()->quick();

		if (Loader::IsPerformingUnitTests())
		{
			Utils::Hook(Game::Com_Printf, Logger::Print_Stub, HOOK_JUMP).install()->quick();
		}

		Events::OnSVInit(Logger::AddServerCommands);
	}

	Logger::~Logger()
	{
		Logger::LoggingAddresses[0].clear();
		Logger::LoggingAddresses[1].clear();

		std::unique_lock lock(Logger::MessageMutex);
		Logger::MessageQueue.clear();
		lock.unlock();

		// Flush the console log
		if (*Game::logfile)
		{
			Game::FS_FCloseFile(*Game::logfile);
		}
	}
}
