/**
 * @file ir.cpp
 * @brief Intermediate Representation (IR) implementation
 */

#include "NovelMind/scripting/ir.hpp"
#include <algorithm>
#include <queue>
#include <sstream>
#include <unordered_set>

namespace NovelMind::scripting {

// ============================================================================
// IRNode Implementation
// ============================================================================

IRNode::IRNode(NodeId id, IRNodeType type) : m_id(id), m_type(type) {}

const char *IRNode::getTypeName() const {
  switch (m_type) {
  case IRNodeType::SceneStart:
    return "SceneStart";
  case IRNodeType::SceneEnd:
    return "SceneEnd";
  case IRNodeType::Comment:
    return "Comment";
  case IRNodeType::Sequence:
    return "Sequence";
  case IRNodeType::Branch:
    return "Branch";
  case IRNodeType::Switch:
    return "Switch";
  case IRNodeType::Loop:
    return "Loop";
  case IRNodeType::Goto:
    return "Goto";
  case IRNodeType::Label:
    return "Label";
  case IRNodeType::ShowCharacter:
    return "ShowCharacter";
  case IRNodeType::HideCharacter:
    return "HideCharacter";
  case IRNodeType::ShowBackground:
    return "ShowBackground";
  case IRNodeType::Dialogue:
    return "Dialogue";
  case IRNodeType::Choice:
    return "Choice";
  case IRNodeType::ChoiceOption:
    return "ChoiceOption";
  case IRNodeType::PlayMusic:
    return "PlayMusic";
  case IRNodeType::StopMusic:
    return "StopMusic";
  case IRNodeType::PlaySound:
    return "PlaySound";
  case IRNodeType::Transition:
    return "Transition";
  case IRNodeType::Wait:
    return "Wait";
  case IRNodeType::SetVariable:
    return "SetVariable";
  case IRNodeType::GetVariable:
    return "GetVariable";
  case IRNodeType::Expression:
    return "Expression";
  case IRNodeType::FunctionCall:
    return "FunctionCall";
  case IRNodeType::Custom:
    return "Custom";
  }
  return "Unknown";
}

void IRNode::setProperty(const std::string &name,
                         const IRPropertyValue &value) {
  m_properties[name] = value;
}

std::optional<IRPropertyValue>
IRNode::getProperty(const std::string &name) const {
  auto it = m_properties.find(name);
  if (it != m_properties.end()) {
    return it->second;
  }
  return std::nullopt;
}

std::string IRNode::getStringProperty(const std::string &name,
                                      const std::string &defaultValue) const {
  auto prop = getProperty(name);
  if (prop && std::holds_alternative<std::string>(*prop)) {
    return std::get<std::string>(*prop);
  }
  return defaultValue;
}

i64 IRNode::getIntProperty(const std::string &name, i64 defaultValue) const {
  auto prop = getProperty(name);
  if (prop && std::holds_alternative<i64>(*prop)) {
    return std::get<i64>(*prop);
  }
  return defaultValue;
}

f64 IRNode::getFloatProperty(const std::string &name, f64 defaultValue) const {
  auto prop = getProperty(name);
  if (prop && std::holds_alternative<f64>(*prop)) {
    return std::get<f64>(*prop);
  }
  return defaultValue;
}

bool IRNode::getBoolProperty(const std::string &name, bool defaultValue) const {
  auto prop = getProperty(name);
  if (prop && std::holds_alternative<bool>(*prop)) {
    return std::get<bool>(*prop);
  }
  return defaultValue;
}

void IRNode::setSourceLocation(const SourceLocation &loc) { m_location = loc; }

void IRNode::setPosition(f32 x, f32 y) {
  m_x = x;
  m_y = y;
}

std::vector<PortDefinition> IRNode::getInputPorts() const {
  std::vector<PortDefinition> ports;

  // All nodes have execution input except SceneStart
  if (m_type != IRNodeType::SceneStart) {
    ports.push_back({"exec_in", "In", true, false, ""});
  }

  // Type-specific input ports
  switch (m_type) {
  case IRNodeType::Branch:
    ports.push_back({"condition", "Condition", false, true, ""});
    break;
  case IRNodeType::ShowCharacter:
    ports.push_back({"character", "Character", false, true, ""});
    ports.push_back({"position", "Position", false, false, "center"});
    ports.push_back({"expression", "Expression", false, false, "default"});
    break;
  case IRNodeType::HideCharacter:
    ports.push_back({"character", "Character", false, true, ""});
    break;
  case IRNodeType::ShowBackground:
    ports.push_back({"background", "Background", false, true, ""});
    break;
  case IRNodeType::Dialogue:
    ports.push_back({"character", "Character", false, false, ""});
    ports.push_back({"text", "Text", false, true, ""});
    break;
  case IRNodeType::Choice:
    break;
  case IRNodeType::PlayMusic:
  case IRNodeType::PlaySound:
    ports.push_back({"track", "Track", false, true, ""});
    ports.push_back({"volume", "Volume", false, false, "1.0"});
    break;
  case IRNodeType::Wait:
    ports.push_back({"duration", "Duration", false, true, ""});
    break;
  case IRNodeType::SetVariable:
    ports.push_back({"name", "Name", false, true, ""});
    ports.push_back({"value", "Value", false, true, ""});
    break;
  default:
    break;
  }

  return ports;
}

std::vector<PortDefinition> IRNode::getOutputPorts() const {
  std::vector<PortDefinition> ports;

  switch (m_type) {
  case IRNodeType::SceneStart:
  case IRNodeType::Sequence:
  case IRNodeType::ShowCharacter:
  case IRNodeType::HideCharacter:
  case IRNodeType::ShowBackground:
  case IRNodeType::Dialogue:
  case IRNodeType::PlayMusic:
  case IRNodeType::StopMusic:
  case IRNodeType::PlaySound:
  case IRNodeType::Transition:
  case IRNodeType::Wait:
  case IRNodeType::SetVariable:
  case IRNodeType::Label:
    ports.push_back({"exec_out", "Out", true, false, ""});
    break;
  case IRNodeType::Branch:
    ports.push_back({"true", "True", true, false, ""});
    ports.push_back({"false", "False", true, false, ""});
    break;
  case IRNodeType::GetVariable:
    ports.push_back({"value", "Value", false, false, ""});
    break;
  case IRNodeType::Expression:
    ports.push_back({"result", "Result", false, false, ""});
    break;
  case IRNodeType::SceneEnd:
    break;
  default:
    break;
  }

  return ports;
}

std::string IRNode::toJson() const {
  std::stringstream ss;
  ss << "{";
  ss << "\"id\":" << m_id << ",";
  ss << "\"type\":\"" << getTypeName() << "\",";
  ss << "\"x\":" << m_x << ",";
  ss << "\"y\":" << m_y << ",";
  ss << "\"properties\":{";

  bool first = true;
  for (const auto &[name, value] : m_properties) {
    if (!first)
      ss << ",";
    first = false;
    ss << "\"" << name << "\":";

    std::visit(
        [&ss](const auto &v) {
          using T = std::decay_t<decltype(v)>;
          if constexpr (std::is_same_v<T, std::nullptr_t>) {
            ss << "null";
          } else if constexpr (std::is_same_v<T, bool>) {
            ss << (v ? "true" : "false");
          } else if constexpr (std::is_same_v<T, i64>) {
            ss << v;
          } else if constexpr (std::is_same_v<T, f64>) {
            ss << v;
          } else if constexpr (std::is_same_v<T, std::string>) {
            ss << "\"" << v << "\"";
          } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
            ss << "[";
            for (size_t i = 0; i < v.size(); ++i) {
              if (i > 0)
                ss << ",";
              ss << "\"" << v[i] << "\"";
            }
            ss << "]";
          }
        },
        value);
  }

