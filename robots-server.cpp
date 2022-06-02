//
// Created by Piotr Kamiński on 30/05/2022.
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>

class chat_message
{
public:
    enum { header_length = 4 };
    enum { max_body_length = 512 };

    chat_message()
            : body_length_(0)
    {
    }

    const char* data() const
    {
        return data_;
    }

    char* data()
    {
        return data_;
    }

    std::size_t length() const
    {
        return header_length + body_length_;
    }

    const char* body() const
    {
        return data_ + header_length;
    }

    char* body()
    {
        return data_ + header_length;
    }

    std::size_t body_length() const
    {
        return body_length_;
    }

    void body_length(std::size_t new_length)
    {
        body_length_ = new_length;
        if (body_length_ > max_body_length)
            body_length_ = max_body_length;
    }

    bool decode_header()
    {
        char header[header_length + 1] = "";
        std::strncat(header, data_, header_length);
        body_length_ = std::atoi(header);
        if (body_length_ > max_body_length)
        {
            body_length_ = 0;
            return false;
        }
        return true;
    }

    void encode_header()
    {
        char header[header_length + 1] = "";
        std::sprintf(header, "%4d", static_cast<int>(body_length_));
        std::memcpy(data_, header, header_length);
    }

private:
    char data_[header_length + max_body_length];
    std::size_t body_length_;
};

using boost::asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
    virtual ~chat_participant() {}
    virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
    void join(chat_participant_ptr participant)
    {
        participants_.insert(participant);
        for (auto msg: recent_msgs_)
            participant->deliver(msg);
    }

    void leave(chat_participant_ptr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const chat_message& msg)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();

        for (auto participant: participants_)
            participant->deliver(msg);
    }

private:
    std::set<chat_participant_ptr> participants_;
    enum { max_recent_msgs = 100 };
    chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
        : public chat_participant,
          public std::enable_shared_from_this<chat_session>
{
public:
    chat_session(tcp::socket socket, chat_room& room)
            : socket_(std::move(socket)),
              room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const chat_message& msg)
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
        {
            do_write();
        }
    }

private:
    void do_read_type()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), 1),
                                [this, self](boost::system::error_code ec, std::size_t)
                                {
                                    if (!ec && read_msg_.data()[0] >= 0 && read_msg_.data()[0] <= 3)
                                    {
                                        switch (read_msg_.data()[0])
                                        {
                                            case 0:
                                                do_read_join();
                                                break;
                                            case 1:
                                                do_read_place_bomb();
                                                break;
                                            case 2:
                                                do_read_place_block();
                                                break;
                                            case 3:
                                                do_read_move();
                                                break;
                                            default:
                                                room_.leave(shared_from_this());
                                        }
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_join()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), *((uint8_t *)read_msg_.data())),
                                [this, self](boost::system::error_code ec, std::size_t)
                                {
                                    if (!ec)
                                    {
                                         // read_msg_.data() + 1 -> + 1 + *read_msg_.data() -> name
                                         do_read_type();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_place_bomb()
    {
        do_read_type();
    }

    void do_read_place_block()
    {
        do_read_type();
    }

    void do_read_move()
    {
        do_read_type();
    }

    void do_read_header()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), chat_message::header_length),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                {
                                    if (!ec)
                                    {
                                        room_.deliver(read_msg_);
                                        do_read_header();
                                    }
                                    else
                                    {
                                        room_.leave(shared_from_this());
                                    }
                                });
    }

    void do_write()
    {
        auto self(shared_from_this());
        boost::asio::async_write(socket_,
                                 boost::asio::buffer(write_msgs_.front().data(),
                                                     write_msgs_.front().length()),
                                 [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                 {
                                     if (!ec)
                                     {
                                         write_msgs_.pop_front();
                                         if (!write_msgs_.empty())
                                         {
                                             do_write();
                                         }
                                     }
                                     else
                                     {
                                         room_.leave(shared_from_this());
                                     }
                                 });
    }

    tcp::socket socket_;
    chat_room& room_;
    chat_message read_msg_;
    chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server
{
public:
    chat_server(boost::asio::io_context& io_context,
                const tcp::endpoint& endpoint)
            : acceptor_(io_context, endpoint)
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
                        std::make_shared<chat_session>(std::move(socket), room_)->start();
                    }

                    do_accept();
                });
    }

    tcp::acceptor acceptor_;
    chat_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: chat_server <port> [<port> ...]\n";
            return 1;
        }

        boost::asio::io_context io_context;

        std::list<chat_server> servers;
        for (int i = 1; i < argc; ++i)
        {
            tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
            servers.emplace_back(io_context, endpoint);
        }

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}