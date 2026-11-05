#include "OnlineClient.hpp"

#include <cstddef>
#include <utility>
#include <vector>

#include "Action.hpp"
#include "EngineError.hpp"
#include "PortOption.hpp"
#include "Protocol.hpp"
#include "TextRules.hpp"
#include "UdpTransport.hpp"

namespace engine::client {

OnlineClient::OnlineClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game, const network::NetworkSettings &settings, std::optional<std::string> address)
    : m_view(assetsDirectory, dataDirectory, game), m_settings(settings), m_network(),
      m_address(address.value_or(m_view.settings().serverAddress())), m_slot(), m_lockedRooms(), m_pendingRoom(0), m_lastRoomList(), m_lastRoomUpdate(), m_metrics(), m_console(m_view)
{
    enterScene(m_view.currentScene());
}

void OnlineClient::run()
{
    while (m_view.isOpen()) {
        const Time now = std::chrono::steady_clock::now();
        m_view.processEvents();
        updateNetwork(now);
        m_view.frame();
        if (m_view.takeLoadingFinished())
            enterScene(m_view.currentScene());
        if (m_view.debugPressed())
            DebugMetrics::toggle(m_view.uiStore());
        handleConsole();
        m_metrics.frame(m_view.uiStore(), m_view.entityCount(), m_network ? m_network->bytesReceived() : 0,
            m_network ? m_network->bytesSent() : 0, m_network ? m_network->pingMs() : 0);
        handleActions();
    }
    m_view.close();
    if (m_network)
        m_network->disconnect();
}

void OnlineClient::updateNetwork(Time now)
{
    if (!m_network)
        return;
    try {
        m_network->update(buttons());
        for (const network::Packet &event : m_network->takeEvents())
            handleEvent(event, now);
        if (auto snapshot = m_network->takeSnapshot())
            applySnapshot(*snapshot);
    } catch (const EngineError &) {
        disconnect("connectionFailed");
        return;
    }
    const std::string &scene = m_view.currentScene();
    m_view.uiStore().setValue("lobby.status", m_view.message(m_network->connected() ? "online" : "connecting"));
    if (scene == LOBBY_SCENE && m_network->connected() && now - m_lastRoomList >= ROOM_LIST_INTERVAL) {
        m_network->listRooms();
        m_lastRoomList = now;
    }
    if ((scene == ROOM_SCENE || scene == GAME_SCENE) && now - m_lastRoomUpdate >= ROOM_SILENCE_LIMIT) {
        enterScene(LOBBY_SCENE);
        m_view.uiStore().setValue("lobby.error", m_view.message("roomLost"));
    }
}

void OnlineClient::handleActions()
{
    std::vector<std::string> actions;
    const std::vector<ecs::UiActionEvent> events = m_view.takeUiActions();
    if (m_console.open())
        return;
    for (const ecs::UiActionEvent &event : events)
        actions.push_back(event.action());
    if (m_view.escapePressed() && popupOpen())
        actions.push_back("close_popup");
    else if (m_view.escapePressed() && !m_view.escapeAction().empty())
        actions.push_back(m_view.escapeAction());
    for (const std::string &action : actions)
        handleAction(action);
}

void OnlineClient::handleAction(const std::string &action)
{
    if (action == "quit")
        m_view.closeWindow();
    else if (action == "connect")
        connect();
    else if (action == "disconnect")
        disconnect({});
    else if (startsWith(action, "scene:"))
        enterScene(action.substr(std::string("scene:").size()));
    else if (!m_network)
        return;
    else if (action == "refresh_rooms")
        m_network->listRooms();
    else if (action == "create_room")
        createRoom();
    else if (startsWith(action, "join:"))
        joinOrAsk(static_cast<std::uint16_t>(std::stoul(action.substr(std::string("join:").size()))));
    else if (action == "join_code")
        m_network->joinRoom(m_pendingRoom, field("joinCode"));
    else if (action == "close_popup")
        closePopup();
    else if (action == "leave_room") {
        m_network->leaveRoom();
        m_slot.reset();
        enterScene(LOBBY_SCENE);
    } else if (action == "start_game")
        m_network->startGame();
    else if (startsWith(action, "kick:"))
        m_network->kickPlayer(static_cast<std::uint8_t>(std::stoul(action.substr(std::string("kick:").size()))));
    else if (action == "delete_room")
        m_network->deleteRoom();
}