  ss << "}}";
  return ss.str();
}

// ============================================================================
// IRGraph Implementation
// ============================================================================

IRGraph::IRGraph() = default;
IRGraph::~IRGraph() = default;

void IRGraph::setName(const std::string &name) { m_name = name; }

NodeId IRGraph::createNode(IRNodeType type) {
  NodeId id = m_nextId++;
  m_nodes[id] = std::make_unique<IRNode>(id, type);
  return id;
}

void IRGraph::removeNode(NodeId id) {
  disconnectAll(id);
  m_nodes.erase(id);
}

IRNode *IRGraph::getNode(NodeId id) {
  auto it = m_nodes.find(id);
  return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

const IRNode *IRGraph::getNode(NodeId id) const {
  auto it = m_nodes.find(id);
  return (it != m_nodes.end()) ? it->second.get() : nullptr;
}

std::vector<IRNode *> IRGraph::getNodes() {
  std::vector<IRNode *> result;
  for (auto &[id, node] : m_nodes) {
    result.push_back(node.get());
  }
  return result;
}

std::vector<const IRNode *> IRGraph::getNodes() const {
  std::vector<const IRNode *> result;
  for (const auto &[id, node] : m_nodes) {
    result.push_back(node.get());
  }
  return result;
}

std::vector<IRNode *> IRGraph::getNodesByType(IRNodeType type) {
  std::vector<IRNode *> result;
  for (auto &[id, node] : m_nodes) {
    if (node->getType() == type) {
      result.push_back(node.get());
    }
  }
  return result;
}

Result<void> IRGraph::connect(const PortId &source, const PortId &target) {
  auto *sourceNode = getNode(source.nodeId);
  auto *targetNode = getNode(target.nodeId);

  if (!sourceNode || !targetNode) {
    return Result<void>::error("Invalid node ID");
  }

  if (isConnected(source, target)) {
    return Result<void>::ok();
  }

  IRConnection conn;
  conn.source = source;
  conn.target = target;
  m_connections.push_back(conn);

  return Result<void>::ok();
}

void IRGraph::disconnect(const PortId &source, const PortId &target) {
  m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(),
                                     [&](const IRConnection &c) {
                                       return c.source == source &&
                                              c.target == target;
                                     }),
                      m_connections.end());
}

void IRGraph::disconnectAll(NodeId nodeId) {
  m_connections.erase(std::remove_if(m_connections.begin(), m_connections.end(),
                                     [nodeId](const IRConnection &c) {
                                       return c.source.nodeId == nodeId ||
                                              c.target.nodeId == nodeId;
                                     }),
                      m_connections.end());
}

std::vector<IRConnection> IRGraph::getConnections() const {
  return m_connections;
}

std::vector<IRConnection> IRGraph::getConnectionsFrom(NodeId nodeId) const {
  std::vector<IRConnection> result;
  for (const auto &conn : m_connections) {
    if (conn.source.nodeId == nodeId) {
      result.push_back(conn);
    }
  }
  return result;
}

std::vector<IRConnection> IRGraph::getConnectionsTo(NodeId nodeId) const {
  std::vector<IRConnection> result;
  for (const auto &conn : m_connections) {
    if (conn.target.nodeId == nodeId) {
      result.push_back(conn);
    }
  }
  return result;
}

bool IRGraph::isConnected(const PortId &source, const PortId &target) const {
  for (const auto &conn : m_connections) {
    if (conn.source == source && conn.target == target) {
      return true;
    }
  }
  return false;
}

std::vector<NodeId> IRGraph::getTopologicalOrder() const {
  std::vector<NodeId> result;
  std::unordered_map<NodeId, int> inDegree;
  std::queue<NodeId> queue;

  for (const auto &[id, node] : m_nodes) {
    inDegree[id] = 0;
  }

  for (const auto &conn : m_connections) {
    inDegree[conn.target.nodeId]++;
  }

  for (const auto &[id, degree] : inDegree) {
    if (degree == 0) {
      queue.push(id);
    }
  }

  while (!queue.empty()) {
    NodeId id = queue.front();
    queue.pop();
    result.push_back(id);

    for (const auto &conn : getConnectionsFrom(id)) {
      inDegree[conn.target.nodeId]--;
      if (inDegree[conn.target.nodeId] == 0) {
        queue.push(conn.target.nodeId);
      }
    }
  }

  return result;
}

std::vector<NodeId> IRGraph::getExecutionOrder() const {
  std::vector<NodeId> result;
  std::unordered_set<NodeId> visited;

  auto startNodes =
      const_cast<IRGraph *>(this)->getNodesByType(IRNodeType::SceneStart);
  for (auto *start : startNodes) {
    std::queue<NodeId> queue;
    queue.push(start->getId());

    while (!queue.empty()) {
      NodeId id = queue.front();
      queue.pop();

      if (visited.count(id)) {
        continue;
      }
      visited.insert(id);
      result.push_back(id);

      for (const auto &conn : getConnectionsFrom(id)) {
        if (conn.source.portName.find("exec") != std::string::npos ||
            conn.source.portName == "true" || conn.source.portName == "false") {
          queue.push(conn.target.nodeId);
        }
      }
    }
  }

  return result;
}

std::vector<std::string> IRGraph::validate() const {
  std::vector<std::string> errors;

  for (const auto &[id, node] : m_nodes) {
    if (node->getType() != IRNodeType::SceneStart &&
        node->getType() != IRNodeType::Comment) {
      auto incoming = getConnectionsTo(id);
      if (incoming.empty()) {
        errors.push_back("Node " + std::to_string(id) +
                         " has no incoming connections");
      }
    }
  }

  for (const auto &[id, node] : m_nodes) {
    for (const auto &port : node->getInputPorts()) {
      if (port.required && !port.isExecution) {
        bool found = false;
        for (const auto &conn : getConnectionsTo(id)) {
          if (conn.target.portName == port.name) {
            found = true;
            break;
          }
        }
        if (!found) {
          auto prop = node->getProperty(port.name);
          if (!prop || std::holds_alternative<std::nullptr_t>(*prop)) {
            errors.push_back("Node " + std::to_string(id) +
                             " missing required input: " + port.name);
          }
        }
      }
    }
  }

  return errors;
}

bool IRGraph::isValid() const { return validate().empty(); }

void IRGraph::addScene(const std::string &sceneName, NodeId startNode) {
  m_sceneStartNodes[sceneName] = startNode;
}

NodeId IRGraph::getSceneStartNode(const std::string &sceneName) const {
  auto it = m_sceneStartNodes.find(sceneName);
  return (it != m_sceneStartNodes.end()) ? it->second : 0;
}

std::vector<std::string> IRGraph::getSceneNames() const {
  std::vector<std::string> names;
  for (const auto &[name, id] : m_sceneStartNodes) {
    names.push_back(name);
  }
  return names;
}

void IRGraph::addCharacter(const std::string &id, const std::string &name,
                           const std::string &color) {
  m_characters[id] = {name, color};
}

bool IRGraph::hasCharacter(const std::string &id) const {
  return m_characters.count(id) > 0;
}

