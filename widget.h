#ifndef WIDGET_H
#define WIDGET_H

#include <QTreeWidget>
#include <QWidget>

#include "managersql.h"

class Widget : public QWidget
{
  Q_OBJECT

public:
  Widget(QWidget* theParent = nullptr);
  ~Widget();

public:
  //! Gets items from tree widget.
  //! @param theTreeWidget the tree widget.
  //! @param theList list to write the result.
  static QList<QTreeWidgetItem*> GetItems(QTreeWidget* theTreeWidget);

public:

  //! Adds sub-element to item of tree.
  //! @param theItem the item to which you want to add a sub-item
  void AddElementToItem(QTreeWidgetItem* theItem);

  //! Edits element of tree.
  //! @param theItem the item to be edited.
  void EditElement(QTreeWidgetItem* theItem);

  //! Removes element from tree.
  //! @param theItem the item to be removed.
  void RemoveElement(QTreeWidgetItem* theItem);

private:
  //! Rebuilds a tree from a database.
  void loadFromDatabase();

public slots:

  //! Adds element to root of tree.
  void AddElementToTree();

private slots:

  //! Shows context menu after right-click.
  //! @param thePoint the point on myTreeWidget.
  void showContextMenu(const QPoint& thePoint);

  //! Slot called when a tree item changes.
  void onItemChanged(QTreeWidgetItem* theItem, int theColumn);

private:
  QTreeWidget* myTreeWidget; //!< Tree widget.
  ManagerSQL myManagerSql; //!< The object responsible for communication with the database
  int myItemsCounter = 0; //!< Counter of added items (does not decrease when an item is removed). Used as an ID.
};
#endif // WIDGET_H