void OnlineClient::handleEvent(const network::Packet &event, Time now)
{
    ecs::UiStore &store = m_view.uiStore();
    if (event.holds<network::RoomListMessage>()) {
        showRoomList(event.as<network::RoomListMessage>());
    } else if (event.holds<network::RoomJoinedMessage>()) {
        const network::RoomJoinedMessage &joined = event.as<network::RoomJoinedMessage>();
        m_slot = joined.slot();
        m_lastRoomUpdate = now;
        store.setValue("room.name", joined.name());
        store.setValue("lobby.error", {});
        if (m_view.currentScene() != ROOM_SCENE && m_view.currentScene() != GAME_SCENE)
            enterScene(ROOM_SCENE);
    } else if (event.holds<network::RoomRejectedMessage>()) {
        handleRejection(event.as<network::RoomRejectedMessage>().reason());
    } else if (event.holds<network::RoomUpdateMessage>()) {
        m_lastRoomUpdate = now;
        showMembers(event.as<network::RoomUpdateMessage>());
    } else if (event.holds<network::GameStartedMessage>()) {
        m_lastRoomUpdate = now;
        enterScene(GAME_SCENE);
    } else if (event.holds<network::KickedMessage>() || event.holds<network::RoomClosedMessage>()) {
        m_slot.reset();
        enterScene(LOBBY_SCENE);
        store.setValue("lobby.error", m_view.message(event.holds<network::KickedMessage>() ? "kicked" : "roomClosed"));
    } else if (event.holds<network::ConsoleTextMessage>()) {
        m_console.print(event.as<network::ConsoleTextMessage>().text());
    } else if (event.holds<network::PlaySoundMessage>() && m_view.currentScene() == GAME_SCENE) {
        m_view.playEventSound(event.as<network::PlaySoundMessage>().sound());
    } else if (event.holds<network::EntityDestroyedMessage>() && m_view.currentScene() == GAME_SCENE) {
        m_view.destroyEntity(event.as<network::EntityDestroyedMessage>().entityId());
    }
}

void OnlineClient::handleRejection(std::uint8_t reason)
{
    static constexpr const char *KEYS[] = {"", "roomNotFound", "roomFull", "wrongCode", "tooManyRooms",
        "invalidRoomName", "tooManyAttempts", "notAllowed"};
    const std::string key = reason < std::size(KEYS) ? KEYS[reason] : "notAllowed";
    std::string target = m_view.currentScene() == ROOM_SCENE ? "room.error" : "lobby.error";
    if (popupOpen())
        target = "lobby.popupError";
    m_view.uiStore().setValue(target, m_view.message(key));
}

void OnlineClient::createRoom()
{
    const std::string name = field("roomName");
    const std::string code = field("roomCode");
    if (!network::TextRules::isValid(name, 1, m_settings.roomNameMaxLength())
        || !network::TextRules::isValid(code, 0, m_settings.roomCodeMaxLength())) {
        m_view.uiStore().setValue("lobby.error", m_view.message("invalidRoomName"));
        return;
    }
    m_network->createRoom(name, code, ruleValues());
}

void OnlineClient::joinOrAsk(std::uint16_t roomId)
{
    if (m_lockedRooms.contains(roomId))
        openPopup(roomId);
    else
        m_network->joinRoom(roomId, {});
}

void OnlineClient::openPopup(std::uint16_t roomId)
{
    m_pendingRoom = roomId;
    m_view.setFieldText("joinCode", {});
    m_view.uiStore().setValue("lobby.popupError", {});
    m_view.uiStore().setValue("lobby.popup", YES);
}

void OnlineClient::closePopup()
{
    m_view.uiStore().setValue("lobby.popup", NO);
    m_view.uiStore().setValue("lobby.popupError", {});
}

bool OnlineClient::popupOpen()
{
    return m_view.currentScene() == LOBBY_SCENE && m_view.uiStore().value("lobby.popup") == YES;
}