std::string IRGraph::toJson() const {
  std::stringstream ss;
  ss << "{";
  ss << "\"name\":\"" << m_name << "\",";

  ss << "\"nodes\":[";
  bool first = true;
  for (const auto &[id, node] : m_nodes) {
    if (!first)
      ss << ",";
    first = false;
    ss << node->toJson();
  }
  ss << "],";

  ss << "\"connections\":[";
  first = true;
  for (const auto &conn : m_connections) {
    if (!first)
      ss << ",";
    first = false;
    ss << "{";
    ss << "\"sourceNode\":" << conn.source.nodeId << ",";
    ss << "\"sourcePort\":\"" << conn.source.portName << "\",";
    ss << "\"targetNode\":" << conn.target.nodeId << ",";
    ss << "\"targetPort\":\"" << conn.target.portName << "\"";
    ss << "}";
  }
  ss << "],";

  ss << "\"scenes\":{";
  first = true;
  for (const auto &[name, id] : m_sceneStartNodes) {
    if (!first)
      ss << ",";
    first = false;
    ss << "\"" << name << "\":" << id;
  }
  ss << "},";

  ss << "\"characters\":{";
  first = true;
  for (const auto &[id, info] : m_characters) {
    if (!first)
      ss << ",";
    first = false;
    ss << "\"" << id << "\":{\"name\":\"" << info.first << "\",\"color\":\""
       << info.second << "\"}";
  }
  ss << "}";

  ss << "}";
  return ss.str();
}

// ============================================================================
// ASTToIRConverter Implementation
// ============================================================================

ASTToIRConverter::ASTToIRConverter() = default;
ASTToIRConverter::~ASTToIRConverter() = default;

Result<std::unique_ptr<IRGraph>>
ASTToIRConverter::convert(const Program &program) {
  m_graph = std::make_unique<IRGraph>();
  m_currentY = 0.0f;

  // Convert character declarations
  for (const auto &decl : program.characters) {
    convertCharacterDecl(decl);
  }

  // Convert scenes
  for (const auto &scene : program.scenes) {
    convertScene(scene);
    m_currentY += 200.0f;
  }

  return Result<std::unique_ptr<IRGraph>>::ok(std::move(m_graph));
}

void ASTToIRConverter::convertCharacterDecl(const CharacterDecl &decl) {
  m_graph->addCharacter(decl.id, decl.displayName, decl.color);
}

NodeId ASTToIRConverter::convertScene(const SceneDecl &scene) {
  NodeId startId = m_graph->createNode(IRNodeType::SceneStart);
  auto *startNode = m_graph->getNode(startId);
  startNode->setProperty("sceneName", scene.name);
  startNode->setPosition(100.0f, m_currentY);

  m_graph->addScene(scene.name, startId);

  NodeId prevNode = startId;
  f32 y = m_currentY + m_nodeSpacing;

  for (const auto &stmt : scene.body) {
    prevNode = convertStatement(*stmt, prevNode);
    y += m_nodeSpacing;
  }

  NodeId endId = m_graph->createNode(IRNodeType::SceneEnd);
  auto *endNode = m_graph->getNode(endId);
  endNode->setPosition(100.0f, y);

  PortId outPort{prevNode, "exec_out", true};
  PortId inPort{endId, "exec_in", false};
  m_graph->connect(outPort, inPort);

  return startId;
}

NodeId ASTToIRConverter::convertStatement(const Statement &stmt,
                                          NodeId prevNode) {
  // Use std::visit to handle the variant
  return std::visit(
      [this, prevNode, &stmt](const auto &stmtData) -> NodeId {
        using T = std::decay_t<decltype(stmtData)>;

        if constexpr (std::is_same_v<T, ShowStmt>) {
          IRNodeType nodeType =
              (stmtData.target == ShowStmt::Target::Background)
                  ? IRNodeType::ShowBackground
                  : IRNodeType::ShowCharacter;

          NodeId nodeId = createNodeAndConnect(nodeType, prevNode);
          auto *node = m_graph->getNode(nodeId);

          if (stmtData.target == ShowStmt::Target::Background) {
            node->setProperty("background", stmtData.identifier);
          } else {
            node->setProperty("character", stmtData.identifier);
          }
          node->setSourceLocation(stmt.location);

          return nodeId;
        } else if constexpr (std::is_same_v<T, HideStmt>) {
          NodeId nodeId =
              createNodeAndConnect(IRNodeType::HideCharacter, prevNode);
          auto *node = m_graph->getNode(nodeId);
          node->setProperty("character", stmtData.identifier);
          node->setSourceLocation(stmt.location);
          return nodeId;
        } else if constexpr (std::is_same_v<T, SayStmt>) {
          NodeId nodeId = createNodeAndConnect(IRNodeType::Dialogue, prevNode);
          auto *node = m_graph->getNode(nodeId);
          if (stmtData.speaker) {
            node->setProperty("character", *stmtData.speaker);
          }
          node->setProperty("text", stmtData.text);
          node->setSourceLocation(stmt.location);
          return nodeId;
        } else if constexpr (std::is_same_v<T, ChoiceStmt>) {
          NodeId choiceId = createNodeAndConnect(IRNodeType::Choice, prevNode);
          auto *choiceNode = m_graph->getNode(choiceId);
          choiceNode->setSourceLocation(stmt.location);

          std::vector<std::string> optionTexts;
          for (const auto &opt : stmtData.options) {
            optionTexts.push_back(opt.text);
          }
          choiceNode->setProperty("options", optionTexts);

          return choiceId;
        } else if constexpr (std::is_same_v<T, IfStmt>) {
          NodeId branchId = createNodeAndConnect(IRNodeType::Branch, prevNode);
          auto *branchNode = m_graph->getNode(branchId);
          branchNode->setSourceLocation(stmt.location);
          return branchId;
        } else if constexpr (std::is_same_v<T, GotoStmt>) {
          NodeId gotoId = createNodeAndConnect(IRNodeType::Goto, prevNode);
          auto *gotoNode = m_graph->getNode(gotoId);
          gotoNode->setProperty("target", stmtData.target);
          gotoNode->setSourceLocation(stmt.location);
          return gotoId;
        } else if constexpr (std::is_same_v<T, PlayStmt>) {
          IRNodeType nodeType = (stmtData.type == PlayStmt::MediaType::Music)
                                    ? IRNodeType::PlayMusic
                                    : IRNodeType::PlaySound;
          NodeId nodeId = createNodeAndConnect(nodeType, prevNode);
          auto *node = m_graph->getNode(nodeId);
          node->setProperty("track", stmtData.resource);
          if (stmtData.loop && *stmtData.loop) {
            node->setProperty("loop", true);
          }
          node->setSourceLocation(stmt.location);
          return nodeId;
        } else if constexpr (std::is_same_v<T, StopStmt>) {
          NodeId nodeId = createNodeAndConnect(IRNodeType::StopMusic, prevNode);
          m_graph->getNode(nodeId)->setSourceLocation(stmt.location);
          return nodeId;
        } else if constexpr (std::is_same_v<T, WaitStmt>) {
          NodeId nodeId = createNodeAndConnect(IRNodeType::Wait, prevNode);
          auto *node = m_graph->getNode(nodeId);
          node->setProperty("duration", static_cast<f64>(stmtData.duration));
          node->setSourceLocation(stmt.location);
          return nodeId;
        } else if constexpr (std::is_same_v<T, TransitionStmt>) {
          NodeId nodeId =
              createNodeAndConnect(IRNodeType::Transition, prevNode);
          auto *node = m_graph->getNode(nodeId);
          node->setProperty("type", stmtData.type);
          node->setProperty("duration", static_cast<f64>(stmtData.duration));
          node->setSourceLocation(stmt.location);
          return nodeId;
        } else {
          // For other statement types, return prevNode unchanged
          return prevNode;
        }
      },
      stmt.data);
}

NodeId ASTToIRConverter::convertExpression(const Expression & /*expr*/) {
  // Stub implementation - would create expression nodes
  return 0;
}

