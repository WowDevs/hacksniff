// hacksniff loader

#include <string>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <hadesmem/injector.hpp>
#include <hadesmem/process.hpp>

int main(int argc, char *argv[])
{
    std::wstring dll, program, logfile;
    std::string exportFunc;
    int processId;

    try
    {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help,h", "display help message")
            ("logfile,l", boost::program_options::wvalue<std::wstring>(&logfile)->default_value(L"snifflog.txt", "snifflog.txt"), "log file")
            ("dll,d", boost::program_options::wvalue<std::wstring>(&dll)->default_value(L"monitor.dll", "monitor.dll"), "dll to inject into program")
            ("export,e", boost::program_options::value<std::string>(&exportFunc)->default_value("Load"), "export to call once dll is injected")
            ("program,p", boost::program_options::wvalue<std::wstring>(&program), "program name")
            ("pid,i", boost::program_options::value<int>(&processId), "process id");

        boost::program_options::variables_map vm;

        try
        {
            boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
            boost::program_options::notify(vm);

            if (vm.count("help"))
            {
                std::cout << desc << std::endl;
                return EXIT_FAILURE;
            }

            if (!vm.count("program") && !vm.count("pid"))
            {
                std::cerr << "ERROR: Must specify program or process ID" << std::endl << std::endl;
                std::cerr << desc << std::endl;
                return EXIT_FAILURE;
            }
        }
        catch (boost::program_options::error const &e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
            std::cerr << desc << std::endl;
            return EXIT_FAILURE;
        }

        if (vm.count("program"))
        {
            std::vector<std::wstring> createArgs;

            const hadesmem::CreateAndInjectData injectData =
                hadesmem::CreateAndInject(program, L"", std::begin(createArgs), std::end(createArgs), dll, exportFunc, hadesmem::InjectFlags::kPathResolution);

            processId = injectData.GetProcess().GetId();
        }
        else
        {
            try
            {
                const hadesmem::Process process(processId);

                auto dllHandle = hadesmem::InjectDll(process, dll, hadesmem::InjectFlags::kPathResolution);

                hadesmem::CallExport(process, dllHandle, exportFunc);
            }
            catch (boost::program_options::error const &e)
            {
                std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
                return EXIT_FAILURE;
            }
        }

        std::wcout << L"Injected.  Process ID: " << processId << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "ERROR: " << std::endl;
        std::cerr << boost::diagnostic_information(e) << std::endl;
    }

    return EXIT_SUCCESS;
}