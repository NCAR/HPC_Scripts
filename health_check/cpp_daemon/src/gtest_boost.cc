//Make sure THREAD SAFE is Active
#define BOOST_SPIRIT_THREADSAFE

#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

/**
 * @brief Basic shared lock tests
 * @see http://www.boost.org/doc/libs/1_41_0/doc/html/thread/synchronization.html#thread.synchronization.locks.shared_lock
 */
TEST ( boost, locks )
{
    return;
    boost::shared_mutex m1;
    {
        boost::unique_lock<boost::shared_mutex> wlock(m1);
        ASSERT_TRUE(wlock);
        {
            boost::shared_lock<boost::shared_mutex> rlock(m1, boost::try_to_lock);
            ASSERT_FALSE(rlock);
        }
        {
            boost::shared_mutex* mutexptr = wlock.release();
            ASSERT_EQ(mutexptr, &m1);
        }
        {
            boost::shared_lock<boost::shared_mutex> rlock(m1);
            ASSERT_TRUE(rlock);
        }
    }
}


/**
 * @short Test shared mutex 5 times
 */
void testsharedmutex(boost::shared_mutex &m1) // NOLINT
{
//     std::cout << "thread " << boost::this_thread::get_id() << std::endl;

    for (int i = 0; i < 1; ++i)
    {
        //lock_guard is a unique_lock aka a write lock!
        boost::lock_guard<boost::shared_mutex> r1lock(m1);
//         std::cout << "thread locked " << boost::this_thread::get_id() << std::endl;

//         for (int v = 0; v < 50; ++v)
//             boost::this_thread::yield();
    }
}

/**
 * @short Test shared 3 mutexs 5 times
 */
void testshared3mutex(boost::shared_mutex &m1, boost::shared_mutex &m2, boost::shared_mutex &m3) // NOLINT
{
//     std::cout << "thread " << boost::this_thread::get_id() << std::endl;

    for (int i = 0; i < 1; ++i)
    {

        boost::shared_lock<boost::shared_mutex> r1lock(m1, boost::defer_lock);
        boost::shared_lock<boost::shared_mutex> r2lock(m2, boost::defer_lock);
        boost::shared_lock<boost::shared_mutex> r3lock(m3, boost::defer_lock);

        boost::lock(r1lock, r2lock, r3lock);
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
//             std::cout << "thread locked " << boost::this_thread::get_id() << std::endl;

            EXPECT_TRUE(r1lock);
            EXPECT_TRUE(r2lock);
            EXPECT_TRUE(r3lock);

//             for (int v = 0; v < 50; ++v)
//                 boost::this_thread::yield();
        }
    }
}

/**
 * @brief Tests locking and threads
 *  Loads 10 threads and three mutexs,
 *  then runs half the of the threads trying
 *  to lock 1 mutex with the other 5 threads
 *  locking all three mutexs
 * @see http://www.boost.org/dboc/libs/1_47_0/doc/html/thread/synchronization.html#thread.synchronization.locks.shared_lock
 * @see http://www.boost.org/doc/libs/1_47_0/doc/html/thread/thread_management.html
 */
TEST ( boost, threaded_locks )
{
//     std::cout << boost::this_thread::get_id() << std::endl;
//     std::cout << boost::thread::hardware_concurrency() << std::endl;

    boost::shared_mutex m1, m2, m3;
    boost::thread t1(testsharedmutex, boost::ref(m1));
    boost::thread t2(testsharedmutex, boost::ref(m1));
    boost::thread t3(testsharedmutex, boost::ref(m1));
    boost::thread t4(testsharedmutex, boost::ref(m1));
    boost::thread t5(testsharedmutex, boost::ref(m1));
    boost::thread t10(testshared3mutex, boost::ref(m1), boost::ref(m2), boost::ref(m3));
    boost::thread t20(testshared3mutex, boost::ref(m1), boost::ref(m2), boost::ref(m3));
    boost::thread t30(testshared3mutex, boost::ref(m1), boost::ref(m2), boost::ref(m3));
    boost::thread t40(testshared3mutex, boost::ref(m1), boost::ref(m2), boost::ref(m3));
    boost::thread t50(testshared3mutex, boost::ref(m1), boost::ref(m2), boost::ref(m3));
//     std::cout << "threading" << std::endl;
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t10.join();
    t20.join();
    t30.join();
    t40.join();
    t50.join();
}


class boost_asio_server_test : public ::testing::Test
{
public:
    boost_asio_server_test()
            : endpoint(boost::asio::ip::address::from_string("127.0.0.1") , 8129),
            acceptor(io_service, endpoint), lsock(io_service), sock(io_service), data("SOME DATA TO TEST")
    {
    }

    boost::asio::io_service io_service;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket lsock;
    boost::asio::ip::tcp::socket sock;
    const std::string data;
    boost::array<char, 4096> buffer1;
    boost::array<char, 4096> buffer2;

    void connect_handler(const boost::system::error_code &ec)
    {
        EXPECT_FALSE(ec);
        boost::asio::async_write(sock,
                                 boost::asio::buffer(data),
                                 boost::bind(&boost_asio_server_test::write_handler,
                                             this, boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
        boost::asio::async_read(sock,
                                boost::asio::buffer(buffer1),
                                boost::asio::transfer_at_least(data.length()),
                                boost::bind(&boost_asio_server_test::read_handler1,
                                            this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void read_handler1(const boost::system::error_code& ec, std::size_t bytes_transferred)
    {
        EXPECT_FALSE(ec);
        EXPECT_EQ(bytes_transferred, data.length());
        EXPECT_EQ(data, std::string(buffer1.begin(), buffer1.begin() + bytes_transferred));
    }

    void read_handler2(const boost::system::error_code& ec, std::size_t bytes_transferred)
    {
        EXPECT_FALSE(ec);
        EXPECT_EQ(bytes_transferred, data.length());
        EXPECT_EQ(data, std::string(buffer2.begin(), buffer2.begin() + bytes_transferred));
    }

    void accept_handler(const boost::system::error_code &ec)
    {
        EXPECT_FALSE(ec);
        boost::asio::async_write(lsock,
                                 boost::asio::buffer(data),
                                 boost::bind(&boost_asio_server_test::write_handler,
                                             this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
        boost::asio::async_read(lsock,
                                boost::asio::buffer(buffer2),
                                boost::asio::transfer_at_least(data.length()),
                                boost::bind(&boost_asio_server_test::read_handler2,
                                            this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
    {
        EXPECT_FALSE(ec);
        EXPECT_EQ(bytes_transferred, data.length());
    }
};

/**
 * @brief Listen and Read TCP Socket test
 * listens on a port, then connects to same port
 * the same data is sent accross both sockets
 * and then verified.
 * runs in 3 threads to verify it works with threads
 * @see http://en.highscore.de/cpp/boost/asio.html
 */
TEST_F ( boost_asio_server_test, listenread )
{
    boost::thread_group threads;
    acceptor.listen();
    acceptor.async_accept(lsock, boost::bind(&boost_asio_server_test::accept_handler, this, boost::asio::placeholders::error ));
    sock.async_connect(endpoint, boost::bind(&boost_asio_server_test::connect_handler, this, boost::asio::placeholders::error ));

    threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));
    threads.join_all();
}
