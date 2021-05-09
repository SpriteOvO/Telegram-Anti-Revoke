#include "Logger.h"

#include <Windows.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/details/file_helper.h>

#include "Config.h"
#include "Utils.h"


namespace Logger
{
    [[noreturn]] void DoError(const std::string &Content, bool bReport)
    {
        std::string Message;

        if (bReport) {
            Message =
                "An error has occurred!\n"
                "Please help us fix this problem.\n"
                "--------------------------------------------------\n"
                "\n" +
                Content + "\n"
                "\n"
                "--------------------------------------------------\n"
                "Click \"Abort\" or \"Ignore\" will pop up GitHub issue tracker page.\n"
                "You can submit this information to us there.\n"
                "Thank you very much.";
        }
        else {
            Message = Content;
        }

        int Result;
        do {
            Result = MessageBoxA(NULL, Message.c_str(), "Anti-Revoke Plugin", MB_ABORTRETRYIGNORE | MB_ICONERROR);
            // lol..
        } while (Result == IDRETRY || Result == IDIGNORE);


        if (bReport) {
            // Pop up Github issues tracker

            // Using ShellExecute will cause freezing. I guess it may be caused by different Runtime libraries like cross-module malloc/free.
            // So replace ShellExecute with system, there is not much difference anyway.

            // ShellExecute(NULL, L"open", L"https://github.com/SpriteOvO/Telegram-Anti-Revoke/issues", NULL, NULL, SW_SHOWNORMAL);
            system("start " AR_ISSUES_URL);
        }

        std::exit(0);
    }

    template <class MutexT = std::mutex>
    class CCustomFileSink : public spdlog::sinks::sink, NonCopyable, NonMovable
    {
    public:
        CCustomFileSink(const spdlog::filename_t &Filename) :
            _Formatter{spdlog::details::make_unique<spdlog::pattern_formatter>()}
        {
            _FileHelper.open(Filename, true);
        }

        ~CCustomFileSink() override = default;

        void log(const spdlog::details::log_msg &Message) final
        {
            std::lock_guard<MutexT> Lock{_Mutex};
            SinkIt(Message);
            PostHandler(Message);
        }

        void flush() final
        {
            std::lock_guard<MutexT> Lock{_Mutex};
            Flush();
        }

        void set_pattern(const std::string &Pattern) final
        {
            std::lock_guard<MutexT> Lock{_Mutex};
            SetPattern(Pattern);
        }

        void set_formatter(std::unique_ptr<spdlog::formatter> SinkFormatter) final
        {
            std::lock_guard<MutexT> Lock{_Mutex};
            SetFormatter(std::move(SinkFormatter));
        }

    protected:
        std::unique_ptr<spdlog::formatter> _Formatter;
        MutexT _Mutex;
        spdlog::details::file_helper _FileHelper;

        void SinkIt(const spdlog::details::log_msg &Message)
        {
            spdlog::memory_buf_t Formatted;
            _Formatter->format(Message, Formatted);
            _FileHelper.write(Formatted);
        }

        void Flush()
        {
            _FileHelper.flush();
        }

        void SetPattern(const std::string &Pattern)
        {
            SetFormatter(spdlog::details::make_unique<spdlog::pattern_formatter>(Pattern));
        }

        void SetFormatter(std::unique_ptr<spdlog::formatter> SinkFormatter)
        {
            _Formatter = std::move(SinkFormatter);
        }

        void PostHandler(const spdlog::details::log_msg &Message)
        {
            std::string Payload{Message.payload.begin(), Message.payload.end()};

            switch (Message.level)
            {
#ifdef _DEBUG
            case spdlog::level::warn:
                MessageBoxA(NULL, Payload.c_str(), "Anti-Revoke Plugin", MB_ICONWARNING);
                break;
#endif
            case spdlog::level::err:
                DoError(Payload, false);
                break;

            case spdlog::level::critical:
                DoError(Payload, true);
                break;
            }
        }
    };

    void Initialize()
    {
        auto CustomLogger = std::make_shared<spdlog::logger>(
            "Main",
            std::initializer_list<spdlog::sink_ptr>{
                std::make_shared<CCustomFileSink<>>("ArLog.txt")
            }
        );

        spdlog::register_logger(CustomLogger);
        spdlog::set_default_logger(CustomLogger);

#if defined _DEBUG
        spdlog::set_level(spdlog::level::debug);
#endif
        spdlog::flush_on(spdlog::level::trace);

        spdlog::set_error_handler(
            [](const std::string &Message) {
                DoError("Spdlog error.\n" + Message, true);
            }
        );
    }

} // namespace Logger