NodeId ASTToIRConverter::createNodeAndConnect(IRNodeType type,
                                              NodeId prevNode) {
  NodeId newId = m_graph->createNode(type);
  auto *newNode = m_graph->getNode(newId);

  auto *prev = m_graph->getNode(prevNode);
  if (prev) {
    newNode->setPosition(prev->getX(), prev->getY() + m_nodeSpacing);
  }

  PortId outPort{prevNode, "exec_out", true};
  PortId inPort{newId, "exec_in", false};
  m_graph->connect(outPort, inPort);

  return newId;
}

// ============================================================================
// IRToASTConverter Implementation
// ============================================================================

IRToASTConverter::IRToASTConverter() = default;
IRToASTConverter::~IRToASTConverter() = default;

Result<Program> IRToASTConverter::convert(const IRGraph &graph) {
  Program program;
  m_visited.clear();

  for (const auto &sceneName : graph.getSceneNames()) {
    NodeId startId = graph.getSceneStartNode(sceneName);
    if (startId == 0) {
      continue;
    }

    SceneDecl scene;
    scene.name = sceneName;

    auto execOrder = graph.getExecutionOrder();
    for (NodeId id : execOrder) {
      const auto *node = graph.getNode(id);
      if (!node || m_visited.count(id)) {
        continue;
      }

      auto stmt = convertNode(node, graph);
      if (stmt) {
        scene.body.push_back(std::move(stmt));
      }
    }

    program.scenes.push_back(std::move(scene));
  }

  return Result<Program>::ok(std::move(program));
}

std::unique_ptr<Statement>
IRToASTConverter::convertNode(const IRNode *node, const IRGraph & /*graph*/) {
  m_visited.insert(node->getId());

  switch (node->getType()) {
  case IRNodeType::ShowCharacter: {
    ShowStmt show;
    show.target = ShowStmt::Target::Character;
    show.identifier = node->getStringProperty("character");
    return std::make_unique<Statement>(std::move(show),
                                       node->getSourceLocation());
  }

  case IRNodeType::ShowBackground: {
    ShowStmt show;
    show.target = ShowStmt::Target::Background;
    show.identifier = node->getStringProperty("background");
    return std::make_unique<Statement>(std::move(show),
                                       node->getSourceLocation());
  }

  case IRNodeType::HideCharacter: {
    HideStmt hide;
    hide.identifier = node->getStringProperty("character");
    return std::make_unique<Statement>(std::move(hide),
                                       node->getSourceLocation());
  }

  case IRNodeType::Dialogue: {
    SayStmt say;
    std::string character = node->getStringProperty("character");
    if (!character.empty()) {
      say.speaker = character;
    }
    say.text = node->getStringProperty("text");
    return std::make_unique<Statement>(std::move(say),
                                       node->getSourceLocation());
  }

  case IRNodeType::PlayMusic: {
    PlayStmt play;
    play.type = PlayStmt::MediaType::Music;
    play.resource = node->getStringProperty("track");
    play.loop = node->getBoolProperty("loop", false);
    return std::make_unique<Statement>(std::move(play),
                                       node->getSourceLocation());
  }

  case IRNodeType::PlaySound: {
    PlayStmt play;
    play.type = PlayStmt::MediaType::Sound;
    play.resource = node->getStringProperty("track");
    return std::make_unique<Statement>(std::move(play),
                                       node->getSourceLocation());
  }

  case IRNodeType::Wait: {
    WaitStmt wait;
    wait.duration = static_cast<f32>(node->getFloatProperty("duration", 1.0));
    return std::make_unique<Statement>(std::move(wait),
                                       node->getSourceLocation());
  }

  case IRNodeType::Goto: {
    GotoStmt gotoStmt;
    gotoStmt.target = node->getStringProperty("target");
    return std::make_unique<Statement>(std::move(gotoStmt),
                                       node->getSourceLocation());
  }

  default:
    return nullptr;
  }
}

std::unique_ptr<Expression>
IRToASTConverter::convertToExpression(const IRNode * /*node*/,
                                      const IRGraph & /*graph*/) {
  // Stub implementation
  return nullptr;
}

// ============================================================================
// ASTToTextGenerator Implementation
// ============================================================================

ASTToTextGenerator::ASTToTextGenerator() = default;
ASTToTextGenerator::~ASTToTextGenerator() = default;

std::string ASTToTextGenerator::generate(const Program &program) {
  m_output.clear();
  m_indentLevel = 0;

  for (const auto &decl : program.characters) {
    generateCharacter(decl);
    newline();
  }

  if (!program.characters.empty()) {
    newline();
  }

  for (const auto &scene : program.scenes) {
    generateScene(scene);
    newline();
  }

  return m_output;
}

void ASTToTextGenerator::generateCharacter(const CharacterDecl &decl) {
  write("character ");
  write(decl.id);
  write("(name=\"");
  write(decl.displayName);
  write("\"");
  if (!decl.color.empty()) {
    write(", color=\"");
    write(decl.color);
    write("\"");
  }
  write(")");
}

void ASTToTextGenerator::generateScene(const SceneDecl &scene) {
  write("scene ");
  write(scene.name);
  write(" {");
  newline();

  m_indentLevel++;
  for (const auto &stmt : scene.body) {
    generateStatement(*stmt, m_indentLevel);
  }
  m_indentLevel--;

  indent();
  write("}");
}

void ASTToTextGenerator::generateStatement(const Statement &stmt,
                                           int /*indentLvl*/) {
  indent();

  std::visit(
      [this](const auto &stmtData) {
        using T = std::decay_t<decltype(stmtData)>;

        if constexpr (std::is_same_v<T, ShowStmt>) {
          if (stmtData.target == ShowStmt::Target::Background) {
            write("show background \"");
            if (stmtData.resource) {
              write(*stmtData.resource);
            } else {
              write(stmtData.identifier);
            }
            write("\"");
          } else {
            write("show ");
            write(stmtData.identifier);
          }
        } else if constexpr (std::is_same_v<T, HideStmt>) {
          write("hide ");
          write(stmtData.identifier);
        } else if constexpr (std::is_same_v<T, SayStmt>) {
          if (stmtData.speaker) {
            write("say ");
            write(*stmtData.speaker);
            write(" \"");
          } else {
            write("say \"");
          }
          write(stmtData.text);
          write("\"");
        } else if constexpr (std::is_same_v<T, GotoStmt>) {
          write("goto ");
          write(stmtData.target);
        } else if constexpr (std::is_same_v<T, PlayStmt>) {
          if (stmtData.type == PlayStmt::MediaType::Music) {
            write("play music \"");
          } else {
            write("play sound \"");
          }
          write(stmtData.resource);
          write("\"");
        } else if constexpr (std::is_same_v<T, StopStmt>) {
          write("stop music");
        } else if constexpr (std::is_same_v<T, WaitStmt>) {
          write("wait ");
          write(std::to_string(stmtData.duration));
        } else if constexpr (std::is_same_v<T, TransitionStmt>) {
          write("transition ");
          write(stmtData.type);
          write(" ");
          write(std::to_string(stmtData.duration));
        }
      },
      stmt.data);

  newline();
}

