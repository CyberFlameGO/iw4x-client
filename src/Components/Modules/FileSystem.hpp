#pragma once

namespace Components
{
	class FileSystem : public Component
	{
	public:
		class AbstractFile
		{
		public:
			virtual ~AbstractFile() = default;

			virtual bool exists() = 0;
			virtual std::string getName() = 0;
			virtual std::string& getBuffer() = 0;
		};

		class File : public AbstractFile
		{
		public:
			File() = default;
			File(std::string file) : filePath{std::move(file)} { this->read(); }
			File(std::string file, Game::FsThread thread) : filePath{std::move(file)} { this->read(thread); }

			bool exists() override { return !this->buffer.empty(); }
			std::string getName() override { return this->filePath; }
			std::string& getBuffer() override { return this->buffer; }

		private:
			std::string filePath;
			std::string buffer;

			void read(Game::FsThread thread = Game::FS_THREAD_MAIN);
		};

		class RawFile : public AbstractFile
		{
		public:
			RawFile() = default;
			RawFile(std::string file) : filePath(std::move(file)) { this->read(); }

			bool exists() override { return !this->buffer.empty(); }
			std::string getName() override { return this->filePath; }
			std::string& getBuffer() override { return this->buffer; }

		private:
			std::string filePath;
			std::string buffer;

			void read();
		};

		class FileReader
		{
		public:
			FileReader() : handle(0), size(-1), name() {}
			FileReader(const std::string& file);
			~FileReader();

			bool exists();
			std::string getName();
			std::string getBuffer();
			int getSize();
			bool read(void* buffer, size_t size);
			void seek(int offset, int origin);

		private:
			int handle;
			int size;
			std::string name;
		};

		class FileWriter
		{
		public:
			FileWriter(std::string file, bool append = false) : handle(0), filePath(std::move(file)) { this->open(append); }
			~FileWriter() { this->close(); }

			void write(const std::string& data);

		private:
			int handle;
			std::string filePath;

			void open(bool append = false);
			void close();
		};

		FileSystem();
		~FileSystem();

		static std::filesystem::path GetAppdataPath();
		static std::vector<std::string> GetFileList(const std::string& path, const std::string& extension);
		static std::vector<std::string> GetSysFileList(const std::string& path, const std::string& extension, bool folders = false);
		static bool _DeleteFile(const std::string& folder, const std::string& file);

	private:
		static std::mutex Mutex;
		static std::recursive_mutex FSMutex;
		static Utils::Memory::Allocator MemAllocator;

		static int ReadFile(const char* path, char** buffer);
		static char* AllocateFile(int size);
		static void FreeFile(void* buffer);

		static void RegisterFolder(const char* folder);

		static void RegisterFolders();
		static void StartupStub();
		static int ExecIsFSStub(const char* execFilename);

		static void FsStartupSync(const char* a1);
		static void FsRestartSync(int a1, int a2);
		static void FsShutdownSync(int a1);
		static void DelayLoadImagesSync();
		static int LoadTextureSync(Game::GfxImageLoadDef **loadDef, Game::GfxImage *image);

		static void IwdFreeStub(Game::iwd_t* iwd);

		static const char* Sys_DefaultInstallPath_Hk();
	};
}
