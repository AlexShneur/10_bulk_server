#include <iostream>
#include "async.h"
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class session
    : public std::enable_shared_from_this<session>
{
public:
    session(tcp::socket socket, size_t _bulk_size)
        : socket_(std::move(socket)), bulk_size(_bulk_size)
    {

    }

    ~session()
    {
        async::disconnect(handle);
    }

    void start()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        socket_.async_read_some(boost::asio::buffer(data_, max_length),
            [this, self](boost::system::error_code ec, std::size_t bytes_received)
        {
            if (!ec)
            {
                auto message = std::string{ data_, bytes_received };
                std::string error = "";
                std::string result_str = "";

                if (handle == nullptr)
                    handle = async::connect(bulk_size);
                async::receive(handle, message.c_str(), message.size());
            }
        });
    }


    void do_write(std::size_t length)
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
                do_read();
            }
        });
    }

    tcp::socket socket_;
    enum { max_length = 131072 };
    char data_[max_length];
    size_t bulk_size;
    async::handle_t handle = nullptr;
};

class server
{
public:
    server(boost::asio::io_context& io_context, short port, size_t _bulk_size)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), bulk_size(_bulk_size)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                std::make_shared<session>(std::move(socket), bulk_size)->start();
            }

            do_accept();
        });
    }

    tcp::acceptor acceptor_;
    size_t bulk_size;
};

int main(int argc, const char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: bulk_server <port> <bulk_size>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        server server(io_context, std::atoi(argv[1]), std::atoi(argv[2]));

        io_context.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << "\n";
    }

    return 0;
}