void ASTToTextGenerator::generateExpression(const Expression &expr) {
  std::visit(
      [this](const auto &exprData) {
        using T = std::decay_t<decltype(exprData)>;

        if constexpr (std::is_same_v<T, LiteralExpr>) {
          std::visit(
              [this](const auto &val) {
                using V = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<V, std::string>) {
                  write("\"");
                  write(val);
                  write("\"");
                } else if constexpr (std::is_same_v<V, i32>) {
                  write(std::to_string(val));
                } else if constexpr (std::is_same_v<V, f32>) {
                  write(std::to_string(val));
                } else if constexpr (std::is_same_v<V, bool>) {
                  write(val ? "true" : "false");
                }
              },
              exprData.value);
        } else if constexpr (std::is_same_v<T, IdentifierExpr>) {
          write(exprData.name);
        }
      },
      expr.data);
}

void ASTToTextGenerator::indent() {
  for (int i = 0; i < m_indentLevel; ++i) {
    m_output += "    ";
  }
}

void ASTToTextGenerator::newline() { m_output += "\n"; }

void ASTToTextGenerator::write(const std::string &text) { m_output += text; }

// ============================================================================
// VisualGraph Implementation
// ============================================================================

VisualGraph::VisualGraph() = default;
VisualGraph::~VisualGraph() = default;

void VisualGraph::fromIR(const IRGraph &ir) {
  m_nodes.clear();
  m_edges.clear();

  for (const auto *node : ir.getNodes()) {
    VisualGraphNode vnode;
    vnode.id = node->getId();
    vnode.type = node->getTypeName();
    vnode.displayName = node->getTypeName();
    vnode.x = node->getX();
    vnode.y = node->getY();

    for (const auto &port : node->getInputPorts()) {
      vnode.inputPorts.push_back({port.name, port.displayName});
    }
    for (const auto &port : node->getOutputPorts()) {
      vnode.outputPorts.push_back({port.name, port.displayName});
    }

    for (const auto &[name, value] : node->getProperties()) {
      std::string strValue;
      std::visit(
          [&strValue](const auto &v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
              strValue = "";
            } else if constexpr (std::is_same_v<T, bool>) {
              strValue = v ? "true" : "false";
            } else if constexpr (std::is_same_v<T, i64>) {
              strValue = std::to_string(v);
            } else if constexpr (std::is_same_v<T, f64>) {
              strValue = std::to_string(v);
            } else if constexpr (std::is_same_v<T, std::string>) {
              strValue = v;
            }
          },
          value);
      vnode.properties[name] = strValue;
    }

    m_nodes.push_back(vnode);
    m_nextId = std::max(m_nextId, vnode.id + 1);
  }

  for (const auto &conn : ir.getConnections()) {
    VisualGraphEdge edge;
    edge.sourceNode = conn.source.nodeId;
    edge.sourcePort = conn.source.portName;
    edge.targetNode = conn.target.nodeId;
    edge.targetPort = conn.target.portName;
    m_edges.push_back(edge);
  }
}

std::unique_ptr<IRGraph> VisualGraph::toIR() const {
  auto ir = std::make_unique<IRGraph>();

  static std::unordered_map<std::string, IRNodeType> typeMap = {
      {"SceneStart", IRNodeType::SceneStart},
      {"SceneEnd", IRNodeType::SceneEnd},
      {"Comment", IRNodeType::Comment},
      {"Sequence", IRNodeType::Sequence},
      {"Branch", IRNodeType::Branch},
      {"ShowCharacter", IRNodeType::ShowCharacter},
      {"HideCharacter", IRNodeType::HideCharacter},
      {"ShowBackground", IRNodeType::ShowBackground},
      {"Dialogue", IRNodeType::Dialogue},
      {"Choice", IRNodeType::Choice},
      {"PlayMusic", IRNodeType::PlayMusic},
      {"StopMusic", IRNodeType::StopMusic},
      {"PlaySound", IRNodeType::PlaySound},
      {"Transition", IRNodeType::Transition},
      {"Wait", IRNodeType::Wait},
      {"SetVariable", IRNodeType::SetVariable},
      {"GetVariable", IRNodeType::GetVariable},
      {"Goto", IRNodeType::Goto},
      {"Label", IRNodeType::Label}};

  std::unordered_map<NodeId, NodeId> idMap;
  for (const auto &vnode : m_nodes) {
    auto typeIt = typeMap.find(vnode.type);
    IRNodeType type =
        (typeIt != typeMap.end()) ? typeIt->second : IRNodeType::Custom;

    NodeId newId = ir->createNode(type);
    idMap[vnode.id] = newId;

    auto *node = ir->getNode(newId);
    node->setPosition(vnode.x, vnode.y);

    for (const auto &[name, value] : vnode.properties) {
      if (value == "true") {
        node->setProperty(name, true);
      } else if (value == "false") {
        node->setProperty(name, false);
      } else {
        try {
          size_t pos;
          i64 intVal = std::stoll(value, &pos);
          if (pos == value.length()) {
            node->setProperty(name, intVal);
            continue;
          }
        } catch (...) {
        }

        try {
          size_t pos;
          f64 floatVal = std::stod(value, &pos);
          if (pos == value.length()) {
            node->setProperty(name, floatVal);
            continue;
          }
        } catch (...) {
        }

        node->setProperty(name, value);
      }
    }
  }

  for (const auto &edge : m_edges) {
    auto sourceIt = idMap.find(edge.sourceNode);
    auto targetIt = idMap.find(edge.targetNode);

    if (sourceIt != idMap.end() && targetIt != idMap.end()) {
      PortId source{sourceIt->second, edge.sourcePort, true};
      PortId target{targetIt->second, edge.targetPort, false};
      ir->connect(source, target);
    }
  }

  return ir;
}

VisualGraphNode *VisualGraph::findNode(NodeId id) {
  for (auto &node : m_nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

const VisualGraphNode *VisualGraph::findNode(NodeId id) const {
  for (const auto &node : m_nodes) {
    if (node.id == id) {
      return &node;
    }
  }
  return nullptr;
}

NodeId VisualGraph::addNode(const std::string &type, f32 x, f32 y) {
  VisualGraphNode node;
  node.id = m_nextId++;
  node.type = type;
  node.displayName = type;
  node.x = x;
  node.y = y;
  m_nodes.push_back(node);
  return node.id;
}

void VisualGraph::removeNode(NodeId id) {
  m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(),
                               [id](const VisualGraphEdge &e) {
                                 return e.sourceNode == id ||
                                        e.targetNode == id;
                               }),
                m_edges.end());

  m_nodes.erase(
      std::remove_if(m_nodes.begin(), m_nodes.end(),
                     [id](const VisualGraphNode &n) { return n.id == id; }),
      m_nodes.end());
}

void VisualGraph::setNodePosition(NodeId id, f32 x, f32 y) {
  auto *node = findNode(id);
  if (node) {
    node->x = x;
    node->y = y;
  }
}

void VisualGraph::setNodeProperty(NodeId id, const std::string &name,
                                  const std::string &value) {
  auto *node = findNode(id);
  if (node) {
    node->properties[name] = value;
  }
}

void VisualGraph::addEdge(NodeId sourceNode, const std::string &sourcePort,
                          NodeId targetNode, const std::string &targetPort) {
  VisualGraphEdge edge;
  edge.sourceNode = sourceNode;
  edge.sourcePort = sourcePort;
  edge.targetNode = targetNode;
  edge.targetPort = targetPort;
  m_edges.push_back(edge);
}

void VisualGraph::removeEdge(NodeId sourceNode, const std::string &sourcePort,
                             NodeId targetNode, const std::string &targetPort) {
  m_edges.erase(std::remove_if(m_edges.begin(), m_edges.end(),
                               [&](const VisualGraphEdge &e) {
                                 return e.sourceNode == sourceNode &&
                                        e.sourcePort == sourcePort &&
                                        e.targetNode == targetNode &&
                                        e.targetPort == targetPort;
                               }),
                m_edges.end());
}

