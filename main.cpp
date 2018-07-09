#include <memory>
#include <string>
#include <tuple>

#include <iomanip> //todo: delete this include
#include <iostream> //todo: delete this include

#include <thread>
#include <STC/RawSocket.hpp>
#include <STC/SafeAPI.hpp>
#include <STC/Packet.hpp>
#include <STC/Sniffer.hpp>
#include <STC/SnifferOptions.hpp>
#include <STC/FilteredIPv4Acceptor.hpp>
#include <STC/IPv4Header.hpp>
#include <STC/TCPAcceptor.hpp>
#include <STC/ControllerAcceptor.hpp>
#include <STC/Controller.hpp>

#include <iphlpapi.h>



int main()
{
    try {
        auto info_ptr = stc::SafeGetAdaptersInfo();

        PIP_ADAPTER_INFO adapter_info = info_ptr.get();

        while (adapter_info) {
            std::cout << "Adapter\n";
            std::cout << "\tName: " << adapter_info->AdapterName << std::endl;
            std::cout << "\tDescription: " << adapter_info->Description << std::endl;

            std::cout << "\tMAC: " << std::hex;
            for (unsigned i = 0; i < adapter_info->AddressLength; ++i) {
                std::cout << (i?".":"") << static_cast<unsigned>(adapter_info->Address[i]);
            }

            std::cout << std::dec << std::endl;

            std::cout << "\tCurrent IP: " << adapter_info->IpAddressList.IpAddress.String << std::endl;


            adapter_info = adapter_info->Next;
        }


    } catch (stc::Exception const & e) {
        std::wcout << e.message() << std::endl;
    } /*catch (...) {
        std::cout << "catch" << std::endl;
    }*/

    std::shared_ptr<stc::Controller> controller =
            std::make_shared<stc::Controller>();

    std::shared_ptr<stc::network::Acceptor> my_acceptor_dryer =
            std::make_shared<stc::ControllerAcceptor>(controller);

    std::shared_ptr<stc::network::Acceptor> tcp_acceptor_dryer =
            std::make_shared<::stc::network::TCPAcceptor>(my_acceptor_dryer);

    std::vector<stc::network::FilterIPv4Options> ip_filter_list{
        stc::network::FilterIPv4Options(
                    tcp_acceptor_dryer,
                    "192.168.0.111",
                    stc::network::FilterIPv4Options::Protocol::TCP
                    ),
    };

    stc::network::SnifferOptions opt;

    opt.setAcceptor(
                std::make_shared<stc::network::FilteredIPv4Acceptor>(
                    std::move(ip_filter_list)
                    )
    );

    opt.setIP(inet_addr("192.168.0.110"));

    stc::network::Sniffer sniffer;

    std::thread th(
     [controller]() {
        stc::Controller::ActualData data;
        while (true) {
            std::this_thread::yield();
            controller->getActualData(data);
            int x = 0;
            for (auto & v: data) {
                std::cout << "Packet " << x++ << " [";
                for (auto & e: v) {
                    if (!e.empty()) {
                        std::cout << e << ' ';
                    }
                }
                std::cout << "]\n\n";
            }
            std::cout << "\n\n";
        }
     }
     );
    th.detach();

    sniffer.start(opt);

    return EXIT_SUCCESS;
}


/*
#include <codecvt>



int
main()
{
    service_logger_init("a/Test.log");
    log4cplus::Logger & test = get_service_logger();
    log4cplus::Logger test2 = get_service_logger();
    for (int i = 0; i < 102400; ++i) {
        LOG4CPLUS_DEBUG(test, LOG4CPLUS_TEXT("this русский текст печатается? is log message with number ") << i);
        LOG4CPLUS_FATAL(test2, LOG4CPLUS_TEXT("rus?"));

    }

    stc::Logger logService(stc::LoggerOptions::make_file_options("file.log", 1024 * 1024, 3));
    stc::Logger logSniffer(stc::LoggerOptions::make_file_options("file.log", 1024 * 1024, 3));
    stc::Logger::logService.warning("Warning");
    return 0;
}
*/
