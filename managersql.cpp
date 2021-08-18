#include "managersql.h"

#include <stdexcept>

#include <QDebug>
#include <QFile>

#define DATABASE_NAME "my_db.sqlite7"

ManagerSQL::ManagerSQL()
{
  myDatabase = QSqlDatabase::addDatabase("QSQLITE");
  myDatabase.setDatabaseName(DATABASE_NAME);
  bool aDatabaseExists = QFile::exists(DATABASE_NAME);

  if (!myDatabase.open())
  {
      qDebug() << myDatabase.lastError();
      return;
  }

  if (!aDatabaseExists)
  {
    QSqlQuery aQuery(myDatabase);
    QString aCommand = "CREATE TABLE tree ("
                       "id integer PRIMARY KEY NOT NULL, "
                       "name VARCHAR(255), "
                       "parent_id integer"
                       ");";
    bool aResult = aQuery.exec(aCommand);
    if (!aResult)
    {
        qDebug() << aQuery.lastError();
    }
  }
}

ManagerSQL::~ManagerSQL()
{
  if (IsOpen())
  {
    myDatabase.close();
  }
}

bool ManagerSQL::IsOpen() const
{
  return myDatabase.isOpen();
}

void ManagerSQL::AddItem(const int theID, const int theParentID, const QString& theName)
{
  QSqlQuery aQuery(myDatabase);
  QString aCommand = "INSERT INTO tree(id, name, parent_id)"
                      "VALUES (%1, '%2', %3);";
  aCommand = aCommand.arg(QString::number(theID))
             .arg(theName)
             .arg(QString::number(theParentID));
  bool aResult = aQuery.exec(aCommand);
  qDebug() << theID << theParentID << theName;
  qDebug() << "  Insert:" << aResult;
  if (!aResult)
  {
    qDebug() << aQuery.lastError();
  }
}

void ManagerSQL::EditItem(const int theID, const int theParentID, const QString& theNewName)
{
  QSqlQuery aQuery(myDatabase);
  QString aCheckCommand = "SELECT * FROM tree "
                          "WHERE id=%1;";
  aCheckCommand = aCheckCommand.arg(QString::number(theID));
  bool aCheckResult = aQuery.exec(aCheckCommand);
  qDebug() << "Edit:" << theID << theParentID << theNewName;
  qDebug() << "  Select:" << aCheckResult;
  if (!aCheckResult)
  {
    qDebug() << aQuery.lastError();
  }
  else
  {
    if (aQuery.next())
    {
      QString anUpdateCommand = "UPDATE tree "
                                "SET name='%1', parent_id=%2 "
                                "WHERE id=%3;";
      anUpdateCommand = anUpdateCommand.arg(theNewName)
                        .arg(QString::number(theParentID))
                        .arg(theID);
      bool anUpdateResult = aQuery.exec(anUpdateCommand);
      qDebug() << "  Update" << anUpdateResult;
      if (!anUpdateResult)
      {
        qDebug() << aQuery.lastError();
      }
    }
    else
    {
      AddItem(theID, theParentID, theNewName);
    }
  }
}

QList<QVector<QVariant>> ManagerSQL::GetData() const
{
  QList<QVector<QVariant>> aResult;
  QSqlQuery aQuery(myDatabase);
  if (!aQuery.exec("SELECT * FROM tree;"))
  {
      qDebug() << aQuery.lastError();
  }
  QSqlRecord aRec = aQuery.record();

  while (aQuery.next()) {
      int anID = aQuery.value(aRec.indexOf("id")).toInt();
      int aParentID = aQuery.value(aRec.indexOf("parent_id")).toInt();
      QString aName = aQuery.value(aRec.indexOf("name")).toString();

      aResult.append(QVector<QVariant>({anID, aName, aParentID}));
  }

  return aResult;
}

void ManagerSQL::RemoveItem(const int theID)
{
  QSqlQuery aQuery(myDatabase);
  QString aCheckCommand = "SELECT * FROM tree "
                          "WHERE id=%1;";
  aCheckCommand = aCheckCommand.arg(QString::number(theID));
  bool aCheckResult = aQuery.exec(aCheckCommand);
  qDebug() << "Remove:" << theID;
  qDebug() << "  Select:" << aCheckResult;
  if (!aCheckResult)
  {
    qDebug() << aQuery.lastError();
  }
  else
  {
    // remove children of item
    aQuery.clear();
    QString aGetChildrenCommand = "SELECT * FROM tree "
                                  "WHERE parent_id=%1;";
    aGetChildrenCommand = aGetChildrenCommand.arg(QString::number(theID));
    bool aGetChildrenResult = aQuery.exec(aGetChildrenCommand);
    if (!aGetChildrenResult)
    {
      qDebug() << aQuery.lastError();
    }
    else
    {
      QSqlRecord aRec = aQuery.record();
      while (aQuery.next())
      {
        int aChildID = aQuery.value(aRec.indexOf("id")).toInt();
        RemoveItem(aChildID);
      }
    }

    // remove item
    aQuery.clear();
    QString aRemoveCommand = "DELETE FROM tree "
                             "WHERE id=%1";
    aRemoveCommand = aRemoveCommand.arg(theID);
    bool aRemoveResult = aQuery.exec(aRemoveCommand);
    if (!aRemoveResult)
    {
      qDebug() << aQuery.lastError();
    }
  }
}
