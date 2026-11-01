#include "DataNode.hpp"

#include <fstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "SceneError.hpp"

namespace engine::scene {

class DataNode::Impl
{
public:
    Impl(std::shared_ptr<const nlohmann::json> root, const nlohmann::json *node, std::string path)
        : m_root(std::move(root)), m_node(node), m_path(std::move(path))
    {
    }

    const nlohmann::json &node() const
    {
        return *m_node;
    }

    const std::shared_ptr<const nlohmann::json> &root() const
    {
        return m_root;
    }

    const std::string &path() const
    {
        return m_path;
    }

private:
    std::shared_ptr<const nlohmann::json> m_root;
    const nlohmann::json *m_node;
    std::string m_path;
};

namespace {

const nlohmann::json &member(const nlohmann::json &node, const std::string &key, const std::string &path)
{
    if (!node.is_object() || !node.contains(key))
        throw SceneError("DataNode: missing key " + path + "." + key);
    return node.at(key);
}

float toFloat(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_number())
        throw SceneError("DataNode: " + path + " must be a number");
    return value.get<float>();
}

int toInt(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_number_integer())
        throw SceneError("DataNode: " + path + " must be an integer");
    return value.get<int>();
}

std::string toString(const nlohmann::json &value, const std::string &path)
{
    if (!value.is_string())
        throw SceneError("DataNode: " + path + " must be a string");
    return value.get<std::string>();
}

} // namespace

DataNode::DataNode(std::shared_ptr<const Impl> impl) : m_impl(std::move(impl))
{
}

DataNode DataNode::fromFile(const std::filesystem::path &path)
{
    std::ifstream file(path);
    if (!file)
        throw SceneError("DataNode: cannot open " + path.string());
    try {
        auto root = std::make_shared<const nlohmann::json>(nlohmann::json::parse(file));
        const nlohmann::json *node = root.get();
        return DataNode(std::make_shared<const Impl>(root, node, path.filename().string()));
    } catch (const nlohmann::json::exception &error) {
        throw SceneError("DataNode: invalid JSON in " + path.string() + ": " + error.what());
    }
}

DataNode DataNode::fromString(const std::string &text, const std::string &source)
{
    try {
        auto root = std::make_shared<const nlohmann::json>(nlohmann::json::parse(text));
        const nlohmann::json *node = root.get();
        return DataNode(std::make_shared<const Impl>(root, node, source));
    } catch (const nlohmann::json::exception &error) {
        throw SceneError("DataNode: invalid JSON in " + source + ": " + error.what());
    }
}

bool DataNode::has(const std::string &key) const
{
    return m_impl->node().is_object() && m_impl->node().contains(key);
}

DataNode DataNode::child(const std::string &key) const
{
    const nlohmann::json &value = member(m_impl->node(), key, m_impl->path());
    return DataNode(std::make_shared<const Impl>(m_impl->root(), &value, m_impl->path() + "." + key));
}

std::vector<std::string> DataNode::keys() const
{
    if (!m_impl->node().is_object())
        throw SceneError("DataNode: " + m_impl->path() + " must be an object");
    std::vector<std::string> keys;
    for (const auto &item : m_impl->node().items())
        keys.push_back(item.key());
    return keys;
}

std::vector<DataNode> DataNode::items() const
{
    if (!m_impl->node().is_array())
        throw SceneError("DataNode: " + m_impl->path() + " must be an array");
    std::vector<DataNode> items;
    std::size_t index = 0;
    for (const nlohmann::json &value : m_impl->node()) {
        const std::string path = m_impl->path() + "[" + std::to_string(index++) + "]";
        items.push_back(DataNode(std::make_shared<const Impl>(m_impl->root(), &value, path)));
    }
    return items;
}

float DataNode::getFloat(const std::string &key) const
{
    return toFloat(member(m_impl->node(), key, m_impl->path()), m_impl->path() + "." + key);
}

float DataNode::getFloat(const std::string &key, float fallback) const
{
    return has(key) ? getFloat(key) : fallback;
}

int DataNode::getInt(const std::string &key) const
{
    return toInt(member(m_impl->node(), key, m_impl->path()), m_impl->path() + "." + key);
}

int DataNode::getInt(const std::string &key, int fallback) const
{
    return has(key) ? getInt(key) : fallback;
}

bool DataNode::getBool(const std::string &key, bool fallback) const
{
    if (!has(key))
        return fallback;
    const nlohmann::json &value = m_impl->node().at(key);
    if (!value.is_boolean())
        throw SceneError("DataNode: " + m_impl->path() + "." + key + " must be a boolean");
    return value.get<bool>();
}

std::string DataNode::getString(const std::string &key) const
{
    return toString(member(m_impl->node(), key, m_impl->path()), m_impl->path() + "." + key);
}

float DataNode::asFloat() const
{
    return toFloat(m_impl->node(), m_impl->path());
}

int DataNode::asInt() const
{
    return toInt(m_impl->node(), m_impl->path());
}

std::string DataNode::asString() const
{
    return toString(m_impl->node(), m_impl->path());
}

const std::string &DataNode::path() const
{
    return m_impl->path();
}

} // namespace engine::scene
