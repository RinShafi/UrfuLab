#include "base_interface.h"

namespace monitor
{

	namespace fs = std::filesystem;

	static bool wdt_create_pipe([[maybe_unused]] int fd[2])
	{
		if (pipe(fd) < 0)
		{
			std::cerr << "pipe fail" << std::endl;
			return false;
		}
		return true;
	}

	static pid_t run_program([[maybe_unused]] fs::path path, [[maybe_unused]] const std::vector<std::string>& args)
	{
		std::vector<const char*> argv;
		argv.push_back(path.c_str());
		for (const std::string& arg : args)
		{
			argv.push_back(arg.c_str());
		}
		argv.push_back(nullptr);

		posix_spawn_file_actions_t fileActions;

		int result = posix_spawn_file_actions_init(&fileActions);
		if (result != 0)
		{
			return -1;
		}

		result = posix_spawn_file_actions_addclose(&fileActions, STDOUT_FILENO);
		if (result != 0)
		{
			std::cerr << "posix_spawn_file_actions_addclose failed" << std::endl;
			return -1;
		}

		posix_spawn_file_actions_t* pFileActions = &fileActions;

		posix_spawnattr_t attr;

		result = posix_spawnattr_init(&attr);
		if (result != 0)
		{
			std::cerr << "posix_spawnattr_init failed" << std::endl;
			return -1;
		}

		result = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSIGMASK);
		if (result != 0)
		{
			std::cerr << "posix_spawnattr_setflags failed" << std::endl;
			return -1;
		}

		sigset_t mask;
		sigfillset(&mask);
		result = posix_spawnattr_setsigmask(&attr, &mask);
		if (result != 0)
		{
			std::cerr << "posix_spawnattr_setsigmask failed" << std::endl;
			return -1;
		}
		posix_spawnattr_t* pAttr = &attr;

		pid_t childPid;
		result = posix_spawnp(&childPid, path.c_str(), pFileActions, pAttr, const_cast<char**>(&argv[0]),
			nullptr);
		if (result != 0)
		{
			std::cerr << "posix_spawn failed" << std::endl;
			return -1;
		}

		if (pAttr != nullptr)
		{
			result = posix_spawnattr_destroy(pAttr);
			if (result != 0)
			{
				std::cerr << "posix_spawnattr_destroy failed" << std::endl;
				return -1;
			}
		}

		if (pFileActions != nullptr) {
			result = posix_spawn_file_actions_destroy(pFileActions);
			if (result != 0)
			{
				std::cerr << "posix_spawn_file_actions_destroy failed" << std::endl;
				return -1;
			}
		}

		int status;
		result = waitpid(childPid, &status, WNOHANG);
		if (result == -1)
		{
			std::cerr << "waitpid failed" << std::endl;
			return -1;
		}
		return childPid;
	}

	static pid_t run_program(fs::path path)
	{
		std::vector<std::string> empty;
		return run_program(path, empty);
	}

	//static
	int IBaseInterface::m_wdtPipe[2] = { -1, -1 };

	//static
	void IBaseInterface::send_request(const pid_t pid)
	{
		// запись pid процесса в пайп.
		write(m_wdtPipe[1], &pid, sizeof(pid_t));
	}

	// static
	bool IBaseInterface::m_isTerminate = false;
	/**
	 * Конструктор
	 */
	IBaseInterface::IBaseInterface()
	{
	}

	IBaseInterface::~IBaseInterface()
	{
	}

	pid_t IBaseInterface::RunProgram(const t_path& path) const
	{
		return run_program(path);
	}

	bool IBaseInterface::InitPipe()
	{
		if (!wdt_create_pipe(m_wdtPipe))
		{
			return false;
		}
		OnCreateWdtPipe();
		return true;
	}

	pid_t IBaseInterface::RunProgram(const t_path& path, const t_args& args) const
	{
		return run_program(path, args);
	}

	struct t_predefined_program
	{
		pid_t pid;
		const fs::path path;
		const std::vector<std::string> args;
		bool watched;
	};

	/** Описание предопределенных программ для мониторинга */
	static const t_predefined_program predefined_progs[] =
	{
		{0, "./server", {"9999"}, true},
		{0, "./client", {"9999"}, true}
	};

	bool IBaseInterface::PreparePrograms()
	{
		m_progs.clear();
		for ([[maybe_unused]] auto& it : predefined_progs)
		{
			try
			{
				m_progs.push_back({ it.pid, it.path, it.args, it.watched });
			}
			catch (const char* error_message)
			{
				return false;
			}
		}

		return true;
	}

	bool IBaseInterface::TerminateProgram([[maybe_unused]] const pid_t pid) const
	{
		if (kill(pid, SIGKILL) < 0)
		{
			std::cerr << strerror(errno) << std::endl;
			return false;
		}
		return true;
	}

	pid_t IBaseInterface::FindTerminatedTask() const
	{
		int pidstatus = 0;
		const pid_t pid = waitpid(-1, &pidstatus, WNOHANG);
		if (pid > 0)
		{
			if (0 == WIFSIGNALED(pidstatus))
			{
				pidstatus = WEXITSTATUS(pidstatus);
			}
			else
			{
				pidstatus = WTERMSIG(pidstatus);
			}
		}
		return pid;
	}

	bool IBaseInterface::GetRequestTask([[maybe_unused]] pid_t& pid) const
	{
		if (read(m_wdtPipe[0], &pid, sizeof(pid_t)) > 0)
		{
			return true;
		}
		return false;
	}

	bool IBaseInterface::WaitExitAllPrograms() const
	{
		pid_t pid = waitpid(-1, NULL, WNOHANG);
		while (pid > 0)
		{
			pid = waitpid(-1, NULL, WNOHANG);
		}

		return pid < 0;
	}

	bool IBaseInterface::ToDaemon() const
	{
		pid_t pid;
		pid = fork();
		if (pid < 0)
		{
			exit(EXIT_FAILURE);
		}
		if (pid > 0)
		{
			exit(EXIT_SUCCESS);
		}
		if (setsid() < 0)
		{
			exit(EXIT_FAILURE);
		}
		//	umask(0);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		return true;
	}

	void IBaseInterface::Destroy()
	{
		close(m_wdtPipe[0]);
		close(m_wdtPipe[1]);
		m_init = false;
		m_isTerminate = false;
	}

	IBaseInterface::t_progs& IBaseInterface::Progs()
	{
		return m_progs;
	}

	/**
	 * Получить список наблюдаемых программ
	 * @return список наблюдаемых программ
	 */
	const IBaseInterface::t_progs& IBaseInterface::Progs() const
	{
		return m_progs;
	}

} // namespace monitor

