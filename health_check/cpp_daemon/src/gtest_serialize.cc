#include<gtest/gtest.h>
#include <stdint.h>
#include "serialize.h"
#include "cache_object.h"

class serialize : public ::testing::Test
{
public:
    class testdata_t : public ibiki::serializable<9>
    {
    public:
        class autofill {};

        testdata_t()
        {
        }

        explicit testdata_t(autofill)
                : data1("test test 1"),
                data2("2 test 2"),
                data3(std::string("test 3") + '\0' + '\r' + '\r' + '\n' + '\t' + '\r'),
                a(-1201),
                b(1201),
                c(0),
                d(0x7fffffffffffffffLL),
                e(20883832u),
                f(0xffffffffffffffffLL),
                g(-12032322)
        {
        }

        std::string data1;
        std::string data2;
        std::string data3;
        int a;
        int b;
        int c;
        uint64_t d;
        uint64_t e;
        int64_t f;
        int64_t g;

        void eq(const testdata_t &other) const
        {
            EXPECT_EQ(data1, other.data1);
            EXPECT_EQ(data2, other.data2);
            EXPECT_EQ(data3, other.data3);
            EXPECT_EQ(a, other.a);
            EXPECT_EQ(b, other.b);
            EXPECT_EQ(c, other.c);
            EXPECT_EQ(d, other.d);
            EXPECT_EQ(e, other.e);
            EXPECT_EQ(f, other.f);
            EXPECT_EQ(g, other.g);
        }

        template<class Serializer>
        void serialize(Serializer &s) //NOLINT
        {
            s % data1 % a % data2 % b % c % d % data3 % e % f % g;
        }
    };

    boost::asio::io_service io_service;
    ibiki::buffer_cache_t cache; ///10mb cache
    ibiki::object_cache_t ocache;
    const testdata_t filled;
    bool finaltest;

    serialize()
            : io_service(), cache(&io_service, 10485760u), ocache(&io_service, 10485760u), filled(testdata_t::autofill()), finaltest(false)
    {
    }

    void onNewObject(boost::shared_ptr<const testdata_t> object)
    {
        ASSERT_TRUE(object);
        filled.eq(*object);
        finaltest = true;
    }

    void onBuffer(boost::shared_ptr<ibiki::buffer_cache_t::buffer_t> buffer)
    {
        ///verify it will read correctly
        testdata_t test;
        ibiki::serialize_reader_t reader(buffer);
        ASSERT_TRUE(reader >> test);
        ASSERT_TRUE(reader);
        filled.eq(test);

        ocache.unserialize<testdata_t>(this, buffer, boost::bind(&serialize::onNewObject, this, _1));

//         std::cout << "Buffer Contents:" << std::endl << buffer->dump_hex() << std::endl;
    }

    void onObject(boost::shared_ptr<const testdata_t> object)
    {
        cache.serialize<testdata_t>(this, object, boost::bind(&serialize::onBuffer, this, _1));
    }
};

TEST_F ( serialize, test)
{
    boost::shared_ptr<testdata_t> filledbuf(new testdata_t(testdata_t::autofill()));
    ocache.insert<testdata_t>(this, filledbuf, boost::bind(&serialize::onObject, this, _1));

    boost::thread_group threads;
    for (int i = 0; i < 4; ++i)
        threads.create_thread(boost::bind(&boost::asio::io_service::run, &io_service));

    threads.join_all();
    ASSERT_TRUE(finaltest);
}
