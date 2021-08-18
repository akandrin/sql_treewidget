#include "widget.h"

#include "managersql.h"

#include <QDebug>
#include <QLayout>
#include <QMenu>

Widget::Widget(QWidget* theParent)
  : QWidget(theParent), myTreeWidget(new QTreeWidget(this)), myManagerSql()
{

  QLayout* aLayout = new QVBoxLayout(this);
  aLayout->addWidget(myTreeWidget);
  setLayout(aLayout);

  myTreeWidget->setHeaderHidden(true);

  myTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(myTreeWidget,
          &QTreeWidget::customContextMenuRequested,
          this,
          &Widget::showContextMenu);

  connect(myTreeWidget,
          &QTreeWidget::itemChanged,
          this,
          &Widget::onItemChanged);

  loadFromDatabase();
}

Widget::~Widget()
{
  disconnect(myTreeWidget,
             &QTreeWidget::customContextMenuRequested,
             this,
             &Widget::showContextMenu);

  disconnect(myTreeWidget,
             &QTreeWidget::itemChanged,
             this,
             &Widget::onItemChanged);
}

QList<QTreeWidgetItem*> Widget::GetItems(QTreeWidget* theTreeWidget)
{
  std::function<void(QTreeWidgetItem*, QList<QTreeWidgetItem*>&)> GetSubItems;
  GetSubItems = [&GetSubItems](QTreeWidgetItem* theTreeWidgetItem, QList<QTreeWidgetItem*>& theList)
  {
    if (theTreeWidgetItem == nullptr)
      return;
    theList << theTreeWidgetItem;
    for(int anInd = 0; anInd < theTreeWidgetItem->childCount(); ++anInd)
    {
        GetSubItems(theTreeWidgetItem->child(anInd), theList);
    }
  };

  QList<QTreeWidgetItem*> aResultList;
  for(int anInd = 0; anInd < theTreeWidget->topLevelItemCount(); ++anInd)
  {
      GetSubItems(theTreeWidget->topLevelItem(anInd), aResultList);
  }
  return aResultList;
}

void Widget::loadFromDatabase()
{
  if (!myManagerSql.IsOpen())
    return;

  myTreeWidget->clear();
  myTreeWidget->blockSignals(true);
  QList<QVector<QVariant>> aData = myManagerSql.GetData();
  std::sort(aData.begin(), aData.end(),
            [](const QVector<QVariant>& aVec1, const QVector<QVariant>& aVec2)
            {
              return aVec1[0].toInt() < aVec2[0].toInt();
            });
  for (const auto& aRow : aData)
  {
    int anID = aRow[0].toInt();
    QString aName = aRow[1].toString();
    int aParentID = aRow[2].toInt();

    QTreeWidgetItem* anItem = nullptr;
    if (aParentID == 0)
    {
      // Current item is root sub-item.
      anItem = new QTreeWidgetItem(myTreeWidget);
    }
    else
    {
      // Current item is sub-item of some item.
      // Search this item.
      auto anItems = GetItems(myTreeWidget);
      for (const auto& aPossibleParentItem : anItems)
      {
        if (aPossibleParentItem->data(0, Qt::UserRole).toInt() == aParentID)
        {
          anItem = new QTreeWidgetItem(aPossibleParentItem);
          break;
        }
      }
      if (anItem == nullptr)
      {
        assert(0);
      }
    }
    anItem->setData(0, Qt::UserRole, QVariant(anID));
    anItem->setText(0, aName);
    myItemsCounter = anID;
  }
  myTreeWidget->blockSignals(false);
}

void Widget::showContextMenu(const QPoint& thePoint)
{
  QMenu aContextMenu(tr("Context menu"), this);

  QTreeWidgetItem* anItem = myTreeWidget->itemAt(mapFrom(this, thePoint));
  qDebug() << anItem;
  if (anItem)
  {
    {
      QAction* anActionAddItem = new QAction("Add adjacent item", &aContextMenu);
      connect(anActionAddItem, &QAction::triggered, this,
              [anItem, this]()
      {
        QTreeWidgetItem* aParent = anItem->parent();
        if (aParent == nullptr)
        {
          AddElementToTree();
        }
        else
        {
          AddElementToItem(aParent);
        }
      });
      aContextMenu.addAction(anActionAddItem);
    }

    {
      QAction* anActionAddSubItem =  new QAction("Add sub-item", &aContextMenu);
      connect(anActionAddSubItem, &QAction::triggered, this,
              [anItem, this]()
      {
        AddElementToItem(anItem);
      });
      aContextMenu.addAction(anActionAddSubItem);
    }

    {
      QAction* anActionEditItem = new QAction("Edit item", &aContextMenu);
      connect(anActionEditItem, &QAction::triggered, this,
              [anItem, this]()
              {
                EditElement(anItem);
              });
      aContextMenu.addAction(anActionEditItem);
    }

    {
      QAction* anActionRemoveItem = new QAction("Remove item", &aContextMenu);
      connect(anActionRemoveItem, &QAction::triggered, this,
              [anItem, this]()
              {
                RemoveElement(anItem);
              });
      aContextMenu.addAction(anActionRemoveItem);
    }
  }
  else
  {
    QAction* anActionAddItem =  new QAction("Add item", &aContextMenu);
    connect(anActionAddItem, &QAction::triggered, this,
            &Widget::AddElementToTree);
    aContextMenu.addAction(anActionAddItem);
  }

  aContextMenu.exec(mapToGlobal(myTreeWidget->mapToParent(thePoint)));
}

void Widget::AddElementToTree()
{
  QTreeWidgetItem* anItem = new QTreeWidgetItem(myTreeWidget);
  anItem->setData(0, Qt::UserRole, QVariant(++myItemsCounter));
  anItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  myTreeWidget->editItem(anItem, 0);
}

void Widget::AddElementToItem(QTreeWidgetItem* theItem)
{
  qDebug() << theItem;
  if (theItem == nullptr)
    return;

  QTreeWidgetItem* aSubItem = new QTreeWidgetItem(theItem);
  aSubItem->setData(0, Qt::UserRole, QVariant(++myItemsCounter));
  aSubItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  myTreeWidget->expandItem(theItem);
  myTreeWidget->editItem(aSubItem, 0);
}

void Widget::EditElement(QTreeWidgetItem* theItem)
{
  myTreeWidget->editItem(theItem, 0);
}

void Widget::RemoveElement(QTreeWidgetItem* theItem)
{
  myManagerSql.RemoveItem(theItem->data(0, Qt::UserRole).toInt());
  delete theItem;
}

void Widget::onItemChanged(QTreeWidgetItem* theItem, int theColumn)
{
  int anID = theItem->data(theColumn, Qt::UserRole).toInt();
  QTreeWidgetItem* aParent = theItem->parent();
  int aParentID = aParent == nullptr ? 0 : aParent->data(theColumn, Qt::UserRole).toInt();
  QString aText = theItem->text(theColumn);
  myManagerSql.EditItem(anID, aParentID, aText);
}
