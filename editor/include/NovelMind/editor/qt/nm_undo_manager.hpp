#pragma once

/**
 * @file nm_undo_manager.hpp
 * @brief Centralized undo/redo management using Qt's QUndoStack
 *
 * This manager provides:
 * - Global undo/redo stack for all editor operations
 * - Command pattern implementation for reversible actions
 * - Integration with Qt's undo framework
 * - Undo history visualization
 */

#include <QObject>
#include <QPointF>
#include <QUndoCommand>
#include <QUndoStack>
#include <QVariant>
#include <memory>

namespace NovelMind::editor::qt {

/**
 * @brief Centralized undo/redo manager (singleton)
 *
 * Manages all undoable operations in the editor using Qt's QUndoStack.
 * All modifications to the scene, story graph, properties, etc. should
 * go through this system to ensure undo/redo support.
 */
class NMUndoManager : public QObject {
  Q_OBJECT

public:
  /**
   * @brief Get the singleton instance
   */
  static NMUndoManager &instance();

  /**
   * @brief Initialize the undo manager
   */
  void initialize();

  /**
   * @brief Shutdown the undo manager
   */
  void shutdown();

  /**
   * @brief Get the underlying QUndoStack
   */
  [[nodiscard]] QUndoStack *undoStack() const { return m_undoStack.get(); }

  /**
   * @brief Push a command onto the undo stack
   * @param command The command to push (takes ownership)
   */
  void pushCommand(QUndoCommand *command);

  /**
   * @brief Check if undo is available
   */
  [[nodiscard]] bool canUndo() const;

  /**
   * @brief Check if redo is available
   */
  [[nodiscard]] bool canRedo() const;

  /**
   * @brief Get the text for the next undo action
   */
  [[nodiscard]] QString undoText() const;

  /**
   * @brief Get the text for the next redo action
   */
  [[nodiscard]] QString redoText() const;

  /**
   * @brief Begin a macro (group of commands)
   * @param text Description of the macro
   */
  void beginMacro(const QString &text);

  /**
   * @brief End the current macro
   */
  void endMacro();

  /**
   * @brief Clear the undo stack
   */
  void clear();

  /**
   * @brief Set the clean state (typically after save)
   */
  void setClean();

  /**
   * @brief Check if the document is clean (no unsaved changes)
   */
  [[nodiscard]] bool isClean() const;

  /**
   * @brief Set the undo limit (0 = unlimited)
   */
  void setUndoLimit(int limit);

public slots:
  /**
   * @brief Perform undo
   */
  void undo();

  /**
   * @brief Perform redo
   */
  void redo();

signals:
  /**
   * @brief Emitted when undo/redo availability changes
   */
  void canUndoChanged(bool canUndo);
  void canRedoChanged(bool canRedo);

  /**
   * @brief Emitted when undo/redo text changes
   */
  void undoTextChanged(const QString &text);
  void redoTextChanged(const QString &text);

  /**
   * @brief Emitted when clean state changes
   */
  void cleanChanged(bool clean);

  /**
   * @brief Emitted when an index changes in the stack
   */
  void indexChanged(int index);

private:
  NMUndoManager();
  ~NMUndoManager() override;

  NMUndoManager(const NMUndoManager &) = delete;
  NMUndoManager &operator=(const NMUndoManager &) = delete;

  std::unique_ptr<QUndoStack> m_undoStack;
  bool m_initialized = false;
};

// =============================================================================
// Common Command Types
// =============================================================================

/**
 * @brief Command for changing a property value
 */
class PropertyChangeCommand : public QUndoCommand {
public:
  PropertyChangeCommand(const QString &objectName, const QString &propertyName,
                        const QVariant &oldValue, const QVariant &newValue,
                        QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_objectName;
  QString m_propertyName;
  QVariant m_oldValue;
  QVariant m_newValue;
};

/**
 * @brief Command for adding an object to the scene
 */
class AddObjectCommand : public QUndoCommand {
public:
  AddObjectCommand(const QString &objectType, const QString &objectName,
                   QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_objectType;
  QString m_objectName;
  // TODO: Store object data for recreation
};

/**
 * @brief Command for deleting an object from the scene
 */
class DeleteObjectCommand : public QUndoCommand {
public:
  DeleteObjectCommand(const QString &objectName,
                      QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_objectName;
  // TODO: Store object data for restoration
};

/**
 * @brief Command for moving/transforming an object
 */
class TransformObjectCommand : public QUndoCommand {
public:
  TransformObjectCommand(const QString &objectName, const QPointF &oldPosition,
                         const QPointF &newPosition,
                         QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;
  bool mergeWith(const QUndoCommand *other) override;
  int id() const override { return 1; } // For command merging

private:
  QString m_objectName;
  QPointF m_oldPosition;
  QPointF m_newPosition;
};

/**
 * @brief Command for creating a graph node
 */
class CreateGraphNodeCommand : public QUndoCommand {
public:
  CreateGraphNodeCommand(const QString &nodeType, const QPointF &position,
                         QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_nodeType;
  QPointF m_position;
  QString m_nodeId; // Generated on first redo
};

/**
 * @brief Command for deleting a graph node
 */
class DeleteGraphNodeCommand : public QUndoCommand {
public:
  DeleteGraphNodeCommand(const QString &nodeId, QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_nodeId;
  // TODO: Store node data for restoration
};

/**
 * @brief Command for connecting two graph nodes
 */
class ConnectGraphNodesCommand : public QUndoCommand {
public:
  ConnectGraphNodesCommand(const QString &sourceNodeId,
                           const QString &targetNodeId,
                           QUndoCommand *parent = nullptr);

  void undo() override;
  void redo() override;

private:
  QString m_sourceNodeId;
  QString m_targetNodeId;
  QString m_connectionId; // Generated on first redo
};

} // namespace NovelMind::editor::qt