void OnlineClient::connect()
{
    m_address = field("serverAddress");
    const std::size_t colon = m_address.rfind(':');
    try {
        const std::string host = colon == std::string::npos ? m_address : m_address.substr(0, colon);
        const std::uint16_t port =
            colon == std::string::npos ? m_settings.defaultPort() : PortOption::parse(m_address.substr(colon + 1));
        m_network = std::make_unique<network::ClientConnection>(net::UdpTransport::resolve(host, port), m_settings);
    } catch (const EngineError &) {
        m_view.uiStore().setValue("menu.error", m_view.message("invalidAddress"));
        return;
    }
    m_lastRoomList = Time();
    enterScene(LOBBY_SCENE);
}

void OnlineClient::disconnect(const std::string &errorKey)
{
    if (m_network)
        m_network->disconnect();
    m_network.reset();
    m_slot.reset();
    enterScene(MENU_SCENE);
    m_view.uiStore().setValue("menu.error", errorKey.empty() ? std::string() : m_view.message(errorKey));
}

void OnlineClient::enterScene(const std::string &scene)
{
    if (scene != m_view.currentScene())
        m_view.switchScene(scene);
    if (m_console.open() && scene != ROOM_SCENE && scene != GAME_SCENE)
        m_console.toggle();
    ecs::UiStore &store = m_view.uiStore();
    store.setValue("menu.error", {});
    store.setValue("lobby.error", {});
    store.setValue("room.error", {});
    store.setValue("lobby.popup", NO);
    store.setValue("lobby.popupError", {});
    if (scene == MENU_SCENE)
        m_view.setFieldText("serverAddress", m_address);
}

void OnlineClient::showRoomList(const network::RoomListMessage &list)
{
    std::vector<ecs::UiStore::Row> rows;
    m_lockedRooms.clear();
    for (const network::RoomInfo &room : list.rooms()) {
        if (room.locked())
            m_lockedRooms.insert(room.id());
        rows.push_back({{"id", std::to_string(room.id())}, {"name", room.name()},
            {"players", std::to_string(room.players()) + "/" + std::to_string(room.capacity())},
            {"lock", room.locked() ? m_view.message("locked") : std::string()},
            {"status", m_view.message(room.playing() ? "playing" : "waiting")}});
    }
    m_view.uiStore().setValue("lobby.empty", rows.empty() ? m_view.message("noRooms") : std::string());
    m_view.uiStore().setList("rooms", std::move(rows));
}

void OnlineClient::showMembers(const network::RoomUpdateMessage &update)
{
    const bool owner = m_slot && *m_slot == update.ownerSlot();
    std::vector<ecs::UiStore::Row> rows;
    for (std::size_t slot = 0; slot < update.occupied().size(); ++slot) {
        const bool occupied = update.occupied()[slot];
        std::string tag;
        if (occupied && slot == update.ownerSlot())
            tag = m_view.message("ownerTag");
        if (occupied && m_slot && slot == *m_slot)
            tag += m_view.message("youTag");
        const bool canKick = owner && occupied && slot != *m_slot;
        rows.push_back({{"slot", std::to_string(slot)}, {"number", std::to_string(slot + 1)},
            {"name", occupied ? "P" + std::to_string(slot + 1) : m_view.message("emptySlot")}, {"tag", tag},
            {"canKick", canKick ? YES : NO}});
    }
    m_view.uiStore().setValue("room.isOwner", owner ? YES : NO);
    m_view.uiStore().setList("members", std::move(rows));
}

void OnlineClient::applySnapshot(const network::SnapshotMessage &snapshot)
{
    if (m_view.currentScene() == ROOM_SCENE)
        enterScene(GAME_SCENE);
    if (m_view.currentScene() != GAME_SCENE)
        return;
    m_view.beginSync();
    for (const network::EntityState &entity : snapshot.entities())
        m_view.syncEntity(
            entity.id(), entity.kind(), static_cast<float>(entity.x()), static_cast<float>(entity.y()), entity.visual());
    m_view.endSync();
    m_view.setPhase(toPhase(snapshot.gameState()));
    showWorld(snapshot);
    for (std::size_t slot = 0; slot < snapshot.players().size(); ++slot) {
        const network::PlayerStatus &player = snapshot.players()[slot];
        showPlayer(slot, player);
        if (player.state() == network::Protocol::SLOT_EMPTY)
            m_view.removePlayer(slot);
        else
            m_view.setPlayer(slot, player.lives(), player.score(), player.avatarId());
    }
}

