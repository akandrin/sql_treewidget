#ifndef MANAGERSQL_H
#define MANAGERSQL_H

#include <QtSql>

class ManagerSQL
{
public:
  ManagerSQL();
  ~ManagerSQL();

  //! Is database open.
  bool IsOpen() const;

  //! Adds item to database.
  void AddItem(const int theID, const int theParentID, const QString& theName);

  //! Edits item in database.
  //! Adds an item if it doesn't exist.
  void EditItem(const int theID, const int theParentID, const QString& theNewName);

  //! Removes element in database.
  void RemoveItem(const int theID);

  //! Get table data (as QList of QVector's).
  //! QVector consists of three elements.
  //! The 1st element is ID (int).
  //! The 2nd element is the name (QString).
  //! The 3rd element is the parent's ID (int).
  QList<QVector<QVariant>> GetData() const;

private:
  QSqlDatabase myDatabase; //!< Database for tree.
};

#endif // MANAGERSQL_H
