#include "NovelMind/editor/qt/nm_undo_manager.hpp"
#include "NovelMind/core/logger.hpp"

#include <QDateTime>

namespace NovelMind::editor::qt {

// =============================================================================
// NMUndoManager Implementation
// =============================================================================

NMUndoManager::NMUndoManager() : QObject(nullptr) {}

NMUndoManager::~NMUndoManager() { shutdown(); }

NMUndoManager &NMUndoManager::instance() {
  static NMUndoManager instance;
  return instance;
}

void NMUndoManager::initialize() {
  if (m_initialized)
    return;

  m_undoStack = std::make_unique<QUndoStack>();

  // Default undo limit (0 = unlimited)
  m_undoStack->setUndoLimit(100);

  // Connect signals
  connect(m_undoStack.get(), &QUndoStack::canUndoChanged, this,
          &NMUndoManager::canUndoChanged);
  connect(m_undoStack.get(), &QUndoStack::canRedoChanged, this,
          &NMUndoManager::canRedoChanged);
  connect(m_undoStack.get(), &QUndoStack::undoTextChanged, this,
          &NMUndoManager::undoTextChanged);
  connect(m_undoStack.get(), &QUndoStack::redoTextChanged, this,
          &NMUndoManager::redoTextChanged);
  connect(m_undoStack.get(), &QUndoStack::cleanChanged, this,
          &NMUndoManager::cleanChanged);
  connect(m_undoStack.get(), &QUndoStack::indexChanged, this,
          &NMUndoManager::indexChanged);

  m_initialized = true;
  core::Logger::instance().info("Undo/Redo system initialized");
}

void NMUndoManager::shutdown() {
  if (!m_initialized)
    return;

  m_undoStack.reset();
  m_initialized = false;
}

void NMUndoManager::pushCommand(QUndoCommand *command) {
  if (!m_initialized || !m_undoStack) {
    core::Logger::instance().error("Undo manager not initialized");
    delete command;
    return;
  }

  m_undoStack->push(command);
}

bool NMUndoManager::canUndo() const {
  return m_undoStack && m_undoStack->canUndo();
}

bool NMUndoManager::canRedo() const {
  return m_undoStack && m_undoStack->canRedo();
}

QString NMUndoManager::undoText() const {
  return m_undoStack ? m_undoStack->undoText() : QString();
}

QString NMUndoManager::redoText() const {
  return m_undoStack ? m_undoStack->redoText() : QString();
}

void NMUndoManager::beginMacro(const QString &text) {
  if (m_undoStack) {
    m_undoStack->beginMacro(text);
  }
}

void NMUndoManager::endMacro() {
  if (m_undoStack) {
    m_undoStack->endMacro();
  }
}

void NMUndoManager::clear() {
  if (m_undoStack) {
    m_undoStack->clear();
  }
}

void NMUndoManager::setClean() {
  if (m_undoStack) {
    m_undoStack->setClean();
  }
}

bool NMUndoManager::isClean() const {
  return m_undoStack && m_undoStack->isClean();
}

void NMUndoManager::setUndoLimit(int limit) {
  if (m_undoStack) {
    m_undoStack->setUndoLimit(limit);
  }
}

void NMUndoManager::undo() {
  if (m_undoStack && m_undoStack->canUndo()) {
    m_undoStack->undo();
    core::Logger::instance().info("Undo: " +
                                  m_undoStack->undoText().toStdString());
  }
}

void NMUndoManager::redo() {
  if (m_undoStack && m_undoStack->canRedo()) {
    m_undoStack->redo();
    core::Logger::instance().info("Redo: " +
                                  m_undoStack->redoText().toStdString());
  }
}

// =============================================================================
// PropertyChangeCommand Implementation
// =============================================================================

PropertyChangeCommand::PropertyChangeCommand(const QString &objectName,
                                             const QString &propertyName,
                                             const QVariant &oldValue,
                                             const QVariant &newValue,
                                             QUndoCommand *parent)
    : QUndoCommand(parent), m_objectName(objectName),
      m_propertyName(propertyName), m_oldValue(oldValue), m_newValue(newValue) {
  setText(QString("Change %1.%2").arg(objectName, propertyName));
}

void PropertyChangeCommand::undo() {
  // TODO: Implement property change undo by interfacing with the property
  // system
  core::Logger::instance().info("Undo property change: " +
                                text().toStdString());
}

void PropertyChangeCommand::redo() {
  // TODO: Implement property change redo
  core::Logger::instance().info("Redo property change: " +
                                text().toStdString());
}

// =============================================================================
// AddObjectCommand Implementation
// =============================================================================

AddObjectCommand::AddObjectCommand(const QString &objectType,
                                   const QString &objectName,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), m_objectType(objectType), m_objectName(objectName) {
  setText(QString("Add %1").arg(objectName));
}

void AddObjectCommand::undo() {
  // TODO: Remove the object from the scene
  core::Logger::instance().info("Undo add object: " + text().toStdString());
}

void AddObjectCommand::redo() {
  // TODO: Add the object to the scene
  core::Logger::instance().info("Redo add object: " + text().toStdString());
}

// =============================================================================
// DeleteObjectCommand Implementation
// =============================================================================

DeleteObjectCommand::DeleteObjectCommand(const QString &objectName,
                                         QUndoCommand *parent)
    : QUndoCommand(parent), m_objectName(objectName) {
  setText(QString("Delete %1").arg(objectName));
}

void DeleteObjectCommand::undo() {
  // TODO: Restore the object to the scene
  core::Logger::instance().info("Undo delete object: " + text().toStdString());
}

void DeleteObjectCommand::redo() {
  // TODO: Delete the object from the scene
  core::Logger::instance().info("Redo delete object: " + text().toStdString());
}

// =============================================================================
// TransformObjectCommand Implementation
// =============================================================================

TransformObjectCommand::TransformObjectCommand(const QString &objectName,
                                               const QPointF &oldPosition,
                                               const QPointF &newPosition,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), m_objectName(objectName),
      m_oldPosition(oldPosition), m_newPosition(newPosition) {
  setText(QString("Move %1").arg(objectName));
}

void TransformObjectCommand::undo() {
  // TODO: Set object position to old position
  core::Logger::instance().info("Undo transform: " + text().toStdString());
}

void TransformObjectCommand::redo() {
  // TODO: Set object position to new position
  core::Logger::instance().info("Redo transform: " + text().toStdString());
}

bool TransformObjectCommand::mergeWith(const QUndoCommand *other) {
  // Merge consecutive transform commands on the same object
  if (other->id() != id())
    return false;

  const TransformObjectCommand *transformCommand =
      static_cast<const TransformObjectCommand *>(other);

  if (transformCommand->m_objectName != m_objectName)
    return false;

  m_newPosition = transformCommand->m_newPosition;
  return true;
}

// =============================================================================
// CreateGraphNodeCommand Implementation
// =============================================================================

CreateGraphNodeCommand::CreateGraphNodeCommand(const QString &nodeType,
                                               const QPointF &position,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), m_nodeType(nodeType), m_position(position) {
  setText(QString("Create %1 Node").arg(nodeType));
}

void CreateGraphNodeCommand::undo() {
  // TODO: Remove the node from the graph
  core::Logger::instance().info("Undo create node: " + text().toStdString());
}

void CreateGraphNodeCommand::redo() {
  // TODO: Create the node in the graph
  if (m_nodeId.isEmpty()) {
    // Generate unique ID on first redo
    m_nodeId = QString("node_%1").arg(QDateTime::currentMSecsSinceEpoch());
  }
  core::Logger::instance().info("Redo create node: " + text().toStdString());
}

// =============================================================================
// DeleteGraphNodeCommand Implementation
// =============================================================================

DeleteGraphNodeCommand::DeleteGraphNodeCommand(const QString &nodeId,
                                               QUndoCommand *parent)
    : QUndoCommand(parent), m_nodeId(nodeId) {
  setText(QString("Delete Node %1").arg(nodeId));
}

void DeleteGraphNodeCommand::undo() {
  // TODO: Restore the node to the graph
  core::Logger::instance().info("Undo delete node: " + text().toStdString());
}

void DeleteGraphNodeCommand::redo() {
  // TODO: Delete the node from the graph
  core::Logger::instance().info("Redo delete node: " + text().toStdString());
}

// =============================================================================
// ConnectGraphNodesCommand Implementation
// =============================================================================

ConnectGraphNodesCommand::ConnectGraphNodesCommand(const QString &sourceNodeId,
                                                   const QString &targetNodeId,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent), m_sourceNodeId(sourceNodeId),
      m_targetNodeId(targetNodeId) {
  setText(QString("Connect Nodes"));
}

void ConnectGraphNodesCommand::undo() {
  // TODO: Remove the connection
  core::Logger::instance().info("Undo connect nodes: " + text().toStdString());
}

void ConnectGraphNodesCommand::redo() {
  // TODO: Create the connection
  if (m_connectionId.isEmpty()) {
    // Generate unique ID on first redo
    m_connectionId =
        QString("conn_%1").arg(QDateTime::currentMSecsSinceEpoch());
  }
  core::Logger::instance().info("Redo connect nodes: " + text().toStdString());
}

} // namespace NovelMind::editor::qt
