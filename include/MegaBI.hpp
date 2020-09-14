#ifndef MEGABI_H
#define MEGABI_H

#define HAS_UNCAUGHT_EXCEPTIONS 1 // https://github.com/HowardHinnant/date/issues/338
#include <date/include/date.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <algorithm>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace MegaBI
{

const char TS_FACT[] = "ts_fact";
const char FACT_NAME[] = "fact_name";
const char PROPS[] = "props";
const char COUNT[] = "count";

inline date::year_month_day TS2YMD(const std::uint32_t ts)
{
    auto tp = std::chrono::system_clock::from_time_t(ts);
    return date::year_month_day(date::floor<date::days>(tp));
}

struct Key
{
    Key(std::uint32_t ts,
        const std::string& factName) : ymd(TS2YMD(ts)), fact(factName)
    {
        props.reserve(10);
    }

    friend bool operator<(const Key& lhs, const Key& rhs)
    {
        return std::tie(lhs.ymd, lhs.fact, lhs.props)
            < std::tie(rhs.ymd, rhs.fact, rhs.props);
    }

    date::year_month_day ymd;
    std::string fact;
    std::vector<std::int32_t> props;
};

class Accumulator
{
public:
    bool parse(const std::string& jsonStr)
    {
        namespace json = rapidjson;

        json::Document record;
        record.Parse(jsonStr.c_str());

        if (record.HasParseError()
            || !record.HasMember(TS_FACT)
            || !record.HasMember(FACT_NAME)
            || !record.HasMember(PROPS))
        {
            return false;
        }

        Key key(record[TS_FACT].GetInt(), record[FACT_NAME].GetString());

        for (auto it = record[PROPS].MemberBegin();
            it != record[PROPS].MemberEnd(); ++it)
        {
            key.props.emplace_back(it->value.GetInt());
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        ++m_storage[key];

        return true;
    }

    Accumulator& operator+=(const Accumulator& rhs)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::for_each(std::begin(rhs.m_storage), std::end(rhs.m_storage),
            [&](const auto& p)
            {
                m_storage[p.first] += p.second;
            });

        return *this;
    }

    friend std::ostream& operator<<(std::ostream& stream, Accumulator& acc)
    {
        std::lock_guard<std::mutex> lock(acc.m_mutex);

        namespace json = rapidjson;

        for (auto& [key, value] : acc.m_storage)
        {
            json::Document record;
            record.SetObject();

            {
                std::stringstream ss;
                ss << key.ymd;
                json::Value ts;
                ts.SetString(std::string(ss.str()).c_str(), record.GetAllocator());
                record.AddMember(TS_FACT, ts, record.GetAllocator());
            }

            {
                json::Value name;
                name.SetString(key.fact.c_str(), record.GetAllocator());
                record.AddMember(FACT_NAME, name, record.GetAllocator());
            }

            {
                json::Value props(rapidjson::kArrayType);
                for (auto& prop : key.props)
                {
                    props.PushBack(prop, record.GetAllocator());
                }
                record.AddMember(PROPS, props, record.GetAllocator());
            }

            {
                record.AddMember(COUNT, value, record.GetAllocator());
            }

            json::StringBuffer buf;
            json::Writer<json::StringBuffer> writer(buf);
            record.Accept(writer);

            stream << buf.GetString() << std::endl;
        }

        return stream;
    }

    std::size_t size() const { return m_storage.size(); }
    void clear() { m_storage.clear(); }

private:

    std::map<Key, std::int32_t> m_storage;
    std::mutex m_mutex;
};

}

#endif  // MEGABI_H