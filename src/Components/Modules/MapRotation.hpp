#pragma once

namespace Components
{
	class MapRotation : public Component
	{
	public:
		MapRotation();

		static bool Contains(const std::string& key, const std::string& value);

		bool unitTest() override;

	private:
		struct ParseRotationError : public std::exception
		{
			const char* what() const noexcept override { return "Parse Rotation Error"; }
		};

		class RotationData
		{
		public:
			using rotationEntry = std::pair<std::string, std::string>;

			RotationData();

			void randomize();

			// In case a new way to enrich the map rotation is added (other than sv_mapRotation)
			// this method should be called to add a new entry (gamemode/map & value)
			void addEntry(const std::string& key, const std::string& value);

			[[nodiscard]] std::size_t getEntriesSize() const;
			rotationEntry& getNextEntry();

			void parse(const std::string& data);

			[[nodiscard]] bool contains(const std::string& key, const std::string& value) const;

			[[nodiscard]] nlohmann::json to_json() const;

		private:
			std::vector<rotationEntry> rotationEntries_;

			std::size_t index_;
		};

		// Rotation Dvars
		static Dvar::Var SVRandomMapRotation;
		static Dvar::Var SVDontRotate;

		// Holds the parsed data from sv_mapRotation
		static RotationData DedicatedRotation;

		static void LoadRotation(const std::string& data);
		static void LoadMapRotation();

		// Use these commands before SV_MapRotate_f is called
		static void AddMapRotationCommands();

		static bool ShouldRotate();
		static void ApplyMap(const std::string& map);
		static void ApplyGametype(const std::string& gametype);
		static void RestartCurrentMap();
		static void ApplyRotation(RotationData& rotation);
		static void ApplyMapRotationCurrent(const std::string& data);
		static void RandomizeMapRotation();

		static void SV_MapRotate_f();
	};
}