void VisualGraph::selectNode(NodeId id, bool addToSelection) {
  if (!addToSelection) {
    for (auto &node : m_nodes) {
      node.selected = false;
    }
  }

  auto *node = findNode(id);
  if (node) {
    node->selected = true;
  }
}

void VisualGraph::deselectNode(NodeId id) {
  auto *node = findNode(id);
  if (node) {
    node->selected = false;
  }
}

void VisualGraph::selectEdge(NodeId /*sourceNode*/,
                             const std::string & /*sourcePort*/,
                             NodeId /*targetNode*/,
                             const std::string & /*targetPort*/) {
  // Find and select the edge
}

void VisualGraph::clearSelection() {
  for (auto &node : m_nodes) {
    node.selected = false;
  }
  for (auto &edge : m_edges) {
    edge.selected = false;
  }
}

void VisualGraph::autoLayout() {
  f32 y = 100.0f;
  f32 spacing = 150.0f;

  for (auto &node : m_nodes) {
    node.x = 200.0f;
    node.y = y;
    y += spacing;
  }
}

std::string VisualGraph::toJson() const {
  std::stringstream ss;
  ss << "{\"nodes\":[";

  bool first = true;
  for (const auto &node : m_nodes) {
    if (!first)
      ss << ",";
    first = false;
    ss << "{\"id\":" << node.id;
    ss << ",\"type\":\"" << node.type << "\"";
    ss << ",\"x\":" << node.x;
    ss << ",\"y\":" << node.y << "}";
  }

  ss << "],\"edges\":[";

  first = true;
  for (const auto &edge : m_edges) {
    if (!first)
      ss << ",";
    first = false;
    ss << "{\"src\":" << edge.sourceNode;
    ss << ",\"srcPort\":\"" << edge.sourcePort << "\"";
    ss << ",\"tgt\":" << edge.targetNode;
    ss << ",\"tgtPort\":\"" << edge.targetPort << "\"}";
  }

  ss << "]}";
  return ss.str();
}

// ============================================================================
// RoundTripConverter Implementation
// ============================================================================

RoundTripConverter::RoundTripConverter()
    : m_lexer(std::make_unique<Lexer>()), m_parser(std::make_unique<Parser>()),
      m_astToIR(std::make_unique<ASTToIRConverter>()),
      m_irToAST(std::make_unique<IRToASTConverter>()),
      m_textGen(std::make_unique<ASTToTextGenerator>()) {}

RoundTripConverter::~RoundTripConverter() = default;

Result<std::unique_ptr<IRGraph>>
RoundTripConverter::textToIR(const std::string &nmScript) {
  // Tokenize the script first
  auto tokenResult = m_lexer->tokenize(nmScript);
  if (!tokenResult.isOk()) {
    return Result<std::unique_ptr<IRGraph>>::error("Lexer error: " +
                                                   tokenResult.error());
  }

  // Parse tokens to AST
  auto parseResult = m_parser->parse(tokenResult.value());
  if (!parseResult.isOk()) {
    return Result<std::unique_ptr<IRGraph>>::error("Parse error: " +
                                                   parseResult.error());
  }

  // Convert AST to IR
  return m_astToIR->convert(parseResult.value());
}

Result<std::string> RoundTripConverter::irToText(const IRGraph &ir) {
  auto astResult = m_irToAST->convert(ir);
  if (!astResult.isOk()) {
    return Result<std::string>::error("IR to AST conversion failed");
  }

  return Result<std::string>::ok(m_textGen->generate(astResult.value()));
}

Result<std::unique_ptr<VisualGraph>>
RoundTripConverter::irToVisualGraph(const IRGraph &ir) {
  auto graph = std::make_unique<VisualGraph>();
  graph->fromIR(ir);
  return Result<std::unique_ptr<VisualGraph>>::ok(std::move(graph));
}

Result<std::unique_ptr<IRGraph>>
RoundTripConverter::visualGraphToIR(const VisualGraph &graph) {
  return Result<std::unique_ptr<IRGraph>>::ok(graph.toIR());
}

Result<std::unique_ptr<VisualGraph>>
RoundTripConverter::textToVisualGraph(const std::string &nmScript) {
  auto irResult = textToIR(nmScript);
  if (!irResult.isOk()) {
    return Result<std::unique_ptr<VisualGraph>>::error(irResult.error());
  }

  return irToVisualGraph(*irResult.value());
}

Result<std::string>
RoundTripConverter::visualGraphToText(const VisualGraph &graph) {
  auto irResult = visualGraphToIR(graph);
  if (!irResult.isOk()) {
    return Result<std::string>::error(irResult.error());
  }

  return irToText(*irResult.value());
}

std::vector<std::string>
RoundTripConverter::validateConversion(const std::string & /*original*/,
                                       const std::string & /*roundTripped*/) {
  std::vector<std::string> differences;
  // Would compare original and round-tripped versions
  return differences;
}

// ============================================================================
// GraphDiffer Implementation
// ============================================================================

GraphDiff GraphDiffer::diff(const VisualGraph &oldGraph,
                            const VisualGraph &newGraph) const {
  GraphDiff result;

  diffNodes(oldGraph, newGraph, result);
  diffEdges(oldGraph, newGraph, result);

  return result;
}

void GraphDiffer::diffNodes(const VisualGraph &oldGraph,
                            const VisualGraph &newGraph,
                            GraphDiff &result) const {
  const auto &oldNodes = oldGraph.getNodes();
  const auto &newNodes = newGraph.getNodes();

  // Build ID sets
  std::unordered_set<NodeId> oldIds;
  std::unordered_set<NodeId> newIds;

  for (const auto &node : oldNodes) {
    oldIds.insert(node.id);
  }
  for (const auto &node : newNodes) {
    newIds.insert(node.id);
  }

  // Find removed nodes
  for (const auto &node : oldNodes) {
    if (newIds.find(node.id) == newIds.end()) {
      GraphDiffEntry entry;
      entry.type = GraphDiffType::NodeRemoved;
      entry.nodeId = node.id;
      entry.oldValue = node.type;
      result.entries.push_back(entry);
      result.hasStructuralChanges = true;
    }
  }

  // Find added and modified nodes
  for (const auto &newNode : newNodes) {
    if (oldIds.find(newNode.id) == oldIds.end()) {
      GraphDiffEntry entry;
      entry.type = GraphDiffType::NodeAdded;
      entry.nodeId = newNode.id;
      entry.newValue = newNode.type;
      result.entries.push_back(entry);
      result.hasStructuralChanges = true;
    } else {
      // Node exists in both - check for modifications
      const auto *oldNode = oldGraph.findNode(newNode.id);
      if (oldNode) {
        diffNodeProperties(*oldNode, newNode, result);

        // Check position changes
        if (oldNode->x != newNode.x || oldNode->y != newNode.y) {
          GraphDiffEntry entry;
          entry.type = GraphDiffType::PositionChanged;
          entry.nodeId = newNode.id;
          entry.oldValue =
              std::to_string(oldNode->x) + "," + std::to_string(oldNode->y);
          entry.newValue =
              std::to_string(newNode.x) + "," + std::to_string(newNode.y);
          result.entries.push_back(entry);
          result.hasPositionChanges = true;
        }
      }
    }
  }
}

