#pragma once

namespace Components
{
	class Command : public Component
	{
	public:
		static_assert(sizeof(Game::cmd_function_t) == 0x18);

		class Params
		{
		public:
			Params() = default;
			virtual ~Params() = default;

			[[nodiscard]] virtual int size() const = 0;
			[[nodiscard]] virtual const char* get(int index) const = 0;
			[[nodiscard]] virtual std::string join(int index) const;

			virtual const char* operator[](const int index)
			{
				return this->get(index);
			}
		};

		class ClientParams final : public Params
		{
		public:
			ClientParams();

			[[nodiscard]] int size() const override;
			[[nodiscard]] const char* get(int index) const override;

		private:
			int nesting_;
		};

		class ServerParams final : public Params
		{
		public:
			ServerParams();

			[[nodiscard]] int size() const override;
			[[nodiscard]] const char* get(int index) const override;

		private:
			int nesting_;
		};

		Command() = default;

		static Game::cmd_function_t* Allocate();

		static void Add(const char* name, const std::function<void()>& callback);
		static void Add(const char* name, const std::function<void(Command::Params*)>& callback);
		static void AddRaw(const char* name, void(*callback)(), bool key = false);
		static void AddSV(const char* name, const std::function<void(Command::Params*)>& callback);
		static void Execute(std::string command, bool sync = true);

		static Game::cmd_function_t* Find(const std::string& command);

	private:
		static std::unordered_map<std::string, std::function<void(Command::Params*)>> FunctionMap;
		static std::unordered_map<std::string, std::function<void(Command::Params*)>> FunctionMapSV;

		static void AddRawSV(const char* name, void(*callback)());

		static void MainCallback();
		static void MainCallbackSV();
	};
}