void OnlineClient::showWorld(const network::SnapshotMessage &snapshot)
{
    ecs::UiStore &store = m_view.uiStore();
    const scene::WorldValueTable &table = m_view.worldTable();
    const auto &values = snapshot.values();
    for (std::size_t index = 0; index < values.size() && index < table.size(); ++index)
        store.setValue("world." + table.name(index), table.label(index, values[index]));
    store.setValue("game.phase", phaseName(snapshot.gameState()));
}

void OnlineClient::showPlayer(std::size_t slot, const network::PlayerStatus &player)
{
    ecs::UiStore &store = m_view.uiStore();
    const std::string prefix = "player" + std::to_string(slot) + ".";
    store.setValue(prefix + "present", player.state() == network::Protocol::SLOT_EMPTY ? NO : YES);
    store.setValue(prefix + "alive", player.state() == network::Protocol::SLOT_ALIVE ? YES : NO);
    store.setValue(prefix + "health", ratio(player.health()));
    store.setValue(prefix + "score", std::to_string(player.score()));
    const scene::WorldValueTable &table = m_view.playerTable();
    for (std::size_t index = 0; index < player.values().size() && index < table.size(); ++index)
        store.setValue(prefix + table.name(index), table.label(index, player.values()[index]));
}

std::string OnlineClient::ratio(std::uint8_t percent)
{
    return std::to_string(static_cast<float>(percent) / static_cast<float>(network::Protocol::MAX_PERCENT));
}

std::string OnlineClient::phaseName(std::uint8_t gameState)
{
    if (gameState == network::Protocol::GAME_PLAYING)
        return "playing";
    if (gameState == network::Protocol::GAME_OVER)
        return "over";
    return "waiting";
}

std::uint8_t OnlineClient::buttons() const
{
    using input::Action;

    if (m_view.currentScene() != GAME_SCENE || m_console.open())
        return 0;
    std::uint8_t bits = 0;
    if (m_view.isPressed(Action::Up))
        bits |= network::Protocol::BUTTON_UP;
    if (m_view.isPressed(Action::Down))
        bits |= network::Protocol::BUTTON_DOWN;
    if (m_view.isPressed(Action::Left))
        bits |= network::Protocol::BUTTON_LEFT;
    if (m_view.isPressed(Action::Right))
        bits |= network::Protocol::BUTTON_RIGHT;
    if (m_view.isPressed(Action::Fire))
        bits |= network::Protocol::BUTTON_FIRE;
    if (m_view.isPressed(Action::Special))
        bits |= network::Protocol::BUTTON_ACTION;
    return bits;
}

std::string OnlineClient::field(const std::string &name)
{
    return m_view.uiStore().value("field." + name);
}

bool OnlineClient::startsWith(const std::string &text, const std::string &prefix)
{
    return text.rfind(prefix, 0) == 0;
}

ecs::Match::Phase OnlineClient::toPhase(std::uint8_t gameState)
{
    if (gameState == network::Protocol::GAME_PLAYING)
        return ecs::Match::Phase::Playing;
    if (gameState == network::Protocol::GAME_OVER)
        return ecs::Match::Phase::Over;
    return ecs::Match::Phase::Waiting;
}

std::vector<std::uint8_t> OnlineClient::ruleValues()
{
    const scene::WorldValueTable &rules = m_view.ruleTable();
    std::vector<std::uint8_t> values;
    for (std::size_t index = 0; index < rules.size(); ++index) {
        const auto value = rules.valueOf(index, field(RULE_PREFIX + rules.name(index)));
        values.push_back(static_cast<std::uint8_t>(value.value_or(0)));
    }
    return values;
}

void OnlineClient::handleConsole()
{
    const bool consoleScene = m_view.currentScene() == ROOM_SCENE || m_view.currentScene() == GAME_SCENE;
    if (m_view.consolePressed() && (consoleScene || m_console.open()))
        m_console.toggle();
    if (!m_console.open() || !m_view.enterPressed())
        return;
    const auto command = m_console.submit();
    if (!command)
        return;
    if (m_network && m_network->connected())
        m_network->sendCommand(*command);
    else
        m_console.print("not connected to a server");
}

} // namespace engine::client