void GraphDiffer::diffEdges(const VisualGraph &oldGraph,
                            const VisualGraph &newGraph,
                            GraphDiff &result) const {
  const auto &oldEdges = oldGraph.getEdges();
  const auto &newEdges = newGraph.getEdges();

  // Helper to compare edges
  auto edgeEqual = [](const VisualGraphEdge &a, const VisualGraphEdge &b) {
    return a.sourceNode == b.sourceNode && a.sourcePort == b.sourcePort &&
           a.targetNode == b.targetNode && a.targetPort == b.targetPort;
  };

  // Find removed edges
  for (const auto &oldEdge : oldEdges) {
    bool found = false;
    for (const auto &newEdge : newEdges) {
      if (edgeEqual(oldEdge, newEdge)) {
        found = true;
        break;
      }
    }
    if (!found) {
      GraphDiffEntry entry;
      entry.type = GraphDiffType::EdgeRemoved;
      entry.edge = oldEdge;
      result.entries.push_back(entry);
      result.hasStructuralChanges = true;
    }
  }

  // Find added edges
  for (const auto &newEdge : newEdges) {
    bool found = false;
    for (const auto &oldEdge : oldEdges) {
      if (edgeEqual(oldEdge, newEdge)) {
        found = true;
        break;
      }
    }
    if (!found) {
      GraphDiffEntry entry;
      entry.type = GraphDiffType::EdgeAdded;
      entry.edge = newEdge;
      result.entries.push_back(entry);
      result.hasStructuralChanges = true;
    }
  }
}

void GraphDiffer::diffNodeProperties(const VisualGraphNode &oldNode,
                                     const VisualGraphNode &newNode,
                                     GraphDiff &result) const {
  // Check type change (rare but possible)
  if (oldNode.type != newNode.type) {
    GraphDiffEntry entry;
    entry.type = GraphDiffType::NodeModified;
    entry.nodeId = oldNode.id;
    entry.propertyName = "type";
    entry.oldValue = oldNode.type;
    entry.newValue = newNode.type;
    result.entries.push_back(entry);
    result.hasPropertyChanges = true;
  }

  // Check display name
  if (oldNode.displayName != newNode.displayName) {
    GraphDiffEntry entry;
    entry.type = GraphDiffType::PropertyChanged;
    entry.nodeId = oldNode.id;
    entry.propertyName = "displayName";
    entry.oldValue = oldNode.displayName;
    entry.newValue = newNode.displayName;
    result.entries.push_back(entry);
    result.hasPropertyChanges = true;
  }

  // Check properties
  std::unordered_set<std::string> allProps;
  for (const auto &[name, value] : oldNode.properties) {
    allProps.insert(name);
  }
  for (const auto &[name, value] : newNode.properties) {
    allProps.insert(name);
  }

  for (const auto &propName : allProps) {
    auto oldIt = oldNode.properties.find(propName);
    auto newIt = newNode.properties.find(propName);

    std::string oldVal =
        (oldIt != oldNode.properties.end()) ? oldIt->second : "";
    std::string newVal =
        (newIt != newNode.properties.end()) ? newIt->second : "";

    if (oldVal != newVal) {
      GraphDiffEntry entry;
      entry.type = GraphDiffType::PropertyChanged;
      entry.nodeId = oldNode.id;
      entry.propertyName = propName;
      entry.oldValue = oldVal;
      entry.newValue = newVal;
      result.entries.push_back(entry);
      result.hasPropertyChanges = true;
    }
  }
}

Result<void> GraphDiffer::applyDiff(VisualGraph &graph,
                                    const GraphDiff &diff) const {
  for (const auto &entry : diff.entries) {
    switch (entry.type) {
    case GraphDiffType::NodeAdded: {
      graph.addNode(entry.newValue, 0.0f, 0.0f);
      break;
    }
    case GraphDiffType::NodeRemoved: {
      graph.removeNode(entry.nodeId);
      break;
    }
    case GraphDiffType::PropertyChanged: {
      graph.setNodeProperty(entry.nodeId, entry.propertyName, entry.newValue);
      break;
    }
    case GraphDiffType::PositionChanged: {
      std::istringstream iss(entry.newValue);
      f32 x, y;
      char comma;
      if (iss >> x >> comma >> y) {
        graph.setNodePosition(entry.nodeId, x, y);
      }
      break;
    }
    case GraphDiffType::EdgeAdded: {
      graph.addEdge(entry.edge.sourceNode, entry.edge.sourcePort,
                    entry.edge.targetNode, entry.edge.targetPort);
      break;
    }
    case GraphDiffType::EdgeRemoved: {
      graph.removeEdge(entry.edge.sourceNode, entry.edge.sourcePort,
                       entry.edge.targetNode, entry.edge.targetPort);
      break;
    }
    default:
      break;
    }
  }

  return Result<void>::ok();
}

GraphDiff GraphDiffer::invertDiff(const GraphDiff &diff) const {
  GraphDiff inverted;
  inverted.hasStructuralChanges = diff.hasStructuralChanges;
  inverted.hasPropertyChanges = diff.hasPropertyChanges;
  inverted.hasPositionChanges = diff.hasPositionChanges;

  for (auto it = diff.entries.rbegin(); it != diff.entries.rend(); ++it) {
    GraphDiffEntry entry = *it;

    switch (entry.type) {
    case GraphDiffType::NodeAdded:
      entry.type = GraphDiffType::NodeRemoved;
      std::swap(entry.oldValue, entry.newValue);
      break;
    case GraphDiffType::NodeRemoved:
      entry.type = GraphDiffType::NodeAdded;
      std::swap(entry.oldValue, entry.newValue);
      break;
    case GraphDiffType::EdgeAdded:
      entry.type = GraphDiffType::EdgeRemoved;
      break;
    case GraphDiffType::EdgeRemoved:
      entry.type = GraphDiffType::EdgeAdded;
      break;
    case GraphDiffType::PropertyChanged:
    case GraphDiffType::PositionChanged:
    case GraphDiffType::NodeModified:
      std::swap(entry.oldValue, entry.newValue);
      break;
    }

    inverted.entries.push_back(entry);
  }

  return inverted;
}

Result<GraphDiff> GraphDiffer::mergeDiffs(const GraphDiff &diff1,
                                          const GraphDiff &diff2) const {
  if (hasConflicts(diff1, diff2)) {
    return Result<GraphDiff>::error(
        "Diffs have conflicts and cannot be merged");
  }

  GraphDiff merged;
  merged.entries = diff1.entries;
  merged.entries.insert(merged.entries.end(), diff2.entries.begin(),
                        diff2.entries.end());
  merged.hasStructuralChanges =
      diff1.hasStructuralChanges || diff2.hasStructuralChanges;
  merged.hasPropertyChanges =
      diff1.hasPropertyChanges || diff2.hasPropertyChanges;
  merged.hasPositionChanges =
      diff1.hasPositionChanges || diff2.hasPositionChanges;

  return Result<GraphDiff>::ok(std::move(merged));
}

bool GraphDiffer::hasConflicts(const GraphDiff &diff1,
                               const GraphDiff &diff2) const {
  for (const auto &e1 : diff1.entries) {
    if (e1.type == GraphDiffType::PropertyChanged ||
        e1.type == GraphDiffType::PositionChanged) {
      for (const auto &e2 : diff2.entries) {
        if ((e2.type == GraphDiffType::PropertyChanged ||
             e2.type == GraphDiffType::PositionChanged) &&
            e1.nodeId == e2.nodeId && e1.propertyName == e2.propertyName &&
            e1.newValue != e2.newValue) {
          return true;
        }
      }
    }
  }

  return false;
}

// ============================================================================
// IDNormalizer Implementation
// ============================================================================

std::unordered_map<NodeId, NodeId>
IDNormalizer::normalize(VisualGraph &graph) const {
  std::unordered_map<NodeId, NodeId> mapping;

  auto order = getTopologicalOrder(graph);

  if (order.empty()) {
    const auto &nodes = graph.getNodes();
    for (const auto &node : nodes) {
      order.push_back(node.id);
    }
    std::sort(order.begin(), order.end());
  }

  NodeId newId = 1;
  for (NodeId oldId : order) {
    mapping[oldId] = newId++;
  }

  return mapping;
}

std::unordered_map<NodeId, NodeId>
IDNormalizer::normalize(IRGraph & /*graph*/) const {
  std::unordered_map<NodeId, NodeId> mapping;
  return mapping;
}

bool IDNormalizer::needsNormalization(const VisualGraph &graph) const {
  const auto &nodes = graph.getNodes();
  if (nodes.empty()) {
    return false;
  }

  std::vector<NodeId> ids;
  for (const auto &node : nodes) {
    ids.push_back(node.id);
  }
  std::sort(ids.begin(), ids.end());

  for (size_t i = 0; i < ids.size(); ++i) {
    if (ids[i] != static_cast<NodeId>(i + 1)) {
      return true;
    }
  }

  return false;
}

std::pair<std::unique_ptr<VisualGraph>, std::unordered_map<NodeId, NodeId>>
IDNormalizer::createNormalizedCopy(const VisualGraph &graph) const {
  auto normalized = std::make_unique<VisualGraph>();
  std::unordered_map<NodeId, NodeId> mapping;

  auto order = getTopologicalOrder(graph);

  if (order.empty()) {
    const auto &nodes = graph.getNodes();
    for (const auto &node : nodes) {
      order.push_back(node.id);
    }
    std::sort(order.begin(), order.end());
  }

  for (NodeId oldId : order) {
    const auto *oldNode = graph.findNode(oldId);
    if (oldNode) {
      NodeId assignedId =
          normalized->addNode(oldNode->type, oldNode->x, oldNode->y);
      mapping[oldId] = assignedId;

      auto *newNode = normalized->findNode(assignedId);
      if (newNode) {
        newNode->displayName = oldNode->displayName;
        newNode->properties = oldNode->properties;
        newNode->width = oldNode->width;
        newNode->height = oldNode->height;
        newNode->inputPorts = oldNode->inputPorts;
        newNode->outputPorts = oldNode->outputPorts;
      }
    }
  }

  for (const auto &edge : graph.getEdges()) {
    auto srcIt = mapping.find(edge.sourceNode);
    auto tgtIt = mapping.find(edge.targetNode);

    if (srcIt != mapping.end() && tgtIt != mapping.end()) {
      normalized->addEdge(srcIt->second, edge.sourcePort, tgtIt->second,
                          edge.targetPort);
    }
  }

  return {std::move(normalized), mapping};
}

std::vector<NodeId>
IDNormalizer::getTopologicalOrder(const VisualGraph &graph) const {
  std::vector<NodeId> result;
  const auto &nodes = graph.getNodes();
  const auto &edges = graph.getEdges();

  std::unordered_map<NodeId, int> inDegree;
  std::unordered_map<NodeId, std::vector<NodeId>> adjacency;

  for (const auto &node : nodes) {
    inDegree[node.id] = 0;
    adjacency[node.id] = {};
  }

  for (const auto &edge : edges) {
    inDegree[edge.targetNode]++;
    adjacency[edge.sourceNode].push_back(edge.targetNode);
  }

  std::queue<NodeId> queue;
  for (const auto &[id, degree] : inDegree) {
    if (degree == 0) {
      queue.push(id);
    }
  }

  while (!queue.empty()) {
    NodeId id = queue.front();
    queue.pop();
    result.push_back(id);

    for (NodeId neighbor : adjacency[id]) {
      inDegree[neighbor]--;
      if (inDegree[neighbor] == 0) {
        queue.push(neighbor);
      }
    }
  }

  return result;
}

// ============================================================================
// RoundTripValidator Implementation
// ============================================================================

RoundTripValidator::RoundTripValidator()
    : m_converter(std::make_unique<RoundTripConverter>()),
      m_differ(std::make_unique<GraphDiffer>()),
      m_normalizer(std::make_unique<IDNormalizer>()) {}

RoundTripValidator::~RoundTripValidator() = default;

RoundTripValidator::ValidationResult
RoundTripValidator::validateTextRoundTrip(const std::string &nmScript) {
  ValidationResult result;
  result.originalText = nmScript;

  auto irResult = m_converter->textToIR(nmScript);
  if (!irResult.isOk()) {
    result.differences.push_back("Failed to convert text to IR: " +
                                 irResult.error());
    return result;
  }

  auto textResult = m_converter->irToText(*irResult.value());
  if (!textResult.isOk()) {
    result.differences.push_back("Failed to convert IR back to text: " +
                                 textResult.error());
    return result;
  }

  result.roundTrippedText = textResult.value();

  if (result.originalText != result.roundTrippedText) {
    result.differences.push_back("Text differs after round-trip");
  } else {
    result.isValid = true;
  }

  return result;
}

RoundTripValidator::ValidationResult
RoundTripValidator::validateIRRoundTrip(const IRGraph &ir) {
  ValidationResult result;

  auto vgResult = m_converter->irToVisualGraph(ir);
  if (!vgResult.isOk()) {
    result.differences.push_back("Failed to convert IR to VisualGraph");
    return result;
  }

  auto ir2Result = m_converter->visualGraphToIR(*vgResult.value());
  if (!ir2Result.isOk()) {
    result.differences.push_back("Failed to convert VisualGraph back to IR");
    return result;
  }

  if (areSemanticalllyEquivalent(ir, *ir2Result.value())) {
    result.isValid = true;
  } else {
    result.differences.push_back(
        "IR differs after round-trip through VisualGraph");
  }

  return result;
}

RoundTripValidator::ValidationResult
RoundTripValidator::validateFullRoundTrip(const std::string &nmScript) {
  ValidationResult result;
  result.originalText = nmScript;

  auto ir1Result = m_converter->textToIR(nmScript);
  if (!ir1Result.isOk()) {
    result.differences.push_back("Failed to convert text to IR: " +
                                 ir1Result.error());
    return result;
  }

  auto vgResult = m_converter->irToVisualGraph(*ir1Result.value());
  if (!vgResult.isOk()) {
    result.differences.push_back("Failed to convert IR to VisualGraph");
    return result;
  }

  auto ir2Result = m_converter->visualGraphToIR(*vgResult.value());
  if (!ir2Result.isOk()) {
    result.differences.push_back("Failed to convert VisualGraph to IR");
    return result;
  }

  auto textResult = m_converter->irToText(*ir2Result.value());
  if (!textResult.isOk()) {
    result.differences.push_back("Failed to convert IR to text");
    return result;
  }

  result.roundTrippedText = textResult.value();

  if (result.originalText == result.roundTrippedText) {
    result.isValid = true;
  } else {
    result.differences.push_back("Text differs after full round-trip");
  }

  return result;
}

bool RoundTripValidator::areSemanticalllyEquivalent(const IRGraph &a,
                                                    const IRGraph &b) const {
  auto nodesA = a.getNodes();
  auto nodesB = b.getNodes();

  if (nodesA.size() != nodesB.size()) {
    return false;
  }

  if (a.getConnections().size() != b.getConnections().size()) {
    return false;
  }

  std::unordered_map<std::string, int> typeCountA;
  std::unordered_map<std::string, int> typeCountB;

  for (const auto *node : nodesA) {
    typeCountA[node->getTypeName()]++;
  }
  for (const auto *node : nodesB) {
    typeCountB[node->getTypeName()]++;
  }

  return typeCountA == typeCountB;
}

} // namespace NovelMind::scripting
